/***
 * @file resources.cpp
 * @brief Resources class implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "resources.h"

///-------------------------------------------------------------------------------

void ResourceManager::clear() {
  resourceList.clear();
  fileList.clear();
}

void ResourceManager::addDataResource(Parser *parser) {
  Lexeme *lexeme = new Lexeme();
  lexeme->name = "_DATA_";
  lexeme->value = lexeme->name;
  resourceList.push_back(lexeme);

  ResourceDataReader *resourceReader = new ResourceDataReader(parser);
  if (resourceReader) {
    resourceReaderList.push_back(resourceReader);
    resourceReader->load();
  }
}

bool ResourceManager::add(string filename) {
  /// @todo implement inputPath alternative search
  // if file not found, try search it at input path
  // if (!fileExists(filename)) {
  //  filename = pathJoin(opts->inputPath, filename);
  //}

  ResourceReader *resourceReader = ResourceFactory::create(filename);
  if (resourceReader) {
    resourceReaderList.push_back(resourceReader);
    return resourceReader->load();
  }
  return false;
}

///-------------------------------------------------------------------------------

ResourceReader *ResourceFactory::create(string filename) {
  if (fileExists(filename)) {
    string fileext = getFileExtension(filename);
    if (ResourceTxtReader::isIt(fileext)) {
      return new ResourceTxtReader(filename);
    }
    if (ResourceCsvReader::isIt(fileext)) {
      return new ResourceCsvReader(filename);
    }
    if (ResourceScrReader::isIt(fileext)) {
      return new ResourceScrReader(filename);
    }
    if (ResourceSprReader::isIt(fileext)) {
      return new ResourceSprReader(filename);
    }
    if (ResourceAkmReader::isIt(fileext)) {
      return new ResourceAkmReader(filename);
    }
    if (ResourceAkxReader::isIt(fileext)) {
      return new ResourceAkxReader(filename);
    }
    if (ResourceMtfReader::isIt(fileext)) {
      return new ResourceMtfReader(filename);
    }
    if (ResourceBlobReader::isIt(fileext)) {
      return new ResourceBlobReader(filename);
    }
  }
  return nullptr;
}

///-------------------------------------------------------------------------------

string ResourceReader::getFilename() {
  return filename;
};

ResourceReader::ResourceReader(string filename) {
  this->filename = filename;
};

///-------------------------------------------------------------------------------

ResourceBlobReader::ResourceBlobReader(string filename)
    : ResourceReader(filename) {};

bool ResourceBlobReader::populateBuffer() {
  return false;
}

bool ResourceBlobReader::isIt(string fileext) {
  return true;
}

bool ResourceBlobReader::load() {
  return false;
}

///-------------------------------------------------------------------------------

ResourceBlobPackedReader::ResourceBlobPackedReader(string filename)
    : ResourceBlobReader(filename) {};

bool ResourceBlobPackedReader::isIt(string fileext) {
  return true;
}

bool ResourceBlobPackedReader::load() {
  return false;
}

///-------------------------------------------------------------------------------

ResourceBlobChunkPackedReader::ResourceBlobChunkPackedReader(string filename)
    : ResourceBlobPackedReader(filename) {};

bool ResourceBlobChunkPackedReader::isIt(string fileext) {
  return true;
}

bool ResourceBlobChunkPackedReader::load() {
  return false;
}

///-------------------------------------------------------------------------------

ResourceTxtReader::ResourceTxtReader(string filename)
    : ResourceReader(filename) {};

bool ResourceTxtReader::populateLines() {
  FILE *file;
  char line[2048];  //! 1 line = 2k of data
  int size, i;

  lines.clear();

  if ((file = fopen(filename.c_str(), "r"))) {
    while (fgets(line, sizeof(line), file)) {
      size = strlen(line);
      for (i = 0; i < size; i++) {
        if (line[i] < 32 || line[i] > 127) line[i] = 32;
      }
      lines.push_back(line);
    }

    fclose(file);

    return true;
  }

  return false;
}

bool ResourceTxtReader::isIt(string fileext) {
  return (strcasecmp(fileext.c_str(), ".TXT") == 0);
}

bool ResourceTxtReader::load() {
  int i, n, lineCount, lineSize;
  data.clear();
  if (populateLines()) {
    lineCount = lines.size();
    data.resize(lineCount + 1);
    data[0].push_back(2);  //! TXT data resource
    for (i = 0, n = 1; i < lineCount; i++, n++) {
      /// string maximum size = 255 chars
      lineSize = (lines[i].size() & 0xFF);
      /// set string size
      data[n].resize(lineSize + 1);
      data[n][0] = lineSize;
      /// copy string text (max = 255 chars)
      memcpy(data[n].data() + 1, lines[i].data(), lineSize);
    }
    return true;
  }
  return false;
}

///-------------------------------------------------------------------------------

ResourceCsvReader::ResourceCsvReader(string filename)
    : ResourceTxtReader(filename) {};

bool ResourceCsvReader::parseFields(string line) {
  int i, lineSize, state;
  string field;

  fields.clear();
  field.clear();

  lineSize = line.size();
  state = 0;

  for (i = 0; i < lineSize; i++) {
    switch (state) {
      case 0: {
        if (line[i] == '"') {
          state = 1;
        } else if (line[i] == ',' || line[i] == ';' || line[i] == 8) {
          fields.push_back(fixField(field));
          field.clear();
        } else if (line[i] == ' ' && field.size() == 0) {
        } else {
          field.push_back(line[i]);
        }
      } break;
      case 1: {
        if (line[i] == '"') {
          state = 2;
        } else {
          field.push_back(line[i]);
        }
      } break;
      case 2: {
        if (line[i] == ',' || line[i] == ';' || line[i] == 8) {
          state = 0;
          fields.push_back(fixField(field));
          field.clear();
        }
      }
    }
  }

  if (!field.empty()) {
    fields.push_back(fixField(field));
  }

  return true;
}

string ResourceCsvReader::fixField(string field) {
  string s = field;
  int k;

  if (!s.empty()) {
    // Remove leading quote
    if (s.front() == '"') {
      s.erase(s.begin());
      if (s.empty()) return s;
    }

    // Remove trailing quote
    if (s.back() == '"') {
      s.pop_back();
      if (s.empty()) return s;
    }

    /// convert non base 10 numbers to base 10
    if (s.front() == '&') {
      try {
        if (s[1] == 'h' || s[1] == 'H')
          k = stoi(s.substr(2), 0, 16);
        else if (s[1] == 'o' || s[1] == 'O')
          k = stoi(s.substr(2), 0, 8);
        else if (s[1] == 'b' || s[1] == 'B')
          k = stoi(s.substr(2), 0, 2);
        else
          k = 0;
      } catch (exception &e) {
        k = 0;
      }
      s = to_string(k);
    }
  }

  return s;
}

bool ResourceCsvReader::isIt(string fileext) {
  return (strcasecmp(fileext.c_str(), ".CSV") == 0);
}

bool ResourceCsvReader::load() {
  int i, n, k, lineCount, fieldCount, fieldSize;
  data.clear();
  if (populateLines()) {
    lineCount = lines.size() & 0xFFFF;
    data.resize(1);
    /// CSV data resource type
    data[0].push_back(1);
    /// line count
    data[0].push_back(lineCount & 0xFF);
    data[0].push_back((lineCount >> 8) & 0xFF);
    for (i = 0, n = data.size(); i < lineCount; i++) {
      if (parseFields(lines[i])) {
        /// lines fields map
        fieldCount = fields.size() & 0xFF;
        data[0].push_back(fieldCount);
        /// lines data
        data.resize(n + fieldCount);
        for (k = 0; k < fieldCount; k++, n++) {
          /// field maximum size = 255 chars
          fieldSize = (fields[k].size() & 0xFF);
          /// set field size
          data[n].resize(fieldSize + 1);
          data[n][0] = fieldSize;
          /// copy string text (max = 255 chars)
          memcpy(data[n].data() + 1, fields[k].data(), fieldSize);
        }
      }
    }
    return true;
  }
  return false;
}

///-------------------------------------------------------------------------------

ResourceScrReader::ResourceScrReader(string filename)
    : ResourceBlobChunkPackedReader(filename) {};

bool ResourceScrReader::isIt(string fileext) {
  return (strcasecmp(fileext.c_str(), ".SC0") == 0 ||
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
          strcasecmp(fileext.c_str(), ".S12") == 0);
}

/* inherited from base class
bool ResourceScrReader::load() {
  return false;
}
*/

