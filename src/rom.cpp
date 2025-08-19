/***
 * @file rom.cpp
 * @brief ROM builder class implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 *   https://www.msx.org/wiki/Develop_a_program_in_cartridge_ROM
 *   https://www.msx.org/wiki/MegaROM_Mappers
 *   https://aoineko.org/msxgl/index.php?title=Create_a_plain_ROM
 *   https://aoineko.org/msxgl/index.php?title=Create_a_mapped_ROM
 */

#include "rom.h"

#include "header.h"
#include "header_pt3.h"
#include "pt3.h"
#include "routines.h"  //! pasmo routines.asm routines.bin | xxd -i routines.bin routines.h
#include "start.h"
#include "turbo.h"  //! xxd -i xbasic.bin

//----------------------------------------------------------------------------------------------

Rom::Rom() {
  data = (unsigned char *)malloc(ROM_DATA_SIZE);
  memset(data, 0x00, ROM_DATA_SIZE);
  //! @remark default rom size
  rom_size = 0x4000 * 2;
}

Rom::~Rom() {
  free(data);
}

bool Rom::build(Tokenizer *tokenizer) {
  int stdPageLen;
  float maxPageLen = 0x4000;

  // initialize data

  this->tokenizer = tokenizer;
  this->opts = tokenizer->opts;
  this->compiler = 0;

  buildInit();

  // process data

  buildHeader();

  buildXBASIC();

  buildRoutines();

  buildMap(&tokenizer->resourceList, tokenizer->font);

  if (tokenizer->resourceList.size()) buildResources(&tokenizer->resourceList);

  if (tokenizer->font) buildFontResources();

  if (tokenizer->pt3) buildPT3TOOLS();

  buildBasicCode();

  buildHeaderAdjust();

  // save ROM file

  if (!errorFound) {
    writeRom(opts->outputFilename);
  }

  // calculate and check sizes

  stdPageLen = hdrLen + rtnLen + mapLen + txtLen + filLen + pt3Len + basLen;

  if (opts->megaROM) {
    errorMessage = "Extended scheme (MegaROM) not allowed to tokenized mode";
    errorFound = true;
  } else {
    if (stdPageLen > maxPageLen) {
      errorMessage =
          "Basic code+resources exceeded valid ROM page size limit (16kb)";
      errorFound = true;
    } else {
      stdMemoryPerc = (stdPageLen / maxPageLen) * 100;
    }
  }

  return !errorFound;
}

bool Rom::build(Compiler *compiler) {
  int stdPageLen;
  float maxPageLen;

  // initialize data

  this->tokenizer = 0;
  this->compiler = compiler;
  this->opts = compiler->opts;

  buildInit();

  // process header data

  buildHeader();

  buildCompilerRoutines();

  if (!compiler->compiled) return false;

  buildCompiledCode();

  // build resource data

  if (compiler->resourceList.size()) {
    buildMapAndResources();
    if (errorFound) return false;
    if (!opts->megaROM) {
      writePage[0] = true;
    }
  }

  // adjust header pointers

  buildHeaderAdjust();

  // save ROM file

  writeRom(opts->outputFilename);

  // calculate and check sizes

  stdPageLen = hdrLen + rtnLen + basLen;

  if (opts->megaROM) {
    maxPageLen = rom_size - 0x4000;  // first page = kernel
    stdMemoryPerc = (stdPageLen / maxPageLen) * 100;
    rscMemoryPerc = (rscLen / maxPageLen) * 100;
  } else {
    maxPageLen = 0x4000;
    if (stdPageLen > maxPageLen) {
      errorMessage = "Basic code exceeded valid ROM page size limit (16kb)";
      errorFound = true;
    } else {
      stdMemoryPerc = (stdPageLen / maxPageLen) * 100;
    }
    rscMemoryPerc = (rscLen / (maxPageLen - 0x100)) * 100;
  }

  return !errorFound;
}

bool Rom::build(CompilerPT3 *compiler) {
  int stdPageLen;
  float maxPageLen = 0x4000;

  // initialize data

  this->tokenizer = 0;
  this->compiler = compiler;
  this->opts = compiler->opts;

  buildInit();

  // process data

  buildHeader();

  // buildXBASIC();  // <--- already included into header

  buildCompilerRoutinesPT3();

  buildMap(&compiler->resourceList, false);

  if (compiler->resourceList.size()) {
    buildResources(&compiler->resourceList);
    if ((txtLen + filLen) > 15500) {
      errorMessage = "Resources exceeded valid size limit (15,5kb)";
      errorFound = true;
      return false;
    }
  }

  if (compiler->compiled)
    buildCompiledCode();
  else
    return false;

  buildHeaderAdjust();

  // save ROM file

  writeRom(opts->outputFilename);

  // calculate and check sizes

  stdPageLen = hdrLen + rtnLen + mapLen + txtLen + filLen + pt3Len + basLen;

  if (opts->megaROM) {
    maxPageLen = rom_size - 0x4000;
    if (compiler->segm_total == 0) compiler->segm_total = 4;
    stdMemoryPerc = (stdPageLen / maxPageLen) * 100;
  } else {
    if (stdPageLen > maxPageLen) {
      errorMessage =
          "Basic code+resources exceeded valid ROM page size limit (16kb)";
      errorFound = true;
    } else {
      stdMemoryPerc = (stdPageLen / maxPageLen) * 100;
    }
  }

  return !errorFound;
}

//----------------------------------------------------------------------------------------------

void Rom::buildInit() {
  int i;

  memset(data, 0x00, ROM_DATA_SIZE);  // sizeof(data));
  stdMemoryPerc = 0;
  hdrLen = 16;
  rtnLen = 0;
  mapLen = 0;
  txtLen = 0;
  filLen = 0;
  pt3Len = 0;
  basLen = 0;
  rscLen = 0;
  rscSgm = 0;
  codeSgm = 0;
  code_start = 0x8000;

  for (i = 0; i < COMPILE_MAX_PAGES; i++) writePage[i] = false;

  writePage[1] = true;
  writePage[2] = true;

  errorFound = false;

  hdrAddr = 0x8000;
  xbcAddr = 0x4000;
  mapAddr = hdrAddr;
  txtAddr = hdrAddr;
  filAddr = hdrAddr;
  rtnAddr = hdrAddr;
  basAddr = hdrAddr;

  pt3Addr = filAddr;
}

void Rom::buildHeader() {
  unsigned char *header = &data[hdrAddr];

  // ID = AB
  header[0] = 0;  // 0x41;
  header[1] = 0;  // 0x42;

  // INIT = 0x0000
  header[2] = 0;
  header[3] = 0;

  // STATEMENT = 0x0000
  header[4] = 0;  // 0x10 + 6;
  header[5] = 0;  // 0x80;    // fake call

  // DEVICE = 0x0000
  header[6] = 0;
  header[7] = 0;

  // TEXT = 0x0000
  header[8] = 0;
  header[9] = 0;

  // reserved area
  header[10] = 0;
  header[11] = 0;
  header[12] = 0;
  header[13] = 0;
  header[14] = 0;
  header[15] = 0;

  hdrLen = 16;
}

void Rom::buildXBASIC() {
  memcpy(&data[xbcAddr], xbasic2_rom, xbasic2_rom_len);
}

void Rom::buildRoutines() {
  rtnAddr = hdrAddr + hdrLen;
  rtnLen = bin_routines_bin_len;
  memcpy(&data[rtnAddr], bin_routines_bin, bin_routines_bin_len);
}

