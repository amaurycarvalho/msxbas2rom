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
  for (int i = 0; i < (int)resources.size(); i++) {
    printf("      Resource #%i: %s (", i, resources[i]->getFilename().c_str());
    if (resources[i]->isPacked)
      printf("%.1fK packed, ", resources[i]->packedSize / 1024.0);
    printf("%.1fK unpacked)\n", resources[i]->unpackedSize / 1024.0);
  }
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
  int mapSize = mapAddress + 2 + resources.size() * 5;
  int resourceItemIndex, resourceItemCount, resourceItemSize;
  int resourceBlockSegment = baseSegment;
  int resourceBlockAddress = mapSize;
  int resourceBlockIndex, resourceBlockCount, resourceBlockSize;
  int resourceBlockOffset, resourceBlockNextAddress;

  resourcesUnpackedSize = mapSize;  //! include resource map size
  resourcesPackedSize = mapSize;

  pages.clear();

  if (resources.size()) {
    /// check resource map size limit
    if (mapSize > 0x4000) {
      errorMessage = "Resource count maximum limit exceeded: " +
                     to_string(resources.size());
      return false;
    }

    /// add a new page
    pages.emplace_back(0x4000, 0xFF);
    memset(pages[0].data(), 0, 16);
    /// resource count
    pages[0][mapAddress++] = (resources.size() & 0xFF);
    pages[0][mapAddress++] = ((resources.size() >> 8) & 0xFF);

    /// write resources to pages
    resourceItemCount = resources.size();
    for (resourceItemIndex = 0; resourceItemIndex < resourceItemCount;
         resourceItemIndex++) {
      /// next resource item
      resourceReader = resources[resourceItemIndex];
      if (!resourceReader->load()) {
        errorMessage = resourceReader->getErrorMessage();
        return false;
      }

      /// add resource data
      resourceItemSize = 0;
      resourceBlockCount = resourceReader->data.size();
      for (resourceBlockIndex = 0; resourceBlockIndex < resourceBlockCount;
           resourceBlockIndex++) {
        /// resource block size check
        resourceBlockSize = resourceReader->data[resourceBlockIndex].size();
        if (resourceBlockSize > 0x4000) {
          errorMessage = "Resource file size exceeds maximum limit (16k): " +
                         resourceReader->getFilename();
          return false;
        }
        /// end of segment check
        resourceBlockNextAddress = (resourceBlockAddress + resourceBlockSize);
        if (resourceItemIndex == 0 && resourceBlockCount > 1) {
          /// 1st block segment disalignment bug mitigation
          /// @todo refactor to use a block linked list
          resourceBlockNextAddress += resourceReader->data[1].size();
        }
        if (resourceBlockNextAddress > 0x4000) {
          pages.emplace_back(0x4000, 0xFF);  //! add a new page
          resourceBlockSegment += 2;
          resourceBlockAddress = 0;
        }
        // remap resource address
        resourceBlockOffset = (resourceBlockAddress + baseAddress);
        if (!resourceReader->remapTo(resourceBlockIndex, resourceBlockSegment,
                                     resourceBlockOffset)) {
          errorMessage = resourceReader->getErrorMessage();
          return false;
        }
        if (resourceBlockIndex == 0) {
          /// add resource map item
          pages[0][mapAddress++] = resourceBlockOffset & 0xFF;
          pages[0][mapAddress++] = (resourceBlockOffset >> 8) & 0xFF;
          pages[0][mapAddress++] = resourceBlockSegment & 0xFF;
        }
        /// copy resource data
        memcpy(pages.back().data() + resourceBlockAddress,
               resourceReader->data[resourceBlockIndex].data(),
               resourceBlockSize);
        resourceItemSize += resourceBlockSize;
        resourceBlockAddress += resourceBlockSize;
      }
      /// resource map item size
      resourcesPackedSize += resourceItemSize;
      resourcesUnpackedSize += resourceReader->unpackedSize;
      resourceItemSize = min(resourceItemSize, 0xFFFF);
      pages[0][mapAddress++] = resourceItemSize & 0xFF;
      pages[0][mapAddress++] = (resourceItemSize >> 8) & 0xFF;
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
    if (ResourceMtfPaletteReader::isIt(fileext)) {
      return new ResourceMtfPaletteReader(filename);
    }
    if (ResourceMtfTilesetReader::isIt(fileext)) {
      return new ResourceMtfTilesetReader(filename);
    }
    if (ResourceMtfMapReader::isIt(fileext)) {
      return new ResourceMtfMapReader(filename);
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
  this->isPacked = false;
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
  int bytesPacked = 0;
  if (!data.size()) {
    errorMessage =
        "Resource file is empty so it's no possible to pack it with pletter: " +
        filename;
    return false;
  }
  unpackedSize = data[0].size();
  packedSize = unpackedSize;
  isPacked = true;
  if (unpackedSize > 0x2000) {
    errorMessage =
        "Resource size > 8k so it's no possible to unpack it\non machines with "
        "limited RAM: " +
        filename;
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
/// implementing resource block linked list
bool ResourceBlobChunkPackedReader::load() {
  unsigned char buffer[1024];
  unsigned char *srcBuf;
  int bytesPacked, bytesUnpacked;
  int srcSize, blockCount = 0;
  if (ResourceBlobReader::load()) {
    packedSize = 0;
    isPacked = true;
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
  ifstream file(filename);
  if (!file) {
    errorMessage = "Resource file not found: " + filename;
    return false;
  }

  lines.clear();
  string line;

  while (getline(file, line)) {
    // remove any CR/LF from the end of string
    while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
      line.pop_back();
    }
    // replace non-printable / non-ASCII characters with spaces
    for (char &ch : line) {
      if (ch < 32) {
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

/// @todo fix 1st block segment disalignment bug
/// implementing resource block linked list
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
    : ResourceTxtReader(filename) {
  resourceType = 1;  //! CSV resource type
  isIntegerData = false;
};

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
/// implementing resource block linked list
bool ResourceCsvReader::populateData() {
  int i, k, lineCount;
  int fieldCount, fieldSize, fieldValue;
  /// initialize
  data.clear();
  /// resource type
  data.emplace_back(3);
  data.back()[0] = resourceType;
  unpackedSize = 3;
  packedSize = 3;
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
        try {
          fieldValue = stoi(lineFields[i][k]);
        } catch (...) {
          fieldValue = 0;
        }
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
  data.emplace_back(0x1000);  // 4k uncompressed resource size
  buffer = data.back().data();

  if (txtReader.populateLines()) {
    *spriteCount = 0;
    sizeRead = 0;
    memset(spriteAttr, 0, 64);
    ok = true;

    for (string line : txtReader.lines) {
      length = (int)line.size();

      /// debug:
      /// printf("---> %s...\n", line.c_str());

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
  } else {
    errorMessage = txtReader.getErrorMessage();
    return false;
  }

  errorMessage = "Error while parsing Tiny Sprite resource: " + filename;
  return false;
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

/***
 * @remarks
 * https://www.julien-nevo.com/arkostracker/index.php/the-akm-player/
 * https://bitbucket.org/JulienNevo/arkostracker3/src/master/
 * https://bitbucket.org/JulienNevo/arkostracker3/src/master/doc/export/AKM.md
 */
bool ResourceAkmReader::fixAKM(unsigned char *data, int address, int length) {
  int t, i, k, w, current, previous, baseAddress, tracks, linker;
  int instrumentIndexTable, arpeggioIndexTable, pitchIndexTable;
  int subsongIndexTable;
  int noteBlockIndexTable, trackIndexTable;
  bool first, track_annotation;

  baseAddress = guessBaseAddress(data, length);
  if (baseAddress < 0) {
    printf("WARNING: cannot guess base address of AKM file\n");
    return false;
  }
  /// debug:
  /// printf("----> akm base address: %04X\n", baseAddress);

  /// adjust header

  instrumentIndexTable = data[0] | (data[1] << 8);
  arpeggioIndexTable = data[2] | (data[3] << 8);
  pitchIndexTable = data[4] | (data[5] << 8);

  instrumentIndexTable -= baseAddress;

  if (arpeggioIndexTable) {
    current = arpeggioIndexTable - baseAddress + address;
    data[2] = current & 0xFF;
    data[3] = (current >> 8) & 0xFF;
  }

  if (pitchIndexTable) {
    current = pitchIndexTable - baseAddress + address;
    data[4] = current & 0xFF;
    data[5] = (current >> 8) & 0xFF;
  }

  /// adjust subsongs table

  first = true;

  for (i = 6; i < instrumentIndexTable; i += 2) {
    subsongIndexTable = data[i] | (data[i + 1] << 8);
    if (subsongIndexTable) {
      subsongIndexTable -= baseAddress;

      /// debug:
      /// printf("===> subsong 0x%02X\n", subsongIndexTable);

      if (first) {
        first = false;
        previous =
            data[subsongIndexTable - 2] |
            (data[subsongIndexTable - 1] << 8);  // adjust the last instrument
        if (previous) {                          // before first subsong
          previous -= baseAddress;
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
        noteBlockIndexTable -= baseAddress;
        noteBlockIndexTable += address;
        if (subsongIndexTable >= 0 && subsongIndexTable < (length - 1)) {
          data[subsongIndexTable] = noteBlockIndexTable & 0xFF;
          data[subsongIndexTable + 1] = (noteBlockIndexTable >> 8) & 0xFF;
        }
      }

      trackIndexTable =
          data[subsongIndexTable + 2] | (data[subsongIndexTable + 3] << 8);
      if (trackIndexTable) {
        trackIndexTable -= baseAddress;

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
            current -= baseAddress;
            current += address;
            if (k >= 0 && k < (length - 1)) {
              data[k] = current & 0xFF;
              data[k + 1] = (current >> 8) & 0xFF;
            }
            k += 2;  //! ...skip loop address
            /// debug:
            /// printf("   end of song\n");
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
                  /// debug:
                  /// printf("   track c %i: 0x%02X\n", w, current);
                } else {
                  /// @remark
                  /// "current" value needs to be calculated to keep
                  /// the state machine loop consistency
                  /// NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores)
                  current = (current << 8) | data[k];
                  k++;
                  /// debug:
                  /// printf("   track c %i: 0x%04X\n", w, current);
                }
              }
            }
          }
        }

        if (track_annotation) {
          tracks = ((tracks & 3) + 1) * 2;
          /// debug:
          /// printf("   tracks: %i\n", tracks / 2);

          t = trackIndexTable + tracks;

          for (k = trackIndexTable; k < t; k += 2) {
            current =
                data[k] | (data[k + 1] << 8);  //! adjust the track address
            if (current) {
              current -= baseAddress;
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
  t -= baseAddress;
  first = true;

  for (i = instrumentIndexTable; i < t; i += 2) {
    current = data[i] | (data[i + 1] << 8);
    if (current) {
      current -= baseAddress;

      if (first)
        first = false;
      else {
        previous = data[current - 2] |
                   (data[current - 1] << 8);  //! adjust the previous instrument
        if (previous) {
          previous -= baseAddress;
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

  /// debug:
  /// ofstream file("song.dat", ios::binary);
  /// file.write(data, length);
  /// file.close();
  return true;
}

/***
 * @remarks
 * https://bitbucket.org/JulienNevo/arkostracker3/src/master/doc/export/AKM.md
 */
int ResourceAkmReader::guessBaseAddress(unsigned char *data, int length) {
  int i, baseAddress;
  int instrumentIndexTable, instrumentIndexItem;
  int subsongIndexTable, subsongIndexHeader;
  unsigned char areEffectsPresent;
  const int areEffectsPresentHeaderPosition = 12;
  const int subsongHeaderSize = 13;
  bool found = false;

  /// AKM header

  instrumentIndexTable = data[0] | (data[1] << 8);
  subsongIndexTable = data[6] | (data[7] << 8);

  /// guessing starts at address: subsong0 - AKM file size
  baseAddress = subsongIndexTable - length;
  while (baseAddress < 0) baseAddress++;

  /// guessing loop
  while (baseAddress < instrumentIndexTable) {
    instrumentIndexItem = instrumentIndexTable - baseAddress;
    if (instrumentIndexItem < 8) break;
    found = true;
    for (i = 6; i < instrumentIndexItem; i += 2) {
      subsongIndexTable = data[i] | (data[i + 1] << 8);
      subsongIndexHeader = subsongIndexTable - baseAddress;
      if (subsongIndexHeader > (length - subsongHeaderSize)) {
        found = false;
        break;
      }
      areEffectsPresent =
          data[subsongIndexHeader + areEffectsPresentHeaderPosition];
      if (areEffectsPresent != 0x0C && areEffectsPresent != 0x0D) {
        found = false;
        break;
      }
    }
    if (found) break;
    baseAddress++;
  }

  if (!found) baseAddress = -1;

  return baseAddress;
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

/***
 * @remarks
 * https://www.julien-nevo.com/arkostracker/index.php/the-akm-player/
 * https://bitbucket.org/JulienNevo/arkostracker3/src/master/
 * https://bitbucket.org/JulienNevo/arkostracker3/src/master/doc/export/SoundEffects.md
 */
bool ResourceAkxReader::fixAKX(unsigned char *data, int address, int length) {
  int i, baseAddress;
  int firstSoundEffectAddress, firstSoundEffectFixed;
  int soundEffectAddress, soundEffectFixed;

  baseAddress = guessBaseAddress(data, length);
  if (baseAddress < 0) {
    printf("WARNING: cannot guess base address of AKX file\n");
    return false;
  }
  /// debug:
  /// printf("----> akx base address: %04X\n", baseAddress);

  firstSoundEffectAddress = data[0] | (data[1] << 8);
  firstSoundEffectFixed = firstSoundEffectAddress - baseAddress;

  // loop the effects list
  for (i = 0; i < firstSoundEffectFixed; i += 2) {
    soundEffectAddress = data[i] | (data[i + 1] << 8);
    soundEffectFixed = soundEffectAddress - baseAddress + address;

    if (i < (length - 1)) {
      data[i] = soundEffectFixed & 0xFF;
      data[i + 1] = (soundEffectFixed >> 8) & 0xFF;
    }
  }

  /// debug:
  /// ofstream file("effect.dat", ios::binary);
  /// file.write(data, length);
  /// file.close();
  return true;
}

/***
 * @remarks
 * https://bitbucket.org/JulienNevo/arkostracker3/src/master/doc/export/SoundEffects.md
 */
int ResourceAkxReader::guessBaseAddress(unsigned char *data, int length) {
  int baseAddress, i;
  int firstSoundEffectAddress, firstSoundEffectItem;
  int soundEffectAddress, soundEffectItem;
  int previousSoundEffect;
  bool found = false;

  /// AKX header

  firstSoundEffectAddress = data[0] | (data[1] << 8);

  /// guessing starts at address: firstSoundEffectAddress - AKX file size

  baseAddress = firstSoundEffectAddress - length;
  while (baseAddress < 0) baseAddress++;

  /// guessing loop
  while (baseAddress < firstSoundEffectAddress) {
    firstSoundEffectItem = firstSoundEffectAddress - baseAddress;
    previousSoundEffect = firstSoundEffectItem;
    found = true;
    for (i = 0; i < firstSoundEffectItem; i += 2) {
      soundEffectAddress = data[i] | (data[i + 1] << 8);
      soundEffectItem = soundEffectAddress - baseAddress;
      if (soundEffectItem >= length || soundEffectItem < previousSoundEffect) {
        found = false;
        break;
      }
      previousSoundEffect = soundEffectItem;
    }
    if (found) break;
    baseAddress++;
  }

  if (!found) baseAddress = -1;

  return baseAddress;
}

///-------------------------------------------------------------------------------

/***
 * @remarks
 * Wrapper to ResourceMtfPalReader, ResourceMtfTilesReader and
 * ResourceMtfMapReader
 */
/// @todo NOT IMPLEMENTED YET
bool ResourceMtfReader::load() {
  errorMessage = "Not implemented yet (ResourceMtfReader::load)";
  return false;
}

ResourceMtfReader::ResourceMtfReader(string filename)
    : ResourceReader(filename) {};

bool ResourceMtfReader::isIt(string fileext) {
  return (strcasecmp(fileext.c_str(), ".MTF") == 0);
}

/***
 * @remarks
 * Palette File (.SC4Pal)
 *    Reserved Bytes (4 bytes): For future use.
 *    Color Data (48 bytes total):
 *      A sequence of 16 color entries (3 bytes each: R, G, B, with values 0-7
 *      per channel).
 * Technical Description of Generated Files
 *   https://github.com/DamnedAngel/msx-tile-forge?tab=readme-ov-file#technical-description-of-generated-files
 * @todo To be used with S.SETPLT EXTENDED BIOS function
 */
bool ResourceMtfPaletteReader::load() {
  if (ResourceBlobReader::load()) {
    /// resource type = 0 (palette)
    data[0].emplace(data[0].begin(), 1);
    data[0][0] = 0;
    packedSize = unpackedSize = data[0].size();
    return true;
  }
  return false;
}

ResourceMtfPaletteReader::ResourceMtfPaletteReader(string filename)
    : ResourceBlobReader(filename) {};

bool ResourceMtfPaletteReader::isIt(string fileext) {
  return (strcasecmp(fileext.c_str(), ".SC4Pal") == 0);
}

/***
 * @remarks
 * Tileset File (.SC4Tiles)
 *    Header:
 *      num_tiles_in_file(1 byte): A value of 0 indicates 256 tiles.
 *      Project Tile Limit (1 byte): A value of 0 indicates 256 tiles.
 *      Reserved Bytes (3 bytes): Currently unused.
 *    All Pattern Data Block (Total: num_tiles * 8 bytes):
 *      Pattern data for all tiles, stored consecutively. Each tile is 8 bytes
 *      (1 byte per row). In each byte, the most significant bit is the
 *      leftmost pixel.
 *    All Color Attribute Data Block (Total: num_tiles * 8 bytes):
 *      Color attribute data for all tiles, stored consecutively. Each
 *      tile is 8 bytes (1 byte per row). The high nibble (4 bits) is the
 *      foreground palette index, and the low nibble is the background palette
 *      index.
 * Technical Description of Generated Files
 *   https://github.com/DamnedAngel/msx-tile-forge?tab=readme-ov-file#technical-description-of-generated-files
 */
bool ResourceMtfTilesetReader::load() {
  if (ResourceBlobReader::load()) {
    /// resource type = 1 (tileset)
    data[0].emplace(data[0].begin(), 1);
    data[0][0] = 1;
    packedSize = unpackedSize = data[0].size();
    return true;
  }
  return false;
}

ResourceMtfTilesetReader::ResourceMtfTilesetReader(string filename)
    : ResourceBlobReader(filename) {};

bool ResourceMtfTilesetReader::isIt(string fileext) {
  return (strcasecmp(fileext.c_str(), ".SC4Tiles") == 0);
}

/***
 * @remarks
 * Supertile Definition File (.SC4Super)
 *    Header:
 *      Supertile Count (1 or 3 bytes):
 *        If the first byte is 1-255, it's the count.
 *        If 0, the next 2 bytes (a Little-Endian unsigned short) are the
 *        count (up to 65535).
 *      Supertile Grid Dimensions (2 bytes):
 *        width (1 byte), height (1 byte).
 *      Project Supertile Limit (2 bytes, Little-Endian)):
 *        A value of 0xFFFF indicates 65535.
 *      Reserved Bytes (2 bytes): Currently unused.
 *      Supertile Definition Blocks:
 *        Each block is  width * height bytes,
 *        with each byte being a tile index (0-255).
 * Map File (.SC4Map)
 *    Header:
 *      map_width (2 bytes, Little-Endian).
 *      map_height (2 bytes, Little-Endian).
 *      Reserved Bytes (4 bytes): Currently unused.
 *    Map Data (Variable size):
 *      A sequence of map_width * map_height supertile indices.
 *      --> Index Size:
 *          If the project's total supertile count was > 255 at save time,
 *          each index is 2 bytes (Little-Endian).
 *          Otherwise, each index is 1 byte.
 * Technical Description of Generated Files
 *   https://github.com/DamnedAngel/msx-tile-forge?tab=readme-ov-file#technical-description-of-generated-files
 */
/// @todo NOT IMPLEMENTED YET
bool ResourceMtfMapReader::load() {
  ResourceBlobReader tilemapReader(filename);
  ResourceBlobReader supertileReader(supertileFilename);
  int tilemapHeightIterator, tilemapWidthIterator;
  int supertileHeightIterator, supertileWidthIterator;
  int tilemapIndex, tilemapBaseIndex, tilemapSaveIndex;
  int supertileIndex, supertileBaseIndex;
  int supertileIndexSize;
  packedSize = unpackedSize = 0;
  if (tilemapReader.load()) {
    if (supertileReader.load()) {
      /// supertile count
      if (supertileReader.data[0][0]) {
        supertileHeaderSkip = 0;
        supertileIndexSize = 1;
        supertileCount = supertileReader.data[0][0];
      } else {
        supertileHeaderSkip = 2;
        supertileIndexSize = 2;
        supertileCount =
            supertileReader.data[0][1] | (supertileReader.data[0][2] << 8);
      }
      /// BYTE resourceType = 2 (map)
      data.clear();
      data.emplace_back(8);
      data[0][0] = 2;
      packedSize += data.back().size();
      unpackedSize += data.back().size();
      /// WORD tilemapWidth
      supertileWidth = supertileReader.data[0][1 + supertileHeaderSkip];
      tilemapWidth = tilemapReader.data[1][0] | (tilemapReader.data[1][1] << 8);
      tilemapResourceWidth = tilemapWidth * supertileWidth + 31;
      data[0][1] = tilemapResourceWidth & 0xFF;
      data[0][2] = (tilemapResourceWidth >> 8) & 0xFF;
      /// WORD tilemapHeight
      supertileHeight = supertileReader.data[0][2 + supertileHeaderSkip];
      tilemapHeight =
          tilemapReader.data[1][2] | (tilemapReader.data[1][3] << 8);
      tilemapResourceHeight = tilemapHeight * supertileHeight;
      data[0][3] = tilemapResourceHeight & 0xFF;
      data[0][4] = (tilemapResourceHeight >> 8) & 0xFF;
      /// BYTE firstLineSegment
      data[0][5] = 0;
      /// WORD firstLineAddress
      data[0][6] = 0;
      data[0][7] = 0;
      ///   Tilemap Line Data [tilemapHeight] <-- .SC4Super + .SC4Map
      ///     BYTE nextLineSegment
      ///     WORD nextLineAddress
      ///     BYTE tilemap[tilemapWidth+31] <-- copy of first
      ///                                       31 tiles at end
      for (tilemapHeightIterator = 0; tilemapHeightIterator < tilemapHeight;
           tilemapHeightIterator++) {
        for (supertileHeightIterator = 0;
             supertileHeightIterator < supertileHeight;
             supertileHeightIterator++) {
          data.emplace_back(tilemapResourceWidth + 3);
          packedSize += data.back().size();
          unpackedSize += data.back().size();
          /// nextLineSegment
          data.back()[0] = 0;
          /// nextLineSegment
          data.back()[1] = 0;
          data.back()[2] = 0;
          /// tilemap
          tilemapSaveIndex = 3;
          tilemapBaseIndex =
              8 + tilemapHeightIterator * tilemapWidth * supertileIndexSize;
          for (tilemapWidthIterator = 0; tilemapWidthIterator < tilemapWidth;
               tilemapWidthIterator++) {
            supertileIndex =
                tilemapReader
                    .data[0][tilemapBaseIndex +
                             tilemapWidthIterator * supertileIndexSize];
            supertileBaseIndex =
                7 + supertileHeaderSkip +
                supertileIndex * supertileWidth * supertileHeight +
                supertileHeightIterator * supertileWidth;
            for (supertileWidthIterator = 0;
                 supertileWidthIterator < supertileWidth;
                 supertileWidthIterator++) {
              tilemapIndex =
                  supertileReader
                      .data[0][supertileBaseIndex + supertileWidthIterator];
              data.back()[tilemapSaveIndex++] = tilemapIndex;
            }
          }
          /// copy of first 31 tiles at end (for scroll engine use)
          for (int i = 0; i < 31; i++)
            data.back()[tilemapSaveIndex++] = data.back()[i + 3];
        }
      }
      return true;
    } else
      errorMessage = supertileReader.getErrorMessage();
  } else
    errorMessage = tilemapReader.getErrorMessage();
  return false;
}

bool ResourceMtfMapReader::remapTo(int index, int mappedSegm,
                                   int mappedAddress) {
  int firstLineSegment, firstLineAddress;

  if (!index) return true;

  if (index == 1) {  //! first line data -> adjust header
    /// set firstLineSegment
    firstLineSegment = mappedSegm;
    data[0][5] = firstLineSegment & 0xFF;
    /// set firstLineAddress
    firstLineAddress = mappedAddress;
    data[0][6] = firstLineAddress & 0xFF;
    data[0][7] = (firstLineAddress >> 8) & 0xFF;
  } else {
    /// get firstLineSegment
    firstLineSegment = data[0][5];
    /// get firstLineAddress
    firstLineAddress = data[0][6] | (data[0][7] << 8);
    /// set last line nextLineSegment
    data[index - 1][0] = mappedSegm & 0xFF;
    /// set last line nextLineAddress
    data[index - 1][1] = mappedAddress & 0xFF;
    data[index - 1][2] = (mappedAddress >> 8) & 0xFF;
  }

  /// set current line nextLineSegment
  data[index][0] = firstLineSegment & 0xFF;
  /// set current line nextLineAddress
  data[index][1] = firstLineAddress & 0xFF;
  data[index][2] = (firstLineAddress >> 8) & 0xFF;

  return true;
}

ResourceMtfMapReader::ResourceMtfMapReader(string filename)
    : ResourceReader(filename) {
  supertileFilename = filename;
  if (filename.size() > 3) {
    supertileFilename.pop_back();
    supertileFilename.pop_back();
    supertileFilename.pop_back();
  }
  supertileFilename += "Super";
};

bool ResourceMtfMapReader::isIt(string fileext) {
  return (strcasecmp(fileext.c_str(), ".SC4Map") == 0);
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
  resourceType = 0;  //! DATA resource type
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
  /// DATA/IDATA resource data
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
  filename = "_IDATA_";
  resourceType = 3;  //! IDATA resource type
  isIntegerData = true;
};
