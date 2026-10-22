/***
 * @file resource_reader.h
 * @brief Resource reader base class header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef RESOURCE_READER_H_INCLUDED
#define RESOURCE_READER_H_INCLUDED

#include <memory>
#include <string>
#include <vector>

class Logger;
class ResourceContentSource;

using namespace std;

/***
 * @class ResourceReader
 * @brief Resource reader base class (abstract)
 */
class ResourceReader {
 protected:
  string filename;
  shared_ptr<Logger> logger;
  shared_ptr<ResourceContentSource> contentSource;

 public:
  vector<vector<unsigned char>> data;

  int unpackedSize;
  int packedSize;
  bool isPacked;
  bool has1stBlockAnd2ndBlockSegmentDisalignmentBug;

  shared_ptr<Logger> getLogger();
  const string getFilename();

  virtual bool remapTo(int index, int mappedSegm, int mappedAddress);
  virtual bool load() = 0;

  ResourceReader(string filename,
                 shared_ptr<ResourceContentSource> contentSource);
  virtual ~ResourceReader();
};

#endif  // RESOURCE_READER_H_INCLUDED
