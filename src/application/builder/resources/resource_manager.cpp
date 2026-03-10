/***
 * @file resource_manager.cpp
 * @brief Resource manager implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "resource_manager.h"

#include <algorithm>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string.h>
#include <utility>

#include "fswrapper.h"
#include "resource_data_reader.h"
#include "resource_factory.h"
#include "resource_idata_reader.h"
#include "resource_string_reader.h"

void ResourceManager::clear() {
  resources.clear();
  pages.clear();
}

string ResourceManager::toString() {
  ostringstream out;
  out << fixed << setprecision(1);
  for (int i = 0; i < (int)resources.size(); i++) {
    out << "      Resource #" << i << ": " << resources[i]->getFilename()
        << " (";
    if (resources[i]->isPacked)
      out << (resources[i]->packedSize / 1024.0) << "K packed, ";
    out << (resources[i]->unpackedSize / 1024.0) << "K unpacked)\n";
  }
  return out.str();
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
  auto resourceReader = ResourceFactory::create(filename);
  if (resourceReader) {
    resources.push_back(std::move(resourceReader));
    return true;
  }
  errorMessage = "Resource type not recognized: " + filename;
  return false;
}

void ResourceManager::addText(string text) {
  resources.emplace_back(new ResourceStringReader(text));
}

void ResourceManager::addDataResource(Parser *parser) {
  resources.emplace_back(new ResourceDataReader(parser));
}

void ResourceManager::addIDataResource(Parser *parser) {
  resources.emplace_back(new ResourceIDataReader(parser));
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
  int copyIndex;
  vector<unsigned char *> copyFrom, copyTo;
  vector<int> copySize;

  resourcesUnpackedSize = mapSize;  //! include resource map size
  resourcesPackedSize = mapSize;

  pages.clear();
  copyFrom.clear();
  copyTo.clear();
  copySize.clear();

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
      resourceReader = resources[resourceItemIndex].get();
      /// debug:
      /// printf("Building resource: %s\n",
      /// resourceReader->getFilename().c_str());
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
        /// calculate next block address
        resourceBlockNextAddress = (resourceBlockAddress + resourceBlockSize);
        /// @brief 1st block segment disalignment bug mitigation code
        /// @todo refactor the affected resources to use a block linked list
        if (resourceReader->has1stBlockAnd2ndBlockSegmentDisalignmentBug &&
            resourceItemIndex == 0 && resourceBlockCount > 1) {
          resourceBlockNextAddress += resourceReader->data[1].size();
        }
        /// check if end of the current segment
        if (resourceBlockNextAddress > 0x4000) {
          pages.emplace_back(0x4000, 0xFF);  //! add a new page
          resourceBlockSegment += 2;
          resourceBlockAddress = 0;
          /// check MegaROM size limit
          if (resourceBlockSegment > 255) {
            errorMessage = "MegaROM size limit exceeded (2048K)";
            return false;
          }
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
        /// copy resource block data
        /// memcpy(pages.back().data() + resourceBlockAddress,
        ///        resourceReader->data[resourceBlockIndex].data(),
        ///        resourceBlockSize);
        copyFrom.push_back(resourceReader->data[resourceBlockIndex].data());
        copyTo.push_back(pages.back().data() + resourceBlockAddress);
        copySize.push_back(resourceBlockSize);
        /// update block metrics
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
    /// update pages data (after address remap calculations)
    for (copyIndex = 0; copyIndex < (int)copyFrom.size(); copyIndex++) {
      memcpy(copyTo[copyIndex], copyFrom[copyIndex], copySize[copyIndex]);
    }
    /// update metrics
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
