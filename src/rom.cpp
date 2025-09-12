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
#include "start.h"

//----------------------------------------------------------------------------------------------

Rom::Rom() {}

Rom::~Rom() {}

bool Rom::build(Compiler *compiler) {
  float romSizeFloat;

  if (!compiler->compiled) return false;

  /// initialize data

  this->compiler = compiler;
  this->opts = compiler->opts;
  this->resourceManager = &compiler->resourceManager;

  buildInit();

  /// add kernel page

  if (!addKernel()) return false;

  /// add code page

  if (!addCompiledCode()) return false;

  // add resources pages

  if (!addResources()) return false;

  // save ROM file

  if (!writeRom(opts->outputFilename)) return false;

  // calculate shares

  romSizeFloat = romSize;
  kernelShare = (0x4000 / romSizeFloat) * 100;
  codeShare = (codeSize / romSizeFloat) * 100;
  resourcesShare = (resourcesSize / romSizeFloat) * 100;

  return true;
}

//----------------------------------------------------------------------------------------------

void Rom::buildInit() {
  pages.clear();

  codeSize = 0;
  resourcesSize = 0;
  codeShare = 0;
  resourcesShare = 0;

  errorFound = false;
}

bool Rom::addKernel() {
  /// add kernel code (support routines, Arkos Tracker and embedded fonts)
  /// @remark
  ///   Free space on kernel:
  ///     ARKOS_PLAYER (06481H) - BASIC_KUN_START_FILLER (05D2EH)
  ///     = 753H = 1875 bytes
  pages.emplace_back(0x4000, 0);  // add new page
  memcpy(pages[0].data(), bin_header_bin, bin_header_bin_len);

  /// add start code
  pages.emplace_back(0x4000, 0);  // add new page
  memcpy(pages[1].data(), bin_start_bin, bin_start_bin_len);

  if (opts->megaROM)
    if (!fixIfKonamiSCC()) return false;

  return true;
}

bool Rom::addResources() {
  int baseAddress;
  if (opts->megaROM) {
    baseAddress = 0x8000;
    resourceSegment = pages.size() * 2;  //! resource map segment (128kb rom)
  } else {
    baseAddress = 0;
    resourceSegment = 0;  //! resource map segment (48kb rom)
  }
  resourceAddress = baseAddress + 0x0010;  //! resource map start address

  if (!resourceManager->buildMap(resourceSegment, baseAddress)) {
    errorMessage = resourceManager->getErrorMessage();
    errorFound = true;
    return false;
  }

  resourcesSize = resourceManager->resourcesPackedSize;

  /// adjust resource map start address

  setResourceMapStartAddress();

  return true;
}

bool Rom::addCompiledCode() {
  vector<unsigned char> buf(COMPILE_CODE_SIZE);
  unsigned char *pbuf = buf.data();
  int copyTotalSize, copyChunkSize;

  /// write compiled code to temporary buffer
  codeSize = compiler->write(pbuf, 0x8000 + bin_start_bin_len);

  /// copy compiled code to rom pages
  if (codeSize) {
    copyChunkSize = min(codeSize, (int)(0x4000 - bin_start_bin_len));
    memcpy(pages[1].data() + bin_start_bin_len, pbuf, copyChunkSize);
    pbuf += copyChunkSize;
    copyTotalSize = codeSize - copyChunkSize;
    while (copyTotalSize) {
      pages.emplace_back(0x4000);
      copyChunkSize = min(copyTotalSize, 0x4000);
      memcpy(pages.back().data(), pbuf, copyChunkSize);
      pbuf += copyChunkSize;
      copyTotalSize -= copyChunkSize;
    }
    return true;
  } else {
    errorMessage = "Compiled code is empty!";
    errorFound = true;
    return false;
  }
}

void Rom::setResourceMapStartAddress() {
  pages[1][11] = resourceAddress & 0xFF;  // resource map start address
  pages[1][12] = (resourceAddress >> 8) & 0xFF;
  pages[1][13] = resourceSegment;  // resource map segment
}

/***
 * @remarks
 * https://www.msx.org/wiki/MegaROM_Mappers
 *   ASCII 8K
 *       Page (8kB)                              Switching address
 *       8000h~9FFFh (mirror: 0000h~1FFFh)       7000h (mirrors:
 *       7001h~77FFh) A000h~BFFFh (mirror: 2000h~3FFFh)       7800h
 *       (mirrors: 7801h~7FFFh)
 *   Konami MegaROMs with SCC
 *       8000h~9FFFh (mirror: 0000h~1FFFh)	    9000h (mirrors:
 *       9001h~97FFh) A000h~BFFFh (mirror: 2000h~3FFFh)	    B000h
 *       (mirrors: B001h~B7FFh)
 */
bool Rom::fixIfKonamiSCC() {
  if (opts->compileMode == BuildOptions::CompileMode::KonamiSCC) {
    char *p = (char *)pages[0].data() + 0xDB;
    int mapperCount = 0;
    for (int i = 0xDB; i < (0x4000 - 3); i++) {
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
      printf("ERROR: Konami SCC ROM format adjust failed (%i locations)\n",
             mapperCount);
      errorFound = true;
      return false;
    }
  }
  return true;
}

void Rom::error() {
  printf("%s\n", errorMessage.c_str());
}

//----------------------------------------------------------------------------------------------

bool Rom::writeRom(string filename) {
  ofstream file(filename, ios::binary);
  vector<unsigned char> fillerPage(0x4000, 0);
  int i, pageCount, fillerCount;

  if (!file) {
    errorMessage = "Cannot create output file: " + filename;
    errorFound = true;
    return false;
  }

  romSize = 0;
  pageCount = 0;

  /// write plain ROM resources page
  if (!opts->megaROM) {
    if ((pageCount = (int)resourceManager->pages.size())) {
      if (pageCount > 1) {
        errorMessage =
            "Resources exceeded 16k plain ROM limit\n"
            "Try to compile it in MegaROM format by adding the -x parameter";
        errorFound = true;
        file.close();
        remove(filename.c_str());
        return false;
      }
      file.write((char *)resourceManager->pages[0].data(),
                 resourceManager->pages[0].size());
      romSize += resourceManager->pages[0].size();
    }
  }

  /// write code pages
  for (i = 0; i < (int)pages.size(); i++) {
    file.write((char *)pages[i].data(), pages[i].size());
    romSize += pages[i].size();
    pageCount++;
  }

  /// write MegaROM resources pages
  if (opts->megaROM) {
    for (i = 0; i < (int)resourceManager->pages.size(); i++) {
      file.write((char *)resourceManager->pages[i].data(),
                 resourceManager->pages[i].size());
      romSize += resourceManager->pages[i].size();
      pageCount++;
    }
    /// write filler pages if necessary
    fillerCount = (pageCount % 8);
    if (fillerCount) {
      fillerCount = 8 - fillerCount;
      while (fillerCount--) {
        file.write((char *)fillerPage.data(), fillerPage.size());
        romSize += fillerPage.size();
      }
    }
  }

  file.close();

  return true;
}
