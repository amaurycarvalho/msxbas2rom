/***
 * @file resource_mtf_map_reader.cpp
 * @brief Resource MTF map reader implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "resource_mtf_map_reader.h"

#include <strings.h>

#include "logger.h"
#include "resource_blob_reader.h"

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
      /// tileset and supertile header values
      supertileWidth = supertileReader.data[0][1 + supertileHeaderSkip];
      tilemapWidth = tilemapReader.data[0][0] | (tilemapReader.data[0][1] << 8);
      tilemapResourceWidth = tilemapWidth * supertileWidth;  // + 31;
      supertileHeight = supertileReader.data[0][2 + supertileHeaderSkip];
      tilemapHeight =
          tilemapReader.data[0][2] | (tilemapReader.data[0][3] << 8);
      tilemapResourceHeight = tilemapHeight * supertileHeight;
      /// Header block allocation
      data.clear();
      data.emplace_back(5 + (tilemapResourceHeight * 3), 0);
      packedSize += data.back().size();
      unpackedSize += data.back().size();
      /// BYTE resourceType = 2 (map)
      data[0][0] = 2;
      /// WORD tilemapWidth
      data[0][1] = tilemapResourceWidth & 0xFF;
      data[0][2] = (tilemapResourceWidth >> 8) & 0xFF;
      /// WORD tilemapHeight
      data[0][3] = tilemapResourceHeight & 0xFF;
      data[0][4] = (tilemapResourceHeight >> 8) & 0xFF;
      /// GROUP linesTable[tilemapHeight] <-- it will be filled by remapTo code
      ///   BYTE lineSegment
      ///   WORD lineAddress
      /// Tilemap Line Data [tilemapHeight] <-- .SC4Super + .SC4Map
      ///   BYTE nextLineSegment
      ///   WORD nextLineAddress
      ///   BYTE tilemap[tilemapWidth+31] <-- copy of first
      ///                                     31 tiles at end
      for (tilemapHeightIterator = 0; tilemapHeightIterator < tilemapHeight;
           tilemapHeightIterator++) {
        for (supertileHeightIterator = 0;
             supertileHeightIterator < supertileHeight;
             supertileHeightIterator++) {
          data.emplace_back(tilemapResourceWidth + 31 + 3);
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
      logger->add(supertileReader.getLogger());

  } else
    logger->add(tilemapReader.getLogger());

  return false;
}

bool ResourceMtfMapReader::remapTo(int index, int mappedSegm,
                                   int mappedAddress) {
  int firstLineSegment, firstLineAddress;
  int blockIndex;

  if (!index) return true;

  /// fill header lines table
  blockIndex = (index - 1) * 3;
  data[0][5 + blockIndex] = mappedSegm & 0xFF;     //! lineSegment
  data[0][6 + blockIndex] = mappedAddress & 0xFF;  //! lineAddress
  data[0][7 + blockIndex] = (mappedAddress >> 8) & 0xFF;

  /// fill previous line linked list, if necessary
  if (index == 1) {
    /// get firstLineSegment and firstLineAddress
    firstLineSegment = mappedSegm;
    firstLineAddress = mappedAddress;
  } else {
    /// get firstLineSegment and firstLineAddress
    firstLineSegment = data[0][5];
    firstLineAddress = data[0][6] | (data[0][7] << 8);
    /// set previous line nextLineSegment
    data[index - 1][0] = mappedSegm & 0xFF;
    /// set previous line nextLineAddress
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
  /// remove any trailing space
  while (!filename.empty() && filename.back() == ' ') {
    filename.pop_back();
  }
  /// remove 'Map' from extension name
  if (filename.size() > 3) {
    supertileFilename.pop_back();
    supertileFilename.pop_back();
    supertileFilename.pop_back();
  }
  /// add 'Super' into extension name
  supertileFilename += "Super";
};

bool ResourceMtfMapReader::isIt(string fileext) {
  return (strcasecmp(fileext.c_str(), ".SC4Map") == 0);
}