void Rom::buildCompilerRoutines() {
  // add start code, Arkos Tracker and FONTS
  // limit XBASIC address: 0x5B36 (0x1B36 bytes = 6966 bytes)
  memcpy(&data[xbcAddr], bin_header_bin, bin_header_bin_len);

  rtnAddr = hdrAddr + hdrLen;
  rtnLen = bin_start_bin_len;
  memcpy(&data[rtnAddr], bin_start_bin, bin_start_bin_len);
}

void Rom::buildCompilerRoutinesPT3() {
  // add start code, PT3TOOLS and FONTS
  // limit XBASIC address: 0x5B36 (0x1B36 bytes = 6966 bytes)
  memcpy(&data[xbcAddr], bin_header_pt3_bin, bin_header_pt3_bin_len);

  rtnAddr = hdrAddr + hdrLen;
  rtnLen = bin_start_bin_len;
  memcpy(&data[rtnAddr], bin_start_bin, bin_start_bin_len);
}

void Rom::buildMap(vector<Lexeme *> *resourceList, bool font) {
  int resCount;

  resCount = resourceList->size();
  if (font) resCount += 3;

  mapAddr = hdrAddr + hdrLen + rtnLen;
  mapLen =
      4 + resCount * 4;  // pt3 address + resources count + [ address, size ]
  mapInd = mapAddr + 2;

  memcpy(&data[mapInd], &resCount, 2);
  mapInd += 2;
}

void Rom::buildResources(vector<Lexeme *> *resourceList) {
  string filename;
  int size_read, i, t;
  Lexeme *lexeme;

  txtLen = 0;
  filLen = 0;
  txtAddr = mapAddr + mapLen;
  filAddr = txtAddr;
  txtInd = txtAddr;
  filInd = filAddr;

  mapInd = mapAddr + 4;

  t = (int)resourceList->size();

  for (i = 0; i < t; i++) {
    filInd = txtInd;

    lexeme = (*resourceList)[i];

    if (lexeme->name == "TEXT") {
      size_read = lexeme->value.size();
      if (size_read) {
        memcpy(&data[txtInd], lexeme->value.c_str(), size_read);
      }
      data[txtInd + size_read] = 0;
      size_read++;

      memcpy(&data[mapInd], &txtInd, 2);
      mapInd += 2;
      memcpy(&data[mapInd], &size_read, 2);
      mapInd += 2;

      txtInd += size_read;
      txtLen += size_read;

    } else if (lexeme->name == "FILE") {
      // strips quotes from file name
      filename = file.stripQuotes(lexeme->value);

      // if file not found, try search it at input path
      if (!fileExists(filename)) {
        filename = pathJoin(opts->inputPath, filename);
      }

      size_read = file.readFromFile(filename, &data[filInd], 0x4000);
      if (size_read > 0) {
        memcpy(&data[mapInd], &filInd, 2);
        mapInd += 2;
        memcpy(&data[mapInd], &size_read, 2);
        mapInd += 2;

        txtInd += size_read;
        filLen += size_read;

      } else {
        errorMessage = "Resource file not found: " + lexeme->value;
        errorFound = true;
      }
    }
  }
}

void Rom::buildMapAndResources() {
  int i, t;
  int start_resource_address;
  int resource_segment, max_resource_size;
  Lexeme *lexeme;

  //! @remarks
  // Resources location on ROM:
  //    48kb ROM - starts on page 0
  //    128kb MEGAROM - starts on the next segment after the code
  // Resources are structured in map and data sections
  //    map section starts at position 0x0010 of first segment
  //       WORD resource_count
  //       struct resource { WORD offset_on_page, BYTE segment, WORD
  //       resource_size } [resource_count] so, a max of 48 resources is allowed
  //       (=5 * 48 + 16 = 256 bytes)
  //    data section starts at position 0x0100 of first segment
  //       TEXT:
  //         same as resource type TXT of DATA
  //       DATA:
  //         BYTE data_resource_type
  //         resource type 0 - DATA
  //            WORD resource_items_count
  //            struct lines_map { WORD line_number, BYTE line_items_count }
  //            [resource_items_count] struct lines_data { STRINGS
  //            item_data[line_items_count] } [resource_items_count]
  //         resource type 1 - CSV
  //            WORD resource_items_count
  //            struct lines_map { BYTE line_items_count }
  //            [resource_items_count] struct lines_data { STRINGS
  //            item_data[line_items_count] } [resource_items_count]
  //         resource type 2 - TXT
  //            STRINGS item_data[]
  //         resource type 3 - IDATA
  //            WORD resource_items_count
  //            struct lines_map { WORD line_number, BYTE line_items_count }
  //            [resource_items_count] struct lines_data { WORD
  //            item_data[line_items_count] } [resource_items_count]
  //       FILE:
  //         ARKOS files - plain data, continuous on the same segment
  //            BYTE data[]
  //         NMSXTILES files - compressed, continuous on the same segment or
  //         move to the next
  //            BYTE data[]
  //         TINYSPRITE files - compressed, continuous on the same segment or
  //         move to the next
  //            BYTE data[]
  //         SCREEN IMAGE (BLOAD) files - compressed, discontinuous (blocks of
  //         256 bytes)
  //            WORD blocks, STRINGS data[]
  //                         ^== each block starts with its compressed data
  //                         length (1 byte)
  //                             followed by the compressed data itself

  if (opts->megaROM) {
    resource_segment = codeSgm;
    start_resource_address = resource_segment * 0x4000;
    rom_size = 0x20000;
    max_resource_size = rom_size - start_resource_address;
    //! resource start address
    rscAddr = 0x8000;
    //! resource start segment (two konami segments of 8kb size each)
    rscSgm = resource_segment * 2;
    //! @remark
    //! skip first page at temporary memory because it will be discarded
    start_resource_address += 0x4000;

  } else {
    resource_segment = 0;
    start_resource_address = resource_segment;
    rscAddr = resource_segment;
    rscSgm = resource_segment;
    max_resource_size = 0x4000 - 0x0100;
    rom_size = 0x4000 * 3;
  }

  mapAddr = start_resource_address + 0x0010;
  txtAddr = start_resource_address + 0x0100;
  filAddr = txtAddr;

  txtLen = 0;
  filLen = 0;
  txtInd = txtAddr;
  filInd = filAddr;

  t = compiler->resourceList.size();

  //! @warning max resource map length
  if (t > 48) {
    errorMessage = "Max of resources limit exceeded (>48)";
    errorFound = true;
    return;
  }

  mapInd = mapAddr;
  mapLen = 16 + t * 5;  // resources count * [ offset, segment, size ]

  memcpy(&data[mapInd], &t, 2);
  mapInd += 2;

  for (i = 0; i < t; i++) {
    filInd = txtInd;

    lexeme = compiler->resourceList[i];

    if (lexeme->name == "TEXT") {
      buildMapAndResourcesText(lexeme);

    } else if (lexeme->name == "_DATA_") {
      buildMapAndResourcesData(compiler->parser);

    } else if (lexeme->name == "FILE") {
      buildMapAndResourcesFile(lexeme);
    }
  }

  rscLen = mapLen + filLen + txtLen;
  mapLen = 0;
  filLen = 0;
  txtLen = 0;

  if (opts->megaROM) {
    i = (start_resource_address - 0x4000 + rscLen + 0x0100);
    rom_size = ((i / 0x20000) + 1) * 0x20000;
    max_resource_size = rom_size - i + rscLen;
    t = rom_size / 0x4000 + 1;
    for (i = 1; i < t; i++) {
      writePage[i] = true;
    }
  }

  if (rscLen > max_resource_size) {
    char s[400];
    sprintf(s,
            "Resources total size = %i byte(s)\nResources exceeded valid ROM "
            "page size limit (%i)\nDifference = %i byte(s)",
            rscLen, max_resource_size, rscLen - max_resource_size);
    errorMessage = s;
    if (!opts->megaROM) {
      errorMessage +=
          "\nTry compiling it in MegaROM format by adding the -x parameter";
    }
    errorFound = true;
  }
}

