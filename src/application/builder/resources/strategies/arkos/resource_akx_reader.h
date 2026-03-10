/***
 * @file resource_akx_reader.h
 * @brief Resource AKX reader header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef RESOURCE_AKX_READER_H_INCLUDED
#define RESOURCE_AKX_READER_H_INCLUDED

#include "resource_blob_reader.h"

/***
 * @class ResourceAkxReader
 * @brief Resource reader for Arkos Tracker sound effects files (.AKX)
 * @remarks
 * https://www.julien-nevo.com/arkostracker/
 * https://bitbucket.org/JulienNevo/arkostracker3/src/master/
 * https://bitbucket.org/JulienNevo/arkostracker3/src/master/doc/export/SoundEffects.md
 */
class ResourceAkxReader : public ResourceBlobReader {
 private:
  bool fixAKX(unsigned char *data, int address, int length);
  int guessBaseAddress(unsigned char *data, int length);

 public:
  static bool isIt(string fileext);
  bool remapTo(int index, int mappedSegm, int mappedAddress);
  ResourceAkxReader(string filename);
};

#endif  // RESOURCE_AKX_READER_H_INCLUDED
