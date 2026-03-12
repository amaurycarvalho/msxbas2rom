/***
 * @file resource_akm_reader.cpp
 * @brief Resource AKM reader implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "resource_akm_reader.h"

#include <strings.h>

#include "logger.h"

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
  logger->error("AKM data is empty");
  return false;
}

/***
 * @remarks
 * https://www.julien-nevo.com/arkostracker/index.php/the-akm-player/
 * https://bitbucket.org/JulienNevo/arkostracker3/src/master/
 * https://bitbucket.org/JulienNevo/arkostracker3/src/master/doc/export/AKM.md
 */
bool ResourceAkmReader::fixAKM(unsigned char* data, int address, int length) {
  int t, i, k, w, current, previous, baseAddress, tracks, linker;
  int instrumentIndexTable, arpeggioIndexTable, pitchIndexTable;
  int subsongIndexTable;
  int noteBlockIndexTable, trackIndexTable;
  bool first, track_annotation;

  baseAddress = guessBaseAddress(data, length);
  if (baseAddress < 0) {
    logger->warning("Cannot guess base address of AKM file");
    return false;
  }
  /// debug:
  /// logger->debug("----> akm base address: %04X", baseAddress);

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
      /// logger->debug("===> subsong 0x%02X\n", subsongIndexTable);

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
            /// logger->debug("   end of song");
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
                  /// logger->debug("   track c %i: 0x%02X\n", w, current);
                } else {
                  /// @remark
                  /// "current" value needs to be calculated to keep
                  /// the state machine loop consistency
                  /// NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores)
                  current = (current << 8) | data[k];
                  k++;
                  /// debug:
                  /// logger->debug("   track c %i: 0x%04X\n", w, current);
                }
              }
            }
          }
        }

        if (track_annotation) {
          tracks = ((tracks & 3) + 1) * 2;
          /// debug:
          /// logger->debug("   tracks: %i\n", tracks / 2);

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
int ResourceAkmReader::guessBaseAddress(unsigned char* data, int length) {
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