void Rom::addResourceToMap(int offset, int length, int filler) {
  int address = (opts->megaROM) ? (offset % 0x4000) + rscAddr : offset;
  int segment = (opts->megaROM) ? ((offset / 0x4000) - 1) * 2 : 0;

  memcpy(&data[mapInd], &address, 2);
  mapInd += 2;
  memcpy(&data[mapInd], &segment, 1);
  mapInd += 1;
  memcpy(&data[mapInd], &length, 2);
  mapInd += 2;

  txtInd += length + filler;
  filLen += length + filler;
}

void Rom::buildMapAndResourcesText(Lexeme *lexeme) {
  int size_read = 0, length;
  int next_segment = ((txtInd / 0x4000) + 1) * 0x4000;
  int filler = 0;

  length = lexeme->value.size();

  if ((txtInd + length + 2) >=
      next_segment) {  // if string doesn't fit on current segment, skip to the
                       // next
    filler += (next_segment - txtInd);
    txtInd = next_segment;
  }

  if (length) {
    memcpy(&data[txtInd], lexeme->value.c_str(), length);
    size_read += length;
  }
  data[txtInd + length] = 0;
  size_read++;

  addResourceToMap(txtInd, size_read, filler);

  filLen -= (size_read + filler);
  txtLen += (size_read + filler);
}

void Rom::buildMapAndResourcesData(Parser *parser) {
  int size_read, i, k, l, len;
  int t = parser->datas.size();
  int map_size_forecast, map_start_segment, map_end_segment;
  int item_size_forecast, item_start_segment, item_end_segment;
  int filler = 0;
  char *s;
  bool idata = false;
  Lexeme *lexeme;
  vector<string> items;
  vector<int> nums;
  vector<int> maps;
  string item;

  // prepare DATA/IDATA items to be processed...

  items.clear();
  nums.clear();
  maps.clear();

  item.clear();
  l = 0;  // current line number
  k = 0;  // item number at current line

  for (i = 0; i < t; i++) {
    lexeme = parser->datas[i];
    if (lexeme->subtype == Lexeme::subtype_integer_data) idata = true;
    if (item == lexeme->tag) {
      k++;  // one more item at current line
    } else {
      if (k) {
        nums.push_back(l);
        maps.push_back(k);
      }
      item = lexeme->tag;
      k = 1;  // first item at current line
      try {
        l = stoi(item);  // msx-basic user code line number
      } catch (exception &e) {
        l = 0;
      }
    }
  }
  if (k) {  // add the last item at current line...
    nums.push_back(l);
    maps.push_back(k);
  }

  // write DATA/IDATA items to the rom

  size_read = 0;

  map_size_forecast = (maps.size() * 3) + 3 + 256;
  map_start_segment = (txtInd / 0x4000);
  map_end_segment = (txtInd + map_size_forecast) / 0x4000;
  if (map_start_segment !=
      map_end_segment) {  // if DATA/IDATA map doesn't fit on current segment...
    txtInd = (map_start_segment + 1) * 0x4000;  // ...skip to the next one
    filler += (txtInd - filInd);
    filInd = txtInd;
  }

  if (idata)
    data[filInd++] = 3;  // IDATA statement resource code
  else
    data[filInd++] = 0;  // DATA statement resource code
  size_read += 1;

  data[filInd++] = (maps.size() & 0xFF);  // WORD lines
  data[filInd++] = ((maps.size() >> 8) & 0xFF);
  size_read += 2;

  for (i = 0; i < (int)maps.size(); i++) {  // DATA/IDATA map
    k = maps[i];
    l = nums[i];

    data[filInd++] = (l & 0xFF);  // WORD line number
    data[filInd++] = ((l >> 8) & 0xFF);

    data[filInd++] = (k & 0xFF);  // BYTE items_in_the_line
  }
  size_read += (maps.size() * 3);

  for (i = 0; i < t; i++) {  // DATA/IDATA items values
    lexeme = parser->datas[i];
    item = lexeme->value;

    if (lexeme->subtype == Lexeme::subtype_integer_data) {
      len = 1;
      try {
        k = stoi(item);  // item value as integer
      } catch (exception &e) {
        k = 0;
      };

      item_size_forecast = 2;
      item_start_segment = (filInd / 0x4000);
      item_end_segment = (filInd + item_size_forecast) / 0x4000;
      if (item_start_segment !=
          item_end_segment) {  // if IDATA item doesn't fit on current
                               // segment...
        item_end_segment =
            (item_start_segment + 1) * 0x4000;  // ...skip to the next one
        while (filInd < item_end_segment) {
          data[filInd++] = 0xFF;
          filler++;
        }
      }

      data[filInd++] = (k & 0xFF);
      data[filInd++] =
          ((k >> 8) & 0xFF);  // write integer value at ROM position
    } else {
      s = (char *)item.c_str();  // item value as string
      len = item.size();
      if (len) {
        if (s[len - 1] == '"') {  // strip quotes
          len--;
          s[len] = 0;
        }
        if (s[0] == '"') {
          s++;
          len--;
        }
      }

      item_size_forecast = len + 2;
      item_start_segment = (filInd / 0x4000);
      item_end_segment = (filInd + item_size_forecast) / 0x4000;
      if (item_start_segment !=
          item_end_segment) {  // if DATA item doesn't fit on current segment...
        item_end_segment =
            (item_start_segment + 1) * 0x4000;  // ...skip to the next one
        while (filInd < item_end_segment) {
          data[filInd++] = 0xFF;
          filler++;
        }
      }

      data[filInd++] = (len & 0xFF);  // string length

      if (len) {
        memcpy(&data[filInd], s, len);  // write string value at rom position
        filInd += len;
      }
    }

    size_read += (len + 1);
  }

  addResourceToMap(txtInd, size_read, filler);
}

void Rom::buildMapAndResourcesFile(Lexeme *lexeme) {
  string filename, fileext;

  // strips quotes from file name
  filename = file.stripQuotes(lexeme->value);

  // if file not found, try search it at input path
  if (!fileExists(filename)) {
    filename = pathJoin(opts->inputPath, filename);
  }

  fileext = file.getFileExt(filename);

  // text data resource file
  if (strcasecmp(fileext.c_str(), ".TXT") == 0) {
    buildMapAndResourcesFileTXT(filename);

    // csv data resource file
  } else if (strcasecmp(fileext.c_str(), ".CSV") == 0) {
    buildMapAndResourcesFileCSV(filename);

    // tiny sprite resource file
  } else if (strcasecmp(fileext.c_str(), ".SPR") == 0) {
    buildMapAndResourcesFileSPR(filename);

    // screen resource file
  } else if (strcasecmp(fileext.c_str(), ".SC0") == 0 ||
             strcasecmp(fileext.c_str(), ".SC1") == 0 ||
             strcasecmp(fileext.c_str(), ".SC2") == 0 ||
             strcasecmp(fileext.c_str(), ".SC3") == 0 ||
             strcasecmp(fileext.c_str(), ".SC4") == 0 ||
             strcasecmp(fileext.c_str(), ".SC5") == 0 ||
             strcasecmp(fileext.c_str(), ".SC6") == 0 ||
             strcasecmp(fileext.c_str(), ".SC7") == 0 ||
             strcasecmp(fileext.c_str(), ".SC8") == 0 ||
             strcasecmp(fileext.c_str(), ".SC9") == 0 ||
             strcasecmp(fileext.c_str(), ".S10") == 0 ||
             strcasecmp(fileext.c_str(), ".S11") == 0 ||
             strcasecmp(fileext.c_str(), ".S12") == 0) {
    buildMapAndResourcesFileSCR(filename);

    // binary resource file
  } else {
    buildMapAndResourcesFileBIN(filename, fileext);
  }
}

