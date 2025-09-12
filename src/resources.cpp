/***
 * @file resources.cpp
 * @brief Resources class implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "resources.h"

///-------------------------------------------------------------------------------

void ResourceManager::clear() {
  resources.clear();
  pages.clear();
}

void ResourceManager::print() {
  for (int i = 0; i < (int)resources.size(); i++)
    printf("Resource #%i: %s\n", i, resources[i]->getFilename().c_str());
}

const string ResourceManager::getErrorMessage() {
  return errorMessage;
}

bool ResourceManager::addFile(string filename, string inputPath) {
  /// if file not found, try search it at input path
  if (!fileExists(filename)) {
    filename = pathJoin(inputPath, filename);
  }

  /// create the resource and add to the list
  ResourceReader *resourceReader = ResourceFactory::create(filename);
  if (resourceReader) {
    resources.push_back(resourceReader);
    return true;
  }
  errorMessage = "Resource type not recognized: " + filename;
  return false;
}

void ResourceManager::addText(string text) {
  ResourceStringReader *resourceReader = new ResourceStringReader(text);
  if (resourceReader) {
    resources.push_back(resourceReader);
  }
}

void ResourceManager::addDataResource(Parser *parser) {
  ResourceDataReader *resourceReader = new ResourceDataReader(parser);
  if (resourceReader) {
    resources.push_back(resourceReader);
  }
}

void ResourceManager::addIDataResource(Parser *parser) {
  ResourceIDataReader *resourceReader = new ResourceIDataReader(parser);
  if (resourceReader) {
    resources.push_back(resourceReader);
  }
}

bool ResourceManager::buildMap(int baseSegment, int baseAddress) {
  ResourceReader *resourceReader;
  int mapAddress = 0x0010;
  int resourceSegment = baseSegment;
  int resourceAddress = mapAddress + 2 + resources.size() * 5;
  int resourceOffset;
  int resourceSize, resourceItemSize;
  int i, k;

  resourcesUnpackedSize = resourceAddress;  //! include resource map size
  resourcesPackedSize = resourceAddress;

  pages.clear();

  if (resources.size()) {
    if (resourceAddress > 0x4000) {
      errorMessage =
          "Resources maximum limit exceeded: " + to_string(resources.size());
      return false;
    }

    /// add a new page
    pages.emplace_back(0x4000, 0xFF);
    memset(pages[0].data(), 0, 16);
    /// resource count
    pages[0][mapAddress++] = (resources.size() & 0xFF);
    pages[0][mapAddress++] = ((resources.size() >> 8) & 0xFF);

    /// write resources to pages
    for (i = 0; i < (int)resources.size(); i++) {
      /// next resource item
      resourceReader = resources[i];
      if (!resourceReader->load()) {
        errorMessage = resourceReader->getErrorMessage();
        return false;
      }

      /// add resource data
      resourceSize = 0;
      for (k = 0; k < (int)resourceReader->data.size(); k++) {
        resourceItemSize = resourceReader->data[k].size();
        if (resourceItemSize > 0x4000) {
          errorMessage = "Resource file size exceeds maximum limit (16kb): " +
                         resourceReader->getFilename();
          return false;
        }
        if ((resourceAddress + resourceItemSize) > 0x4000) {
          pages.emplace_back(0x4000, 0xFF);  //! add a new page
          resourceSegment += 2;
          resourceAddress = 0;
        }
        if (k == 0) {
          // remap resource address
          if (!resourceReader->remapTo(i, resourceSegment,
                                       baseAddress + resourceAddress)) {
            errorMessage = resourceReader->getErrorMessage();
            return false;
          }
          /// add resource map item
          resourceOffset = (resourceAddress + baseAddress);
          pages[0][mapAddress++] = resourceOffset & 0xFF;
          pages[0][mapAddress++] = (resourceOffset >> 8) & 0xFF;
          pages[0][mapAddress++] = resourceSegment & 0xFF;
        }
        /// copy resource data
        memcpy(pages.back().data() + resourceAddress,
               resourceReader->data[k].data(), resourceItemSize);
        resourceSize += resourceItemSize;
        resourceAddress += resourceItemSize;
      }
      /// resource map item size
      resourcesPackedSize += resourceSize;
      resourcesUnpackedSize += resourceReader->unpackedSize;
      resourceSize = min(resourceSize, 0xFFFF);
      pages[0][mapAddress++] = resourceSize & 0xFF;
      pages[0][mapAddress++] = (resourceSize >> 8) & 0xFF;
    }
    if (resourcesUnpackedSize) {
      packedRate = resourcesPackedSize;
      packedRate /= resourcesUnpackedSize;
      packedRate = 1 - packedRate;
      packedRate *= 100;
    } else
      packedRate = 0;
  }

  return true;
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

const string ResourceReader::getErrorMessage() {
  return errorMessage;
}

const string ResourceReader::getFilename() {
  return filename;
}

bool ResourceReader::remapTo(int index, int mappedSegm, int mappedAddress) {
  return true;
};

ResourceReader::ResourceReader(string filename) {
  this->filename = filename;
  this->packedSize = 0;
  this->unpackedSize = 0;
};

///-------------------------------------------------------------------------------

ResourceBlobReader::ResourceBlobReader(string filename)
    : ResourceReader(filename) {};

bool ResourceBlobReader::isIt(string fileext) {
  return true;
}

bool ResourceBlobReader::load() {
  ifstream fileStream(filename, ios::binary | ios::ate);

  data.clear();
  unpackedSize = 0;
  packedSize = 0;

  if (!fileStream) {
    errorMessage = "Resource file not found: " + filename;
    return false;
  }

  streamsize size = fileStream.tellg();
  fileStream.seekg(0, ios::beg);

  if (size <= 0) {
    errorMessage = "Resource is empty: " + filename;
    return false;
  }

  data.emplace_back(size);
  if (!fileStream.read((char *)data.back().data(), size)) {
    errorMessage = "Error reading resource: " + filename;
    return false;
  }

  unpackedSize = size;
  packedSize = size;

  return true;
}

///-------------------------------------------------------------------------------

ResourceBlobPackedReader::ResourceBlobPackedReader(string filename)
    : ResourceBlobReader(filename) {};

bool ResourceBlobPackedReader::pack() {
  Pletter pletter;
  int bytesPacked;
  if (data[0].size() > 0x2000) {
    errorMessage = "Resource size > 8k: " + filename;
    return false;
  }
  data.emplace_back(data[0].size() * 2);
  bytesPacked = pletter.pack(data[0].data(), data[0].size(), data[1].data());
  if (!bytesPacked) {
    errorMessage =
        "Error while packing resource file with pletter: " + filename;
    return false;
  }
  data.erase(data.begin());     //! removes the uncompressed data
  data[0].resize(bytesPacked);  //! fix the compressed data size
  packedSize = bytesPacked;
  return true;
}

bool ResourceBlobPackedReader::isIt(string fileext) {
  return true;
}

bool ResourceBlobPackedReader::load() {
  if (ResourceBlobReader::load()) {
    return pack();
  }
  return false;
}

///-------------------------------------------------------------------------------

ResourceBlobChunkPackedReader::ResourceBlobChunkPackedReader(string filename)
    : ResourceBlobPackedReader(filename) {};

bool ResourceBlobChunkPackedReader::isIt(string fileext) {
  return true;
}

/// @todo fix 1st block segment disalignment bug
bool ResourceBlobChunkPackedReader::load() {
  Pletter pletter;
  unsigned char buffer[1024];
  unsigned char *srcBuf;
  int bytesPacked, bytesUnpacked;
  int srcSize, blockCount = 0;
  if (ResourceBlobReader::load()) {
    packedSize = 0;
    srcBuf = data[0].data();
    srcSize = data[0].size();
    while (srcSize) {
      bytesUnpacked = min(srcSize, 200);
      bytesPacked = pletter.pack(srcBuf, bytesUnpacked, buffer);
      if (bytesPacked <= 0) {
        errorMessage =
            "Error while packing resource file with pletter: " + filename;
        return false;
      }
      if (bytesPacked > 255) {
        errorMessage =
            "Block size > 255 bytes (#" + std::to_string(blockCount) +
            ") while packing resource file with pletter: " + filename;
        return false;
      }
      srcBuf += bytesUnpacked;
      srcSize -= bytesUnpacked;
      blockCount++;
      data.emplace_back(bytesPacked + 1);  // create new block
      data.back()[0] = bytesPacked;
      memcpy(data.back().data() + 1, buffer, bytesPacked);
      packedSize += bytesPacked + 1;
    }
    data[0].resize(2);  //! block count
    data[0][0] = blockCount & 0xFF;
    data[0][1] = (blockCount >> 8) & 0xFF;
    packedSize += 2;
    return true;
  }
  return false;
}

///-------------------------------------------------------------------------------

ResourceTxtReader::ResourceTxtReader(string filename)
    : ResourceReader(filename) {};

bool ResourceTxtReader::populateLines() {
  ifstream fileStream(filename);
  if (!fileStream) {
    errorMessage = "Resource file not found: " + filename;
    return false;
  }

  lines.clear();
  string line;

  while (getline(fileStream, line)) {
    // Replace non-printable / non-ASCII characters with spaces
    for (char &ch : line) {
      if (ch < 32 || ch > 127) {
        ch = ' ';
      }
    }
    lines.push_back(line);
  }

  return true;
}

bool ResourceTxtReader::isIt(string fileext) {
  return (strcasecmp(fileext.c_str(), ".TXT") == 0);
}

const vector<string> ResourceTxtReader::getLines() {
  return lines;
}

/// @todo fix 1st block segment disalignment bug
bool ResourceTxtReader::load() {
  int i, lineCount, lineSize;
  data.clear();
  if (populateLines()) {
    unpackedSize = 1;
    packedSize = 1;
    lineCount = lines.size();
    data.emplace_back(1);
    data.back()[0] = 2;  //! TXT data resource
    for (i = 0; i < lineCount; i++) {
      /// string maximum size = 255 chars
      lineSize = min((int)lines[i].size(), 255);
      /// add new string element
      data.emplace_back(lineSize + 1);
      data.back()[0] = lineSize;
      /// copy string text (max = 255 chars)
      memcpy(data.back().data() + 1, lines[i].c_str(), lineSize);
      packedSize += lineSize + 1;
      unpackedSize += lineSize + 1;
    }
    return true;
  }
  return false;
}

///-------------------------------------------------------------------------------

ResourceCsvReader::ResourceCsvReader(string filename)
    : ResourceTxtReader(filename) {};

void ResourceCsvReader::addFields(string line) {
  enum class State { Normal, Quoted, QuoteInQuoted };
  string field;
  State state = State::Normal;

  auto pushField = [&]() {
    lineFields.back().push_back(fixFieldValue(field));
    field.clear();
  };

  lineFields.emplace_back(0);

  for (char ch : line) {
    switch (state) {
      case State::Normal:
        if (ch == '"') {
          state = State::Quoted;
        } else if (ch == ',' || ch == ';' || ch == '\b') {
          pushField();
        } else if (!(ch == ' ' && field.empty())) {
          field.push_back(ch);
        }
        break;

      case State::Quoted:
        if (ch == '"') {
          state = State::QuoteInQuoted;  // possible escaped quote or end of
                                         // quoted field
        } else {
          field.push_back(ch);
        }
        break;

      case State::QuoteInQuoted:
        if (ch == '"') {
          field.push_back('"');   // escaped quote
          state = State::Quoted;  // back to inside quoted field
        } else if (ch == ',' || ch == ';' || ch == '\b') {
          pushField();
          state = State::Normal;  // end of quoted field
        } else {
          // Any other character after closing quote is ignored (common CSV
          // behavior)
          state = State::Normal;
        }
        break;
    }
  }

  if (!field.empty()) {
    pushField();
  }

  return;
}

string ResourceCsvReader::fixFieldValue(string field) {
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

bool ResourceCsvReader::populateFields() {
  int lineCount, i;
  /// CSV resource type
  resourceType = 1;
  /// CSV resource data
  lineFields.clear();
  lineNumbers.clear();
  if (populateLines()) {
    lineCount = min((int)lines.size(), 0xFFFF);
    /// parse line and add fields
    for (i = 0; i < lineCount; i++) {
      addFields(lines[i]);
    }
    return true;
  }
  return false;
}

/// @todo fix 1st block segment disalignment bug
bool ResourceCsvReader::populateData() {
  int i, k, lineCount, fieldCount, fieldSize, fieldValue;
  bool isIntegerData = (resourceType == 3);  //! IDATA resource type
  data.clear();
  unpackedSize = 3;
  packedSize = 3;
  /// Resource type
  data.emplace_back(3);
  data.back()[0] = resourceType;
  /// line count
  lineCount = lineFields.size();
  data.back()[1] = (lineCount & 0xFF);
  data.back()[2] = ((lineCount >> 8) & 0xFF);
  /// lines data group
  for (i = 0; i < lineCount; i++) {
    /// lines numbers map
    if (i < (int)lineNumbers.size()) {
      data[0].push_back(lineNumbers[i] & 0xFF);
      data[0].push_back((lineNumbers[i] >> 8) & 0xFF);
      unpackedSize += 2;
      packedSize += 2;
    }
    /// lines fields map
    fieldCount = min((int)lineFields[i].size(), 0xFF);
    data[0].push_back(fieldCount);
    unpackedSize++;
    packedSize++;
    /// lines data
    for (k = 0; k < fieldCount; k++) {
      if (isIntegerData) {
        fieldValue = stoi(lineFields[i][k]);
        data.emplace_back(2);
        data.back()[0] = (fieldValue & 0xFF);
        data.back()[1] = ((fieldValue >> 8) & 0xFF);
        unpackedSize += 2;
        packedSize += 2;
      } else {
        /// field maximum size = 255 chars
        fieldSize = min((int)lineFields[i][k].size(), 0xFF);
        /// set field size
        data.emplace_back(fieldSize + 1);
        data.back()[0] = fieldSize;
        /// copy string text (max = 255 chars)
        memcpy(data.back().data() + 1, lineFields[i][k].c_str(), fieldSize);
        unpackedSize += fieldSize + 1;
        packedSize += fieldSize + 1;
      }
    }
  }
  return true;
}

bool ResourceCsvReader::isIt(string fileext) {
  return (strcasecmp(fileext.c_str(), ".CSV") == 0);
}

bool ResourceCsvReader::load() {
  if (populateFields()) {
    return populateData();
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

bool ResourceSprReader::isIt(string fileext) {
  return (strcasecmp(fileext.c_str(), ".SPR") == 0);
}

bool ResourceSprReader::load() {
  if (parseTinySpriteFile()) {
    return pack();
  }
  return false;
}

/// @remark Tiny Sprite file structure
/// for msx1 tiny sprite format file:
///   [type=0][sprite count][sprites patterns=collection of 32 bytes][sprites
///   colors=1 byte per sprite]
/// for msx2 format file:
///   [type=1][sprite count][sprites patterns=collection of 32 bytes][sprites
///   colors=16 bytes per sprite]
bool ResourceSprReader::parseTinySpriteFile() {
  ResourceTxtReader txtReader(filename);
  enum class ParseState { Header, Type, Slot, SpriteData };
  ParseState state = ParseState::Header;
  int length, sizeRead;
  int spriteType = 0;
  int spriteX = 0, spriteY = 0, spriteC = 0;
  int spriteK, spriteW, spriteN;
  unsigned char spriteData[16][4][8];
  unsigned char spriteAttr[16 * 255];
  unsigned char spriteColor[16];
  unsigned char *spriteCount = &spriteColor[0];
  unsigned char *buffer;
  bool ok = false, found;

  data.clear();
  data.emplace_back(0x4000);  // 16k uncompressed resource size
  buffer = data.back().data();

  if (txtReader.populateLines()) {
    *spriteCount = 0;
    sizeRead = 0;
    memset(spriteAttr, 0, 64);
    ok = true;

    for (string line : txtReader.getLines()) {
      length = (int)line.size();

      if (length) {
        switch (state) {
          // file header
          case ParseState::Header: {
            ok = (strcasecmp(line.c_str(), "!type") == 0);
            if (ok) state = ParseState::Type;
          } break;

          // sprite type
          case ParseState::Type: {
            if (strcasecmp(line.c_str(), "msx1") == 0) {
              spriteType = 0;  // msx 1 (screen mode <= 3)
            } else if (strcasecmp(line.c_str(), "msx2") == 0) {
              spriteType = 1;  // msx 2 (screen mode >= 4)
            } else {
              ok = false;
              break;
            }
            buffer[0] = spriteType;  // MSX 1
            buffer[1] = 0;
            spriteCount = &buffer[1];
            buffer += 2;
            sizeRead += 2;
            state = ParseState::Slot;
          } break;

          // slot number
          case ParseState::Slot: {
            if (line[0] == '#') {
              spriteY = 0;
              memset(spriteData, 0, 16 * 4 * 8);
              memset(spriteColor, 0, 16);
              state = ParseState::SpriteData;
            } else {
              ok = false;
            }
          } break;

          // sprite data
          case ParseState::SpriteData: {
            for (spriteX = 0; spriteX < 16; spriteX++) {
              spriteC = line[spriteX];

              if (spriteC >= '0' && spriteC <= '9') {
                spriteC -= '0';
              } else if (spriteC >= 'A' && spriteC <= 'F') {
                spriteC -= 'A';
                spriteC += 10;
              } else if (spriteC >= 'a' && spriteC <= 'f') {
                spriteC -= 'a';
                spriteC += 10;
              } else {
                spriteC = 0;
              }

              if (spriteC) {
                if (spriteY < 8) {
                  if (spriteX < 8) {
                    spriteK = 0;
                  } else {
                    spriteK = 2;
                  }
                } else {
                  if (spriteX < 8) {
                    spriteK = 1;
                  } else {
                    spriteK = 3;
                  }
                }
                spriteW = spriteY % 8;
                spriteN = (1 << (7 - (spriteX % 8)));
                spriteData[spriteC][spriteK][spriteW] |= spriteN;
                spriteColor[spriteC] = 1;
              }
            }

            spriteY++;

            if (spriteY >= 16) {
              if (spriteType) {
                // msx 2
                do {
                  found = false;
                  for (spriteY = 0; spriteY < 16; spriteY++) {
                    if (spriteY < 8) {
                      spriteK = 0;
                    } else {
                      spriteK = 1;
                    }
                    spriteW = spriteY % 8;
                    spriteN = spriteK + 2;
                    spriteData[0][spriteK][spriteW] = 0;
                    spriteData[0][spriteN][spriteW] = 0;
                    spriteColor[spriteY] = 0;
                    for (spriteC = 1; spriteC < 16; spriteC++) {
                      if (spriteData[spriteC][spriteK][spriteW] |
                          spriteData[spriteC][spriteN][spriteW]) {
                        spriteData[0][spriteK][spriteW] =
                            spriteData[spriteC][spriteK][spriteW];
                        spriteData[0][spriteN][spriteW] =
                            spriteData[spriteC][spriteN][spriteW];
                        spriteData[spriteC][spriteK][spriteW] = 0;
                        spriteData[spriteC][spriteN][spriteW] = 0;
                        spriteColor[spriteY] = spriteC;
                        found = true;
                        break;
                      }
                    }
                  }
                  if (found) {
                    spriteN = *spriteCount * 16;
                    memcpy(&spriteAttr[spriteN], &spriteColor[0], 16);
                    memcpy(buffer, spriteData[0], 32);
                    *spriteCount += 1;
                    buffer += 32;
                    sizeRead += 32;
                  }
                } while (found);
              } else {
                // msx 1
                for (spriteC = 0; spriteC < 16; spriteC++) {
                  if (spriteColor[spriteC]) {
                    spriteAttr[*spriteCount] = spriteC;
                    memcpy(buffer, spriteData[spriteC], 32);
                    *spriteCount += 1;
                    buffer += 32;
                    sizeRead += 32;
                  }
                }
              }
              state = ParseState::Slot;
            }
          } break;
        }

        if (!ok) break;
      }
    }

    if (ok) {
      if (*spriteCount) {
        if (spriteType) {
          // msx 2
          spriteN = *spriteCount * 16;
          memcpy(buffer, spriteAttr, spriteN);
          sizeRead += spriteN;
        } else {
          // msx 1
          memcpy(buffer, spriteAttr, *spriteCount);
          sizeRead += *spriteCount;
        }
      }
      if (sizeRead) {
        data.back().resize(sizeRead);
        return true;
      }
    }
  }
  errorMessage = "Error while parsing Tiny Sprite resource: " + filename;
  return false;
}

/// @remark Tiny Sprite file structure
/// for msx1 tiny sprite format file:
///   [type=0][sprite count][sprites patterns=collection of 32 bytes][sprites
///   colors=1 byte per sprite]
/// for msx2 format file:
///   [type=1][sprite count][sprites patterns=collection of 32 bytes][sprites
///   colors=16 bytes per sprite]
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

bool ResourceAkmReader::remapTo(int index, int mappedSegm, int mappedAddress) {
  if (index) return true;
  if (data.size()) {
    fixAKM(data[0].data(), mappedAddress, data[0].size());
    return true;
  }
  errorMessage = "AKM data is empty";
  return false;
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

bool ResourceAkxReader::remapTo(int index, int mappedSegm, int mappedAddress) {
  if (index) return true;
  if (data.size()) {
    fixAKX(data[0].data(), mappedAddress, data[0].size());
    return true;
  }
  errorMessage = "AKX data is empty";
  return false;
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

      if (i < (length - 1)) {
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
    : ResourceBlobReader(filename) {
  palleteSegment = palleteAddress = 0;
  tilesetSegment = tilesetAddress = 0;
  tilemapSegment = tilemapAddress = 0;
  tilemapWidth = tilemapHeight = 0;
};

bool ResourceMtfReader::isIt(string fileext) {
  return (strcasecmp(fileext.c_str(), ".MTF") == 0);
}

/***
 * @remarks
 * Resource structure
 *   Resource Map
 *     BYTE palleteSegment
 *     WORD palleteAddress
 *     BYTE tilesetSegment
 *     WORD tilesetAddress
 *     BYTE tilemapSegment
 *     WORD tilemapAddress
 *   Pallete Data (.SC4Pal)
 *     Color Data
 *   Tileset Data (.SC4Tiles, formated to screen 2/4)
 *     All Pattern Data Block and All Color Attribute Data Block
 *   Tilemap Header (.SC4Super and .SC4Map)
 *     WORD tilemapWidth
 *     WORD tilemapHeight
 *     GROUP tilemapLinesAddresses[tilemapHeight]
 *       BYTE tilemapLineSegment
 *       WORD tilemapLineAddress
 *   Tilemap Line Data [tilemapHeight] (formatted to screen 2/4)
 *     BYTE tileNumber[tilemapWidth]
 *
 * Technical Description of Generated Files
 * https://github.com/DamnedAngel/msx-tile-forge?tab=readme-ov-file#technical-description-of-generated-files
 *
 * 1. Palette File (.SC4Pal)
 *    Reserved Bytes (4 bytes): For future use.
 *    Color Data (48 bytes total):
 *      A sequence of 16 color entries (3 bytes each: R, G, B, with values 0-7
 *      per channel).
 * 2. Tileset File (.SC4Tiles)
 *    Header:
 *      num_tiles_in_file(1 byte): A value of 0 indicates 256 tiles.
 *      Project Tile Limit (1 byte): A value of 0 indicates 256 tiles.
 *      Reserved Bytes (3 bytes): Currently unused.
 *    All Pattern Data Block (Total: num_tiles * 8 bytes):
 *      Pattern data for all tiles, stored consecutively. Each tile is 8 bytes
 *      (1 byte per row). In each byte, the most significant bit is the leftmost
 *      pixel.
 *    All Color Attribute Data Block (Total: num_tiles * 8 bytes):
 *      Color attribute data for all tiles, stored consecutively. Each tile is 8
 *      bytes (1 byte per row). The high nibble (4 bits) is the foreground
 * palette index, and the low nibble is the background palette index.
 * 3. Supertile Definition File (.SC4Super)
 *    Header:
 *      Supertile Count (1 or 3 bytes):
 *        If the first byte is 1-255, it's the count.
 *        If 0, the next 2 bytes (a Little-Endian unsigned short) are the count
 *        (up to 65535).
 *      Supertile Grid Dimensions (2 bytes):
 *        width (1 byte), height (1 byte).
 *      Project Supertile Limit (2 bytes, Little-Endian)):
 *        A value of 0xFFFF indicates 65535.
 *      Reserved Bytes (2 bytes):
 *        Currently unused.
 *    Supertile Definition Blocks:
 *      Each block is width * height bytes, with each byte being a tile index
 *      (0-255).
 * 4. Map File (.SC4Map)
 *    Header:
 *      map_width (2 bytes, Little-Endian).
 *      map_height (2 bytes, Little-Endian).
 *      Reserved Bytes (4 bytes): Currently unused.
 *    Map Data (Variable size):
 *      A sequence of map_width * map_height supertile indices.
 *      -> Index Size: If the project's total supertile count was > 255 at save
 *      time, each index is 2 bytes (Little-Endian). Otherwise, each index is 1
 *      byte. The application detects this based on file size during loading.
 */