///-------------------------------------------------------------------------------

ResourceSprReader::ResourceSprReader(string filename)
    : ResourceBlobPackedReader(filename) {};

bool ResourceSprReader::parseTinySpriteFile() {
  return false;
}

bool ResourceSprReader::isIt(string fileext) {
  return (strcasecmp(fileext.c_str(), ".SPR") == 0);
}

bool ResourceSprReader::load() {
  return false;
}

int ResourceSprReader::ParseTinySpriteFile(string filename, unsigned char *data,
                                           int maxlen) {
  FILE *file;
  char line[255];
  int len, size_read, state;
  int sprite_type = 0;
  int sprite_x = 0, sprite_y = 0, sprite_c = 0;
  int sprite_k, sprite_w, sprite_n;
  unsigned char sprite_data[16][4][8];
  unsigned char sprite_attr[16 * 255];
  unsigned char sprite_color[16];
  unsigned char *sprite_count = &sprite_color[0];
  bool ok = false, found;

  *sprite_count = 0;

  /// @note structure
  // for msx1 tiny sprite format file:
  //   [type=0][sprite count][sprites patterns=collection of 32 bytes][sprites
  //   colors=1 byte per sprite]
  // for msx2 format file:
  //   [type=1][sprite count][sprites patterns=collection of 32 bytes][sprites
  //   colors=16 bytes per sprite]

  if ((file = fopen(filename.c_str(), "r"))) {
    size_read = 0;
    state = 0;
    memset(sprite_attr, 0, 64);
    ok = true;

    while (fgets(line, sizeof(line), file)) {
      len = strlen(line);

      // strip CR LF of the line
      while (len) {
        if (line[len - 1] < 0x20) {
          line[len - 1] = 0;
          len--;
        } else
          break;
      }

      if (len) {
        switch (state) {
          // file header
          case 0: {
            if (strcasecmp(line, "!type") == 0) {
              state++;
            } else {
              ok = false;
            }
          } break;

          // sprite type
          case 1: {
            if (strcasecmp(line, "msx1") == 0) {
              sprite_type = 0;  // msx 1 (screen mode <= 3)
            } else if (strcasecmp(line, "msx2") == 0) {
              sprite_type = 1;  // msx 2 (screen mode >= 4)
            } else {
              ok = false;
              break;
            }
            data[0] = sprite_type;  // MSX 1
            data[1] = 0;
            sprite_count = &data[1];
            data += 2;
            size_read += 2;
            state++;
          } break;

          // slot number
          case 2: {
            if (line[0] == '#') {
              sprite_y = 0;
              memset(sprite_data, 0, 16 * 4 * 8);
              memset(sprite_color, 0, 16);
              state++;
            } else {
              ok = false;
            }
          } break;

          // sprite data
          case 3: {
            for (sprite_x = 0; sprite_x < 16; sprite_x++) {
              sprite_c = line[sprite_x];

              if (sprite_c >= '0' && sprite_c <= '9') {
                sprite_c -= '0';
              } else if (sprite_c >= 'A' && sprite_c <= 'F') {
                sprite_c -= 'A';
                sprite_c += 10;
              } else if (sprite_c >= 'a' && sprite_c <= 'f') {
                sprite_c -= 'a';
                sprite_c += 10;
              } else {
                sprite_c = 0;
              }

              if (sprite_c) {
                if (sprite_y < 8) {
                  if (sprite_x < 8) {
                    sprite_k = 0;
                  } else {
                    sprite_k = 2;
                  }
                } else {
                  if (sprite_x < 8) {
                    sprite_k = 1;
                  } else {
                    sprite_k = 3;
                  }
                }
                sprite_w = sprite_y % 8;
                sprite_n = (1 << (7 - (sprite_x % 8)));
                sprite_data[sprite_c][sprite_k][sprite_w] |= sprite_n;
                sprite_color[sprite_c] = 1;
              }
            }

            sprite_y++;

            if (sprite_y >= 16) {
              if (sprite_type) {
                // msx 2
                do {
                  found = false;
                  for (sprite_y = 0; sprite_y < 16; sprite_y++) {
                    if (sprite_y < 8) {
                      sprite_k = 0;
                    } else {
                      sprite_k = 1;
                    }
                    sprite_w = sprite_y % 8;
                    sprite_n = sprite_k + 2;
                    sprite_data[0][sprite_k][sprite_w] = 0;
                    sprite_data[0][sprite_n][sprite_w] = 0;
                    sprite_color[sprite_y] = 0;
                    for (sprite_c = 1; sprite_c < 16; sprite_c++) {
                      if (sprite_data[sprite_c][sprite_k][sprite_w] |
                          sprite_data[sprite_c][sprite_n][sprite_w]) {
                        sprite_data[0][sprite_k][sprite_w] =
                            sprite_data[sprite_c][sprite_k][sprite_w];
                        sprite_data[0][sprite_n][sprite_w] =
                            sprite_data[sprite_c][sprite_n][sprite_w];
                        sprite_data[sprite_c][sprite_k][sprite_w] = 0;
                        sprite_data[sprite_c][sprite_n][sprite_w] = 0;
                        sprite_color[sprite_y] = sprite_c;
                        found = true;
                        break;
                      }
                    }
                  }
                  if (found) {
                    sprite_n = *sprite_count * 16;
                    memcpy(&sprite_attr[sprite_n], &sprite_color[0], 16);
                    memcpy(data, sprite_data[0], 32);
                    *sprite_count += 1;
                    data += 32;
                    size_read += 32;
                  }
                } while (found);
              } else {
                // msx 1
                for (sprite_c = 0; sprite_c < 16; sprite_c++) {
                  if (sprite_color[sprite_c]) {
                    sprite_attr[*sprite_count] = sprite_c;
                    memcpy(data, sprite_data[sprite_c], 32);
                    *sprite_count += 1;
                    data += 32;
                    size_read += 32;
                  }
                }
              }
              state = 2;
            }
          } break;
        }

        if (!ok) break;
      }
    }

    fclose(file);

    if (ok) {
      if (*sprite_count) {
        if (sprite_type) {
          // msx 2
          sprite_n = *sprite_count * 16;
          memcpy(data, sprite_attr, sprite_n);
          size_read += sprite_n;
        } else {
          // msx 1
          memcpy(data, sprite_attr, *sprite_count);
          size_read += *sprite_count;
        }
      }
      return size_read;
    } else
      return 0;

  } else {
    return -1;
  }
}

