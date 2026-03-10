/***
 * @file resource_akm_reader.h
 * @brief Resource AKM reader header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef RESOURCE_AKM_READER_H_INCLUDED
#define RESOURCE_AKM_READER_H_INCLUDED

#include "resource_blob_reader.h"

/***
 * @class ResourceAkmReader
 * @brief Resource reader for Arkos Tracker minimalist player music files
 * (.AKM)
 * @remarks
 * https://www.julien-nevo.com/arkostracker/
 * https://bitbucket.org/JulienNevo/arkostracker3/src/master/
 * https://bitbucket.org/JulienNevo/arkostracker3/src/master/doc/export/AKM.md
 */
class ResourceAkmReader : public ResourceBlobReader {
 private:
  bool fixAKM(unsigned char *data, int address, int length);
  int guessBaseAddress(unsigned char *data, int length);

 public:
  static bool isIt(string fileext);
  bool remapTo(int index, int mappedSegm, int mappedAddress);
  ResourceAkmReader(string filename);
};

#endif  // RESOURCE_AKM_READER_H_INCLUDED