/// @todo NOT IMPLEMENTED YET
bool ResourceMtfReader::load() {
  errorMessage = "Not implemented yet (ResourceMtfReader::load)";
  return false;
}

bool ResourceMtfReader::remapTo(int index, int mappedSegm, int mappedAddress) {
  if (!data.size()) {
    errorMessage = "MTF resource map is empty";
    return false;
  }
  switch (index) {
    /// Resource Map
    case 0:
      return true;
    /// Pallete Data (.SC4Pal)
    case 1: {
      palleteSegment = mappedSegm;
      palleteAddress = mappedAddress;
      data[0][0] = mappedSegm;
      data[0][1] = mappedAddress & 0xFF;
      data[0][2] = (mappedAddress >> 8) & 0xFF;
    } break;
    /// Tileset Data (.SC4Tiles)
    case 2: {
      tilesetSegment = mappedSegm;
      tilesetAddress = mappedAddress;
      data[0][3] = mappedSegm;
      data[0][4] = mappedAddress & 0xFF;
      data[0][5] = (mappedAddress >> 8) & 0xFF;
    } break;
    /// Tilemap Header (.SC4Super and .SC4Map)
    case 3: {
      tilemapSegment = mappedSegm;
      tilemapAddress = mappedAddress;
      data[0][6] = mappedSegm;
      data[0][7] = mappedAddress & 0xFF;
      data[0][8] = (mappedAddress >> 8) & 0xFF;
    } break;
    /// Tilemap Line Head [tilemapHeight] (formatted to screen 2/4)
    default: {
      int i = (index - 4);
      if (i < tilemapHeight) {
        i = i * 3 + 4;
        data[3][i++] = mappedSegm;
        data[3][i++] = mappedAddress & 0xFF;
        data[3][i] = (mappedAddress >> 8) & 0xFF;
      }
    } break;
  }
  return true;
}