///-------------------------------------------------------------------------------

ResourceAkmReader::ResourceAkmReader(string filename)
    : ResourceBlobReader(filename) {};

bool ResourceAkmReader::isIt(string fileext) {
  return (strcasecmp(fileext.c_str(), ".AKM") == 0);
}

bool ResourceAkmReader::load() {
  if (ResourceBlobReader::load()) {
    fix();
    return true;
  }
  return false;
}

void ResourceAkmReader::fix() {
  return;
}

void ResourceAkmReader::fixAKM(unsigned char *data, int address, int length) {
  int t, i, k, w, current, previous, start, tracks, linker;
  int instrumentIndexTable, arpeggioIndexTable, pitchIndexTable;
  int subsongIndexTable;
  int noteBlockIndexTable, trackIndexTable;
  bool first, track_annotation;

  /// adjust header

  instrumentIndexTable = data[0] | (data[1] << 8);
  arpeggioIndexTable = data[2] | (data[3] << 8);
  pitchIndexTable = data[4] | (data[5] << 8);

  start = instrumentIndexTable & 0xFF00;
  instrumentIndexTable -= start;

  if (arpeggioIndexTable) {
    current = arpeggioIndexTable - start + address;
    data[2] = current & 0xFF;
    data[3] = (current >> 8) & 0xFF;
  }

  if (pitchIndexTable) {
    current = pitchIndexTable - start + address;
    data[4] = current & 0xFF;
    data[5] = (current >> 8) & 0xFF;
  }

  /// adjust subsongs table

  first = true;

  for (i = 6; i < instrumentIndexTable; i += 2) {
    subsongIndexTable = data[i] | (data[i + 1] << 8);
    if (subsongIndexTable) {
      subsongIndexTable -= start;

      // printf("===> subsong 0x%02X\n", subsongIndexTable);

      if (first) {
        first = false;
        previous =
            data[subsongIndexTable - 2] |
            (data[subsongIndexTable - 1] << 8);  // adjust the last instrument
        if (previous) {                          // before first subsong
          previous -= start;
          previous += address;
          if (subsongIndexTable > 1 && subsongIndexTable < length) {
            data[subsongIndexTable - 2] = previous & 0xFF;
            data[subsongIndexTable - 1] = (previous >> 8) & 0xFF;
          }
        }
      }

      noteBlockIndexTable =
          data[subsongIndexTable] | (data[subsongIndexTable + 1] << 8);
      if (noteBlockIndexTable) {
        noteBlockIndexTable -= start;
        noteBlockIndexTable += address;
        if (subsongIndexTable >= 0 && subsongIndexTable < (length - 1)) {
          data[subsongIndexTable] = noteBlockIndexTable & 0xFF;
          data[subsongIndexTable + 1] = (noteBlockIndexTable >> 8) & 0xFF;
        }
      }

      trackIndexTable =
          data[subsongIndexTable + 2] | (data[subsongIndexTable + 3] << 8);
      if (trackIndexTable) {
        trackIndexTable -= start;

        /// read subsong linker

        tracks = 0x80;
        track_annotation = false;

        /// @brief linker state machine

        for (k = subsongIndexTable + 13; k < trackIndexTable;) {
          linker = data[k++];  //! linker code

          if (linker & 1) {  //! if speed change/end song...
            k++;             //! ...skip speed change/end song
          }

          if (linker == 1) {  //! if end of song...
            current = data[k] | (data[k + 1] << 8);
            current -= start;
            current += address;
            if (k >= 0 && k < (length - 1)) {
              data[k] = current & 0xFF;
              data[k + 1] = (current >> 8) & 0xFF;
            }
            k += 2;  //! ...skip loop address
                     // printf("   end of song\n");
          } else {
            linker >>= 1;
            if (linker & 1) {  //! if line count...
              k++;             //! ...skip line count
            }
            for (w = 0; w < 3; w++) {
              linker >>= 1;
              if (linker & 1) {  //! if transposition w...
                k++;             //! ...skip transp w
              }
              linker >>= 1;
              if (linker & 1) {     //! if channel w...
                current = data[k];  //! ...check channel w
                k++;
                if (current & 0x80) {
                  if (current >= 0x80 && current <= 0x83) {
                    track_annotation = true;
                    if (current > tracks) tracks = current;
                  }
                  // printf("   track c %i: 0x%02X\n", w, current);
                } else {
                  /// @remark
                  /// "current" value needs to be calculated to keep
                  /// the state machine loop consistency
                  /// NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores)
                  current = (current << 8) | data[k];
                  k++;
                  // printf("   track c %i: 0x%04X\n", w, current);
                }
              }
            }
          }
        }

        if (track_annotation) {
          tracks = ((tracks & 3) + 1) * 2;
          // printf("   tracks: %i\n", tracks / 2);

          t = trackIndexTable + tracks;

          for (k = trackIndexTable; k < t; k += 2) {
            current =
                data[k] | (data[k + 1] << 8);  //! adjust the track address
            if (current) {
              current -= start;
              current += address;
              if (k >= 0 && k < (length - 1)) {
                data[k] = current & 0xFF;
                data[k + 1] = (current >> 8) & 0xFF;
              }
            }
          }
        }

        trackIndexTable += address;
        if (subsongIndexTable >= 0 && subsongIndexTable < (length - 3)) {
          data[subsongIndexTable + 2] = trackIndexTable & 0xFF;
          data[subsongIndexTable + 3] = (trackIndexTable >> 8) & 0xFF;
        }
      }

      subsongIndexTable += address;
      if (i >= 0 && i < (length - 1)) {
        data[i] = subsongIndexTable & 0xFF;
        data[i + 1] = (subsongIndexTable >> 8) & 0xFF;
      }
    }
  }

  /// adjust instruments table

  t = data[instrumentIndexTable] | (data[instrumentIndexTable + 1] << 8);
  t -= start;
  first = true;

  for (i = instrumentIndexTable; i < t; i += 2) {
    current = data[i] | (data[i + 1] << 8);
    if (current) {
      current -= start;

      if (first)
        first = false;
      else {
        previous = data[current - 2] |
                   (data[current - 1] << 8);  //! adjust the previous instrument
        if (previous) {
          previous -= start;
          previous += address;
          if (current > 1 && current < length) {
            data[current - 2] = previous & 0xFF;
            data[current - 1] = (previous >> 8) & 0xFF;
          }
        }
      }

      current += address;
      if (i >= 0 && i < (length - 1)) {
        data[i] = current & 0xFF;
        data[i + 1] = (current >> 8) & 0xFF;
      }
    }
  }

  instrumentIndexTable += address;
  data[0] = instrumentIndexTable & 0xFF;
  data[1] = (instrumentIndexTable >> 8) & 0xFF;

  /// @note debugging helper code
  // FileNode *file = new FileNode();
  // file->writeToFile((char *) "song.dat", data, length);
  // delete file;
}