void Rom::buildMapAndResourcesFileTXT(string filename) {
  FILE *file;
  char line[255];
  int len;
  int size_read, filler = 0;
  int item_size_forecast, item_start_segment, item_end_segment;

  if ((file = fopen(filename.c_str(), "r"))) {
    data[filInd++] = 2;  // TXT data resource
    size_read = 1;

    while (fgets(line, sizeof(line), file)) {
      len = strlen(line);

      while (len) {
        if (line[len - 1] < 0x20) {
          line[len - 1] = 0;
          len--;
        } else
          break;
      }

      item_size_forecast = len + 2;
      item_start_segment = (filInd / 0x4000);
      item_end_segment = (filInd + item_size_forecast) / 0x4000;
      if (item_start_segment !=
          item_end_segment) {  // if DATA item doesn't fit on current segment...
        item_end_segment =
            (item_start_segment + 1) * 0x4000;  // ...skip to the next one
        while (filInd < item_end_segment) {
          data[filInd++] = 0xFF;
          filler++;
        }
      }

      data[filInd++] = (len & 0xFF);

      if (len) {
        memcpy(&data[filInd], line, len);
        filInd += len;
      }

      size_read += (len + 1);
    }

    fclose(file);

    addResourceToMap(txtInd, size_read, filler);

  } else {
    errorMessage = "Resource file not found: " + string(filename);
    errorFound = true;
    return;
  }
}

void Rom::buildMapAndResourcesFileCSV(string filename) {
  FILE *file;
  char line[255];
  int len;
  int size_read, filler = 0;
  int state;
  char *s;
  int i, k;
  int map_size_forecast, map_start_segment, map_end_segment;
  int item_size_forecast, item_start_segment, item_end_segment;
  vector<string> items;
  vector<int> nums;
  vector<int> maps;
  string item;

  if ((file = fopen(filename.c_str(), "r"))) {
    // prepare CSV items to be processed...

    items.clear();
    maps.clear();

    while (fgets(line, sizeof(line), file)) {
      len = strlen(line);

      while (len) {
        if (line[len - 1] < 0x20) {
          line[len - 1] = 0;
          len--;
        } else
          break;
      }

      item.clear();
      state = 0;
      k = 0;

      for (i = 0; i < len; i++) {
        switch (state) {
          case 0: {
            if (line[i] == '"') {
              state = 1;
            } else if (line[i] == ',' || line[i] == ';' || line[i] == 8) {
              k++;
              items.push_back(item);
              item.clear();
            } else if (line[i] == ' ' && item.size() == 0) {
            } else {
              item.push_back(line[i]);
            }
          } break;
          case 1: {
            if (line[i] == '"') {
              state = 2;
            } else {
              item.push_back(line[i]);
            }
          } break;
          case 2: {
            if (line[i] == ',' || line[i] == ';' || line[i] == 8) {
              state = 0;
              k++;
              items.push_back(item);
              item.clear();
            }
          }
        }
      }

      k++;
      items.push_back(item);
      maps.push_back(k);
    }

    fclose(file);

    // write CSV items to the rom

    size_read = 0;

    map_size_forecast = maps.size() + 3 + 256;
    map_start_segment = (txtInd / 0x4000);
    map_end_segment = (txtInd + map_size_forecast) / 0x4000;
    if (map_start_segment != map_end_segment) {  // if CSV items map doesn't fit
                                                 // on current segment...
      txtInd = (map_start_segment + 1) * 0x4000;  // ...skip to the next one
      filler += (txtInd - filInd);
      filInd = txtInd;
    }

    data[filInd++] = 1;  // CSV data resource
    size_read += 1;

    data[filInd++] = (maps.size() & 0xFF);  // WORD lines
    data[filInd++] = ((maps.size() >> 8) & 0xFF);
    size_read += 2;

    for (i = 0; i < (int)maps.size(); i++) {
      k = maps[i];
      data[filInd++] = (k & 0xFF);  // BYTE items_in_the_line
    }
    size_read += maps.size();

    for (i = 0; i < (int)items.size(); i++) {
      item = items[i];

      s = (char *)item.c_str();
      if (s[0] == '&') {
        try {
          if (s[1] == 'h' || s[1] == 'H')
            k = stoi(item.substr(2), 0, 16);
          else if (s[1] == 'o' || s[1] == 'O')
            k = stoi(item.substr(2), 0, 8);
          else if (s[1] == 'b' || s[1] == 'B')
            k = stoi(item.substr(2), 0, 2);
          else
            k = 0;
        } catch (exception &e) {
          k = 0;
        }
        item = to_string(k);
      }

      len = item.size();

      item_size_forecast = len + 2;
      item_start_segment = (filInd / 0x4000);
      item_end_segment = (filInd + item_size_forecast) / 0x4000;
      if (item_start_segment !=
          item_end_segment) {  // if CSV item doesn't fit on current segment...
        item_end_segment =
            (item_start_segment + 1) * 0x4000;  // ...skip to the next one
        while (filInd < item_end_segment) {
          data[filInd++] = 0xFF;
          filler++;
        }
      }

      data[filInd++] = (len & 0xFF);

      if (len) {
        memcpy(&data[filInd], item.c_str(), len);
        filInd += len;
      }

      size_read += (len + 1);
    }

    addResourceToMap(txtInd, size_read, filler);

  } else {
    errorMessage = "Resource file not found: " + string(filename);
    errorFound = true;
    return;
  }
}

// bload alternative via resource file
void Rom::buildMapAndResourcesFileSCR(string filename) {
  FileNode *file;
  int block_start;
  int bytes, size_read, filler;
  int item_size_forecast, item_start_segment, item_end_segment;
  int map_size_forecast, map_start_segment, map_end_segment;

  file = new FileNode();
  file->name = filename;
  file->packed = true;

  size_read = 0;
  filler = 0;

  if (file->open()) {
    map_size_forecast = 0x102;
    map_start_segment = (filInd / 0x4000);
    map_end_segment = (filInd + map_size_forecast) / 0x4000;
    if (map_start_segment != map_end_segment) {  // if SCR items map doesn't fit
                                                 // on current segment...
      map_end_segment =
          (map_start_segment + 1) * 0x4000;  // ...skip to the next one
      while (filInd < map_end_segment) {
        data[filInd++] = 0xFF;
        filler++;
      }
    }

    block_start = filInd;
    filInd += 2;
    size_read += 2;

    while (!file->eof()) {
      bytes = file->read();
      if (bytes == 0) {
        file->close();
        errorMessage = "Error packing image file resource (empty block size)";
        errorFound = true;
        delete file;
        return;
      }
      if (bytes > 255) {
        file->close();
        errorMessage =
            "Error packing image file resource (block size > 255 bytes)";
        errorFound = true;
        delete file;
        return;
      }

      item_size_forecast = bytes + 2;
      item_start_segment = (filInd / 0x4000);
      item_end_segment = (filInd + item_size_forecast) / 0x4000;
      if (item_start_segment !=
          item_end_segment) {  // if SCR item doesn't fit on current segment...
        item_end_segment =
            (item_start_segment + 1) * 0x4000;  // ...skip to the next one
        while (filInd < item_end_segment) {
          data[filInd++] = 0xFF;
          filler++;
        }
      }

      data[filInd] = bytes;
      filInd++;
      memcpy(&data[filInd], file->buffer, bytes);

      filInd += bytes;
      size_read += bytes + 1;
    }
    file->close();

    if (size_read) {
      // blocks count
      data[block_start] = (file->blocks & 0xff);
      data[block_start + 1] = (file->blocks >> 8) & 0xff;

      addResourceToMap(block_start, size_read, filler);

    } else {
      errorMessage = "Empty image file resource";
      errorFound = true;
    }

  } else {
    errorMessage = "Resource file not found: " + string(filename);
    errorFound = true;
  }

  delete file;
}