///-------------------------------------------------------------------------------

ResourceStringReader::ResourceStringReader(string text)
    : ResourceReader(text) {};

bool ResourceStringReader::load() {
  int lineSize;

  data.clear();

  /// string maximum size = 255 chars
  lineSize = min((int)filename.size(), 255);
  /// set string size
  data.emplace_back(lineSize + 1);
  /// copy string text (max = 255 chars)
  memcpy(data.back().data(), filename.c_str(), lineSize);
  /// end of line
  data.back()[lineSize] = 0;

  unpackedSize = lineSize + 1;
  packedSize = lineSize + 1;

  return true;
}

///-------------------------------------------------------------------------------

ResourceDataReader::ResourceDataReader(Parser *parser)
    : ResourceCsvReader(string("_DATA_")) {
  this->parser = parser;
  isIntegerData = false;
};

bool ResourceDataReader::load() {
  if (populateFields()) {
    return populateData();
  }
  return false;
}

bool ResourceDataReader::populateFields() {
  int fieldCount = parser->datas.size(), i;
  Lexeme *lexeme;
  string lineNumber;
  /// DATA resource type
  resourceType = 0;
  /// DATA resource data
  lineFields.clear();
  lineNumbers.clear();
  for (i = 0; i < fieldCount; i++) {  // DATA/IDATA items values
    if ((lexeme = parser->datas[i])) {
      if ((isIntegerData && lexeme->subtype == Lexeme::subtype_integer_data) ||
          (!isIntegerData && lexeme->subtype != Lexeme::subtype_integer_data)) {
        if (lineNumber != lexeme->tag) {
          lineNumber = lexeme->tag;
          lineFields.emplace_back(0);
          try {
            lineNumbers.push_back(stoi(lineNumber));
          } catch (...) {
            errorMessage =
                "DATA/IDATA statement invalid line number: " + lineNumber;
            return false;
          }
        }
        lineFields.back().push_back(fixFieldValue(lexeme->value));
      }
    }
  }
  return true;
}

///-------------------------------------------------------------------------------

ResourceIDataReader::ResourceIDataReader(Parser *parser)
    : ResourceDataReader(parser) {
  isIntegerData = true;
  filename = "_IDATA_";
};