///-------------------------------------------------------------------------------

ResourceAkxReader::ResourceAkxReader(string filename)
    : ResourceBlobReader(filename) {};

bool ResourceAkxReader::isIt(string fileext) {
  return (strcasecmp(fileext.c_str(), ".AKX") == 0);
}

bool ResourceAkxReader::load() {
  if (ResourceBlobReader::load()) {
    fix();
    return true;
  }
  return false;
}

void ResourceAkxReader::fix() {
  return;
}

void ResourceAkxReader::fixAKX(unsigned char *data, int address, int length) {
  int t = length;
  int i = 0, current, start = 0;
  bool first = true;

  // loop the effects list

  while (i < t) {
    current = data[i] | (data[i + 1] << 8);
    if (current) {
      if (first) {
        start = current & 0xFF00;
        t = current - start;
        first = false;
      }

      current = current - start + address;

      if (i >= 0 && i < (length - 1)) {
        data[i] = current & 0xFF;
        data[i + 1] = (current >> 8) & 0xFF;
      }
    }

    i += 2;
  }

  /// @note debugging helper code
  // FileNode *file = new FileNode();
  // file->writeToFile((char *) "effect.dat", data, length);
  // delete file;
}

///-------------------------------------------------------------------------------

ResourceMtfReader::ResourceMtfReader(string filename)
    : ResourceBlobReader(filename) {};

bool ResourceMtfReader::isIt(string fileext) {
  return (strcasecmp(fileext.c_str(), ".MTF.JSON") == 0);
}

bool ResourceMtfReader::load() {
  return false;
}

///-------------------------------------------------------------------------------

ResourceStringReader::ResourceStringReader(string text)
    : ResourceReader(text) {};

bool ResourceStringReader::load() {
  int lineSize;

  data.clear();
  data.resize(1);
  /// string maximum size = 255 chars
  lineSize = filename.size();
  /// set string size
  data[0].resize(lineSize + 1);
  /// copy string text (max = 255 chars)
  memcpy(data[0].data(), filename.c_str(), lineSize);
  data[0][lineSize] = 0;

  return true;
}

///-------------------------------------------------------------------------------

ResourceDataReader::ResourceDataReader(Parser *parser)
    : ResourceCsvReader(string("_DATA_")) {
  this->parser = parser;
};

bool ResourceDataReader::load() {
  // parseLine()
  return false;
}