void Rom::buildMapAndResourcesFileSPR(string filename) {
  unsigned char *buffer = (unsigned char *)malloc(0x4000);
  int size_read = file.ParseTinySpriteFile(filename, buffer, 0x4000);
  int next_segment = ((filInd / 0x4000) + 1) * 0x4000;
  int filler = 0;
  Pletter pletter;

  if (size_read > 0) {
    if ((filInd + size_read + 0xF) >=
        next_segment) {  // skip data to the next segment if it doesn't fit in
                         // the current one
      filler = next_segment - filInd;
      filInd = next_segment;
    }

    compiler->has_tiny_sprite = true;

    // compress buffer to data[filInd]
    size_read = pletter.pack(buffer, size_read, (unsigned char *)&data[filInd]);

    addResourceToMap(filInd, size_read, filler);
  } else {
    if (size_read < 0) {
      errorMessage = "Resource file not found: " + string(filename);
    } else {
      errorMessage = "Invalid Tiny Sprite resource: " + string(filename);
    }
    errorFound = true;
  }

  free(buffer);
}

void Rom::buildMapAndResourcesFileBIN(string filename, string fileext) {
  unsigned char *buffer = (unsigned char *)malloc(0x4000);
  int size_read = file.readFromFile(filename, buffer, 0x4000);
  int next_segment = ((filInd / 0x4000) + 1) * 0x4000, address;
  int filler = 0;

  if (size_read <= 0) {
    errorMessage = "Resource file not found: " + filename;
    errorFound = true;
  } else if (size_read >
             0x3D00) {  // 15kb: arkos tracker AKM+AKX files (+map) must fit
                        // together on the first resource page
    errorMessage =
        "Resource file size exceeds maximum limit (15kb): " + filename;
    errorFound = true;
  } else {
    if ((filInd + size_read + 0xF) >=
        next_segment) {  // skip data to the next segment if it doesn't fit in
                         // the current one
      filler = next_segment - filInd;
      filInd = next_segment;
    }

    memcpy(&data[filInd], buffer, size_read);

    if (opts->megaROM)
      address = (filInd % 0x4000) + 0x8000;
    else
      address = filInd;

    if (strcasecmp(fileext.c_str(), ".AKM") == 0) {
      file.fixAKM(&data[filInd], address, size_read);
    } else if (strcasecmp(fileext.c_str(), ".AKX") == 0) {
      file.fixAKX(&data[filInd], address, size_read);
    }

    addResourceToMap(filInd, size_read, filler);

    filInd += size_read;
  }

  free(buffer);
}

void Rom::buildPT3TOOLS() {
  pt3Len = bin_pt3_bin_plet5_len;
  pt3Addr = hdrAddr + hdrLen + rtnLen + mapLen + txtLen + filLen;

  memcpy(&data[pt3Addr], bin_pt3_bin_plet5, bin_pt3_bin_plet5_len);

  memcpy(&data[mapAddr], &pt3Addr,
         2);  // pt3 address at map list (last resource)
}

void Rom::buildCompiledCode() {
  int i, t, codeLen;

  basAddr = hdrAddr + hdrLen + rtnLen + mapLen + txtLen;
  basAddr += filLen + pt3Len;

  basInd = basAddr;
  basLen = 0;

  // start of MSX BASIC code
  data[basInd] = 0;
  basInd++;
  basLen++;

  // insert dummy MAXFILES=1:CLEAR into code
  // if it has file treatments
  // if(compiler->file_support) {
  //    buildFilesLine();
  //}

  // insert dummy DEF USR into code to run compiled code
  buildAssemblyLine();

  // insert assembly code
  codeLen = compiler->write(&data[basInd], code_start);
  basLen += codeLen;
  codeSgm = (code_start + codeLen) / 0x4000;

  t = compiler->segm_total;
  if (t) {
    t = t / 2 + 1;
    for (i = 1; i < t; i++) {
      writePage[i] = true;
    }
  }
}

void Rom::buildBasicCode() {
  TokenLine *line;

  basAddr = hdrAddr + hdrLen + rtnLen + mapLen + txtLen;
  basAddr += filLen + pt3Len;

  basInd = basAddr;
  basLen = 0;

  // start of MSX BASIC code
  data[basInd] = 0;
  basInd++;
  basLen++;

  // insert dummy MAXFILES=1:CLEAR into code
  // if it has file treatments
  if (tokenizer->open_cmd) {
    buildFilesLine();
  }

  // insert dummy CALL TURBO ON into code
  // if its not included yet
  if (opts->turbo && !tokenizer->turbo_mode) {
    buildTurboLine();
  }

  // calculate lines address
  for (unsigned int i = 0; i < tokenizer->lines.size(); i++) {
    line = tokenizer->lines[i];
    if (line->data[0] == 143 || (line->data[0] == ':' && line->data[1] == 143))
      if (!opts->noStripRemLines &&
          (line->data[2] != '#' && line->data[3] != '#'))
        continue;
    calcBasicLineAddress(line);
  }

  // save data
  for (unsigned int i = 0; i < tokenizer->lines.size(); i++) {
    line = tokenizer->lines[i];
    if (line->data[0] == 143 || (line->data[0] == ':' && line->data[1] == 143))
      if (!opts->noStripRemLines &&
          (line->data[2] != '#' && line->data[3] != '#'))
        continue;
    buildBasicLine(line);
  }
}

void Rom::buildAssemblyLine() {
  TokenLine dummy;

  code_start = basInd + 16 + 6;

  dummy.address = 0;
  dummy.next = 0;
  dummy.goto_gosub = false;
  dummy.number = 0;
  dummy.length = 16;
  dummy.data[0] = 0x97;  // DEFUSR9=&H0000:X=USR9(0)
  dummy.data[1] = 0xDD;
  dummy.data[2] = 0x1A;
  dummy.data[3] = 0xEF;
  dummy.data[4] = 0x0C;
  dummy.data[5] = code_start & 0xff;  // assembly code init address (word)
  dummy.data[6] = (code_start >> 8) & 0xff;
  dummy.data[7] = 0x3A;
  dummy.data[8] = 0x58;
  dummy.data[9] = 0xEF;
  dummy.data[10] = 0xDD;
  dummy.data[11] = 0x1A;
  dummy.data[12] = 0x28;
  dummy.data[13] = 0x11;
  dummy.data[14] = 0x29;
  dummy.data[15] = 0x00;

  calcBasicLineAddress(&dummy);
  buildBasicLine(&dummy);

  basInd += 2;
}

