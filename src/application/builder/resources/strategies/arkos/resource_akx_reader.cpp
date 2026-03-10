/***
 * @file resource_akx_reader.cpp
 * @brief Resource AKX reader implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "resource_akx_reader.h"

#include <strings.h>

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
