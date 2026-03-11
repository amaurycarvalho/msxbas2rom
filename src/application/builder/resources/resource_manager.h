/***
 * @file resource_manager.h
 * @brief Resource manager header
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#ifndef RESOURCE_MANAGER_H_INCLUDED
#define RESOURCE_MANAGER_H_INCLUDED

#include <memory>

#include "resource_reader.h"

class Parser;
class Logger;

using namespace std;

/***
 * @class ResourceManager
 * @brief Resource manager
 */
class ResourceManager {
 private:
  string errorMessage;

 public:
  vector<std::unique_ptr<ResourceReader>> resources;
  vector<vector<unsigned char>> pages;

  unique_ptr<Logger> logger;

  int resourcesPackedSize;
  int resourcesUnpackedSize;
  float packedRate;

  ResourceManager();
  ~ResourceManager();

  /***
   * @brief clear all resources
   */
  void clear();

  /***
   * @brief add a new FILE resource
   */
  bool addFile(string filename, string inputPath);

  /***
   * @brief Add a new TEXT string resource to the resource list
   */
  void addText(string text);

  /***
   * @brief Add a DATA statement resource to the resource list
   * @todo Implement IDATA+DATA mix up bug fix
   */
  void addDataResource(Parser* parser);

  /***
   * @brief Add a IDATA statement resource to the resource list
   * @todo Implement IDATA+DATA mix up bug fix
   */
  void addIDataResource(Parser* parser);

  /***
   * @brief build map and resources data
   * @remarks
   * Resources location on ROM:
   *    48kb ROM - starts on page 0;
   *    128kb MEGAROM - starts on the next segment after the code.
   * Resource table starts at position 0x10 of the allocated segment.
   * So, a max of 3273 resources is possible:
   *    maxResourceCount = (0x4000 - 0x10 - 2) / 5
   * Resource table structure:
   *    CHAR filler[0x10]
   *    WORD resourceCount
   *    GROUP resourceTable[resourceCount]
   *      WORD offsetOnPage
   *      BYTE segmentNumber
   *      WORD resourceSize
   * Resources data starts immediatelly following resource table.
   */
  bool buildMap(int startSegment, int startAddress);

  /***
   * @brief return last error message
   */
  const string getErrorMessage();

  /***
   * @brief print resources names
   */
  string toString();
};

#endif  // RESOURCE_MANAGER_H_INCLUDED