void Rom::buildFilesLine() {
  TokenLine dummy;

  dummy.address = 0;
  dummy.next = 0;
  dummy.goto_gosub = false;
  dummy.number = 0;
  dummy.length = 7;
  dummy.data[0] = 0xCD;  // MAX
  dummy.data[1] = 0xB7;  // FILES
  dummy.data[2] = 0xEF;  // =
  dummy.data[3] = 0x12;  // 1
  dummy.data[4] = 0x3A;  // :
  dummy.data[5] = 0x92;  // CLEAR
  dummy.data[6] = 0x00;  // FILES

  calcBasicLineAddress(&dummy);
  buildBasicLine(&dummy);
}

void Rom::buildTurboLine() {
  TokenLine dummy;

  dummy.address = 0;
  dummy.next = 0;
  dummy.goto_gosub = false;
  dummy.number = 0;
  dummy.length = 10;
  strncpy((char *)&dummy.data[0], " TURBO ON", sizeof(dummy.data));
  dummy.data[0] = 0xCA;  // CALL instruction token

  calcBasicLineAddress(&dummy);
  buildBasicLine(&dummy);
}

void Rom::calcBasicLineAddress(TokenLine *line) {
  line->address = basInd;

  basInd += line->length + 4;

  line->next = basInd;

  basLen += line->length + 4;
}

void Rom::buildBasicLine(TokenLine *line) {
  int number, address, i;
  int state = 0;

  memcpy(&data[line->address], &line->next, 2);

  memcpy(&data[line->address + 2], &line->number, 2);

  // goto/gosub optimization (swap line number for address)
  if (line->goto_gosub) {
    for (i = 0; i < line->length - 1; i++) {
      switch (state) {
        // search for optimizable GOTO/GOSUBs
        case 0: {
          if (line->data[i] == 0x95 &&
              (line->data[i + 1] == 0xA6     // on error
               || line->data[i + 1] == 0xFF  // on interval / on strig
               || line->data[i + 1] == 0xCC  // on key
               || line->data[i + 1] == 0xC7  // on sprite
               || line->data[i + 1] == 0x90  // on stop
               )) {
            state = 1;  // non optimizable GOTO/GOSUB instruction
            break;
          }

          // if goto/gosub ...
          if (line->data[i + 1] == 0x0E &&
              (line->data[i] == 0x89 || line->data[i] == 0x8D)) {
            number = line->data[i + 2] | (line->data[i + 3] << 8);
            if ((address = getBasicLineAddress(number))) {
              address--;
              line->data[i + 1] = 0x0D;
              line->data[i + 2] = address & 0xFF;
              line->data[i + 3] = (address >> 8);  // & 0xFF;
              i += 4;
            }
          }
        } break;
        // search for next instruction
        case 1: {
          // if break line or ELSE instruction
          if (line->data[i] == ':' ||
              (line->data[i] == 0x3A && line->data[i + 1] == 0xA1))
            state = 0;
        } break;
      }
    }
  }

  memcpy(&data[line->address + 4], line->data, line->length);

  data[line->next] = 0;
  data[line->next + 1] = 0;
}

int Rom::getBasicLineAddress(int number) {
  TokenLine *line;
  int address = 0;

  if (tokenizer) {
    for (unsigned int i = 0; i < tokenizer->lines.size(); i++) {
      line = tokenizer->lines[i];
      if (line->number == number) {
        address = line->address;
        break;
      }
    }
  } else
    printf("ERROR searching BASIC line address\n");

  return address;
}

void Rom::buildHeaderAdjust() {
  unsigned char *header = &data[hdrAddr];
  int resource_address, resource_segment;

  memcpy(&header[14], &basAddr, 2);  // address of user basic code (BASTEXT)

  if (compiler) {
    if (compiler->pt3 && compiler->akm) {
      errorMessage = "Cannot use PT3 player and AKM player simultaneously";
      errorFound = true;
      return;
    }

    if (compiler->pt3) {
      resource_address = rtnAddr + rtnLen;
      resource_segment = 0xFF;  // music player enabled
    } else {
      if (opts->megaROM) {
        resource_address = rscAddr + 0x0010;  // resource map start address
        resource_segment = rscSgm;  // resource map segment (128kb rom)
        // https://www.msx.org/wiki/MegaROM_Mappers
        //   ASCII 8K
        //       Page (8kB)                              Switching address
        //       8000h~9FFFh (mirror: 0000h~1FFFh)       7000h (mirrors:
        //       7001h~77FFh) A000h~BFFFh (mirror: 2000h~3FFFh)       7800h
        //       (mirrors: 7801h~7FFFh)
        //   Konami MegaROMs with SCC
        //       8000h~9FFFh (mirror: 0000h~1FFFh)	    9000h (mirrors:
        //       9001h~97FFh) A000h~BFFFh (mirror: 2000h~3FFFh)	    B000h
        //       (mirrors: B001h~B7FFh)
        if (opts->compileMode == BuildOptions::CompileMode::KonamiSCC) {
          char *p = (char *)&data[xbcAddr + 0xDB];
          int mapperCount = 0;
          for (int i = 0; i < 0x4000; i++) {
            if (p[1] == 0 && (p[0] == 0x32 || p[0] == 0x3A) &&
                (p[2] == 0x70 || p[2] == 0x78)) {
              if (p[2] == 0x70) {
                p[2] = 0x90;
              } else
                p[2] = 0xB0;
              mapperCount++;
              p += 3;
            } else
              p++;
          }
          if (mapperCount != 11) {
            printf(
                "ERROR: Konami SCC ROM format adjust failed (%i locations)\n",
                mapperCount);
            errorFound = true;
          }
        }
      } else {
        resource_address = mapAddr;  // resource map start address
        resource_segment = 0;        // resource map segment (48kb rom)
      }
    }
    header[11] = resource_address & 0xFF;  // resource map start address
    header[12] = (resource_address >> 8) & 0xFF;
    header[13] = resource_segment;  // resource map segment
  }
}

void Rom::error() {
  printf("%s\n", errorMessage.c_str());
}

//----------------------------------------------------------------------------------------------

void Rom::writeRom(string filename) {
  int i, k, t;

  file.name = filename;

  if (!file.create()) {
    errorMessage = "Cannot create output file";
    errorFound = true;
    return;
  }

  t = 0x4000;
  rom_size = 0;

  for (i = 0, k = 0; i < COMPILE_MAX_PAGES; i++, k += t) {
    if (writePage[i]) {
      file.write(&data[k], t);
      rom_size += t;
    }
  }

  file.close();
}

//----------------------------------------------------------------------------------------------

