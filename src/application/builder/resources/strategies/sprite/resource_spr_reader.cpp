/***
 * @file resource_spr_reader.cpp
 * @brief Resource SPR reader implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "resource_spr_reader.h"

#include <string.h>
#include <strings.h>

#include "resource_txt_reader.h"

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