void Rom::buildFontResources() {
  int font_size[] = {453, 345, 326};
  unsigned char font_data[][500] = {
      {0x3e, 0x00, 0x32, 0x00, 0x20, 0x00, 0x05, 0x96, 0x02, 0x50, 0x00, 0x92,
       0x0f, 0x06, 0xf8, 0x01, 0xa0, 0x0b, 0x11, 0x78, 0xa0, 0x70, 0x28, 0x40,
       0xf0, 0x17, 0xc0, 0xc8, 0x10, 0x20, 0x40, 0x00, 0x98, 0x18, 0x00, 0x40,
       0xa0, 0x40, 0xa8, 0x90, 0x19, 0x98, 0x60, 0x00, 0x0d, 0xdd, 0x27, 0x07,
       0x14, 0x00, 0x20, 0x10, 0x17, 0x03, 0x10, 0xe0, 0x13, 0x20, 0xa8, 0x70,
       0x20, 0x1c, 0x70, 0xa8, 0x20, 0x50, 0xf8, 0xef, 0x51, 0x38, 0x5c, 0x2c,
       0x78, 0x00, 0xf9, 0x00, 0x60, 0x4c, 0x3f, 0x00, 0x08, 0x2e, 0x00, 0x80,
       0x00, 0x3c, 0x66, 0x66, 0x00, 0xcc, 0xcc, 0x80, 0x1a, 0x04, 0x06, 0x06,
       0x00, 0x0c, 0x0c, 0x50, 0x08, 0x0f, 0x07, 0x38, 0xc0, 0xc0, 0xba, 0x0f,
       0x07, 0x0f, 0x95, 0x07, 0x24, 0x1f, 0x07, 0xd6, 0x17, 0x33, 0x9d, 0x0f,
       0x07, 0xcf, 0x2f, 0x3c, 0xad, 0x2f, 0x1f, 0xb4, 0x0f, 0x27, 0xee, 0x62,
       0x7b, 0xd3, 0x74, 0x40, 0x77, 0x18, 0x30, 0x60, 0xc0, 0x18, 0x60, 0x30,
       0x18, 0x0f, 0xf8, 0xcb, 0x01, 0x00, 0xb0, 0x0c, 0x12, 0x00, 0x70, 0x65,
       0x88, 0x7f, 0x23, 0xc4, 0x07, 0x48, 0xa8, 0xb3, 0x44, 0x18, 0x3e, 0x71,
       0x00, 0x7f, 0x45, 0x71, 0x07, 0x7e, 0x71, 0x01, 0x70, 0x02, 0x0f, 0x70,
       0x70, 0x71, 0x75, 0x3e, 0x0f, 0x00, 0xc2, 0x0f, 0x7f, 0x70, 0x7e, 0x10,
       0x75, 0x7f, 0x07, 0x05, 0x68, 0x02, 0x1f, 0x77, 0x67, 0x73, 0x1f, 0x53,
       0x35, 0x00, 0x07, 0x1c, 0x49, 0x00, 0x07, 0x0e, 0x86, 0x00, 0x4e, 0x3c,
       0x05, 0x17, 0x72, 0x7c, 0x74, 0x72, 0x17, 0x75, 0x60, 0x00, 0x37, 0x0e,
       0x0f, 0x7b, 0x7f, 0x75, 0x27, 0x13, 0x71, 0x79, 0x7d, 0x37, 0x71, 0x6f,
       0x6f, 0x1b, 0x5f, 0x7e, 0x06, 0x4f, 0x75, 0x75, 0x72, 0x3d, 0xc7, 0x0f,
       0x73, 0x73, 0x0a, 0x0f, 0x3c, 0x0e, 0x43, 0x1f, 0x6f, 0xf1, 0x57, 0x71,
       0xe6, 0x2f, 0x0e, 0x05, 0x73, 0x76, 0x7c, 0x07, 0x1c, 0x75, 0x7f, 0x7b,
       0x4f, 0x3a, 0x9e, 0x1d, 0x3a, 0x70, 0x07, 0x27, 0x7f, 0x0e, 0x1c, 0x53,
       0x38, 0x6f, 0x76, 0x40, 0xa9, 0x00, 0x57, 0x00, 0x80, 0x92, 0x9a, 0x08,
       0x0f, 0x74, 0x10, 0x00, 0x0f, 0x20, 0x3e, 0x50, 0x88, 0x86, 0xb9, 0x93,
       0xef, 0x1d, 0x24, 0x0b, 0x89, 0xf8, 0x88, 0xe0, 0x1b, 0xf0, 0x48, 0x70,
       0x48, 0x74, 0xf0, 0xae, 0x80, 0x9a, 0x00, 0x78, 0x3a, 0x0f, 0x48, 0x0f,
       0x2b, 0x0f, 0x80, 0xf0, 0x80, 0xf8, 0x1a, 0x07, 0x80, 0x1c, 0x1f, 0xb8,
       0x88, 0x5f, 0x88, 0xf2, 0x37, 0xee, 0xae, 0x00, 0x1f, 0x1b, 0x38, 0x08,
       0x08, 0x0e, 0x17, 0x90, 0xe0, 0x90, 0x17, 0xce, 0x46, 0x37, 0x8e, 0x0f,
       0xd8, 0xa8, 0x27, 0x86, 0x07, 0xc8, 0xa8, 0x98, 0x95, 0x2f, 0x0d, 0x39,
       0xcd, 0x5f, 0x88, 0xab, 0x4f, 0x0f, 0xd8, 0x74, 0xe8, 0x07, 0x15, 0xa0,
       0x68, 0x90, 0x5f, 0x70, 0x3c, 0x08, 0xf8, 0x57, 0xe3, 0xa3, 0x3c, 0x2e,
       0x6f, 0x1d, 0x90, 0xa0, 0x40, 0x07, 0x34, 0x2f, 0xd8, 0x4f, 0x47, 0x50,
       0xd3, 0x9d, 0x07, 0x27, 0x67, 0x2f, 0xd7, 0x32, 0x37, 0x18, 0xfb, 0xab,
       0x02, 0xf7, 0xb9, 0x13, 0x03, 0xc0, 0x2a, 0x03, 0x10, 0x02, 0xf7, 0x1a,
       0x40, 0xa8, 0x10, 0x1f, 0xed, 0xff, 0xff, 0xff, 0xf8},
      {0x3e, 0x00, 0x00, 0x00, 0x06, 0x0e, 0x0c, 0x18, 0x00, 0x60, 0x60, 0x2d,
       0x00, 0x6c, 0x00, 0x26, 0x0f, 0x06, 0xfe, 0x40, 0x01, 0x0b, 0x10, 0x7c,
       0xd0, 0x7c, 0x16, 0x20, 0x7c, 0x10, 0x10, 0x62, 0x64, 0x08, 0x10, 0x00,
       0x26, 0x46, 0x00, 0x70, 0x88, 0x50, 0x20, 0x54, 0x23, 0x88, 0x76, 0x0f,
       0x18, 0x30, 0x4b, 0x27, 0x06, 0xab, 0x00, 0x3a, 0x02, 0xb0, 0x00, 0x13,
       0x24, 0x18, 0x1d, 0x7e, 0x18, 0x24, 0x18, 0x2d, 0x07, 0x7e, 0x03, 0x96,
       0x23, 0x18, 0x3a, 0x5e, 0x06, 0x0f, 0x74, 0x6b, 0x16, 0x6f, 0x1c, 0x01,
       0x38, 0x70, 0xe0, 0xc0, 0x00, 0x7c, 0xc6, 0xc9, 0x00, 0x7c, 0x13, 0x38,
       0xd4, 0x3f, 0x21, 0x0f, 0x0e, 0x0a, 0x3c, 0x78, 0xe0, 0xfe, 0x2c, 0x8e,
       0x0c, 0x3c, 0x06, 0x17, 0x00, 0x1c, 0x2c, 0x4c, 0x8c, 0xfe, 0x0c, 0x0c,
       0x00, 0x0e, 0xfe, 0xc0, 0xfc, 0x06, 0x0f, 0x2e, 0x3c, 0x60, 0x08, 0x2f,
       0x1c, 0xfe, 0xc6, 0x0c, 0x7a, 0x00, 0x01, 0x78, 0xc4, 0xe4, 0x7c, 0x9e,
       0x86, 0x7c, 0xc1, 0x47, 0x7e, 0x06, 0x0c, 0x78, 0xdd, 0x5c, 0x5f, 0xe5,
       0x07, 0x77, 0x88, 0x25, 0x60, 0x9b, 0xbb, 0x3f, 0x7d, 0x8c, 0x80, 0x0b,
       0xa8, 0x26, 0x13, 0x47, 0x66, 0x66, 0xac, 0x06, 0x21, 0x37, 0x48, 0xde,
       0x00, 0xc0, 0x3f, 0x38, 0x6c, 0x93, 0x40, 0xfe, 0x02, 0x00, 0x3b, 0x5c,
       0x02, 0x10, 0x1f, 0xc0, 0x00, 0x66, 0x3c, 0x00, 0x30, 0xf8, 0xcc, 0x9e,
       0x32, 0xcc, 0xf8, 0x7f, 0x77, 0x9e, 0x02, 0xfe, 0x42, 0x07, 0xb7, 0x3e,
       0x87, 0x00, 0xce, 0xc6, 0x66, 0x3e, 0x00, 0xc6, 0xea, 0x36, 0x37, 0x2f,
       0xd0, 0xff, 0x91, 0x2f, 0x0e, 0xa5, 0xa1, 0xa7, 0x00, 0x33, 0xd8, 0xf0,
       0xf8, 0xdc, 0xce, 0x00, 0xc0, 0xea, 0x00, 0x37, 0x0f, 0x0a, 0xee, 0xfe,
       0xfe, 0xd6, 0x27, 0x07, 0x0b, 0xe6, 0xf6, 0xfe, 0xde, 0x39, 0xe3, 0xf7,
       0xac, 0x6f, 0x6d, 0x4f, 0xe1, 0x0f, 0xde, 0xcc, 0x7a, 0xa7, 0x0f, 0x51,
       0x37, 0x0f, 0x94, 0xd8, 0x47, 0xc3, 0xf7, 0x00, 0x77, 0x67, 0x2f, 0x06,
       0x07, 0x44, 0x6c, 0x38, 0x10, 0x25, 0x07, 0xd6, 0x50, 0xee, 0x4f, 0x0c,
       0x57, 0x7c, 0x38, 0x7c, 0x07, 0x87, 0xce, 0x24, 0x11, 0x82, 0x9f, 0xa3,
       0xd7, 0x34, 0x6f, 0x3c, 0x97, 0x90, 0x3c, 0x7f, 0xe0, 0x70, 0x38, 0x44,
       0x1c, 0x94, 0x0f, 0x0c, 0xe9, 0x00, 0x0f, 0x18, 0x0f, 0xd8, 0x00, 0xbc,
       0x00, 0x90, 0x74, 0x0a, 0xff, 0xff, 0xff, 0xff, 0xc0},
      {0x3f, 0x00, 0x01, 0x00, 0x0c, 0x18, 0x30, 0x20, 0x00, 0x40, 0x0d, 0x07,
       0x66, 0x66, 0x22, 0x06, 0x0f, 0x24, 0x7e, 0x24, 0x00, 0x02, 0x00, 0x10,
       0x7c, 0xd0, 0x7c, 0x16, 0x7c, 0x40, 0x10, 0x0f, 0x62, 0x64, 0x08, 0x10,
       0x26, 0x00, 0x46, 0x00, 0x70, 0x88, 0x50, 0x20, 0x54, 0x88, 0x56, 0x76,
       0x0f, 0x2e, 0x99, 0x26, 0x35, 0x09, 0x00, 0x30, 0x18, 0x00, 0x02, 0x08,
       0x70, 0x00, 0x13, 0x24, 0x18, 0x7e, 0x29, 0x18, 0x24, 0x18, 0x38, 0x10,
       0x5b, 0x35, 0x36, 0x5c, 0x23, 0x1b, 0x09, 0x3c, 0xfa, 0x6a, 0x0f, 0x98,
       0x03, 0x06, 0x6f, 0x60, 0x00, 0xc0, 0x80, 0x00, 0xfe, 0x86, 0x8a, 0x92,
       0xa2, 0x23, 0xc2, 0xfe, 0x2e, 0x30, 0x10, 0xa0, 0x00, 0x0f, 0x82, 0x02,
       0xfe, 0x80, 0x38, 0x80, 0xfe, 0x07, 0x3e, 0x02, 0x52, 0x82, 0x07, 0x0b,
       0x88, 0x00, 0x5d, 0xfe, 0x5a, 0x14, 0xdc, 0x0f, 0x07, 0x82, 0xe1, 0x07,
       0x02, 0x02, 0x04, 0x47, 0x55, 0xb1, 0x77, 0x0c, 0x0f, 0x2e, 0x04, 0x02,
       0x3a, 0x5c, 0xef, 0x5f, 0x2e, 0x07, 0x21, 0x66, 0x86, 0x9b, 0x0c, 0x34,
       0x10, 0x7e, 0x02, 0xce, 0x0b, 0x7a, 0xe5, 0x2f, 0x0c, 0x6d, 0x08, 0x01,
       0x7c, 0xc4, 0x9c, 0x00, 0xc0, 0x7c, 0x00, 0x00, 0x02, 0x06, 0x0a, 0x12,
       0x3e, 0x42, 0x82, 0x06, 0x00, 0xf8, 0x84, 0x82, 0xfc, 0x76, 0x02, 0x67,
       0x44, 0x00, 0x4f, 0xf0, 0x88, 0x0d, 0x84, 0x39, 0x88, 0xf0, 0x0f, 0xfc,
       0xdb, 0x0f, 0x39, 0x07, 0x80, 0x07, 0x9e, 0xd7, 0x77, 0x03, 0x51, 0x7f,
       0x37, 0xbd, 0xe0, 0xbf, 0x08, 0xe5, 0x00, 0xf8, 0x17, 0x32, 0x34, 0x90,
       0x3a, 0xbf, 0x3c, 0x47, 0x0e, 0x82, 0xc6, 0xaa, 0x92, 0x27, 0x02, 0x82,
       0xc2, 0xa2, 0x92, 0x8a, 0x86, 0x07, 0xef, 0x34, 0x07, 0xb7, 0x9e, 0x74,
       0x4f, 0x0f, 0x18, 0xfa, 0xd3, 0x0f, 0xb3, 0x37, 0x5f, 0x8b, 0xcf, 0x07,
       0xc7, 0x57, 0x82, 0x9c, 0x2f, 0x57, 0xa0, 0xc4, 0xb7, 0x89, 0x99, 0x04,
       0xaa, 0xaa, 0xcc, 0xcc, 0x88, 0x0f, 0x44, 0x07, 0x28, 0x10, 0x28, 0x44,
       0x82, 0x6d, 0x07, 0x7f, 0xca, 0x95, 0x40, 0x27, 0x68, 0x1c, 0x36, 0x1c,
       0x1c, 0x00, 0xc0, 0x60, 0xf1, 0x0d, 0x02, 0x00, 0x38, 0x04, 0x97, 0x38,
       0x93, 0x9a, 0x3c, 0x66, 0x97, 0xf2, 0x8e, 0x90, 0x9f, 0x09, 0xff, 0xff,
       0xff, 0xf8}};

  for (int i = 2; i >= 0; i--) {
    filInd = txtInd;

    memcpy(&data[filInd], &font_data[i], font_size[i]);
    memcpy(&data[mapInd], &filInd, 2);
    mapInd += 2;
    memcpy(&data[mapInd], &font_size[i], 2);
    mapInd += 2;

    txtInd += font_size[i];
    filLen += font_size[i];
  }
}
