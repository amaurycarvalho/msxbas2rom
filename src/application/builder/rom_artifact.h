/***
 * @file rom_artifact.h
 * @brief In-memory assembled ROM artifact
 * @author Amaury Carvalho (2026)
 */

#ifndef ROM_ARTIFACT_H_INCLUDED
#define ROM_ARTIFACT_H_INCLUDED

#include <string>
#include <vector>

class RomArtifact {
 public:
  std::vector<std::vector<unsigned char>> pages;

  int romSize;
  int codeSize;
  int resourcesSize;
  int resourceMapSegment;
  int resourceMapAddress;

  double codeShare;
  double resourcesShare;
  double kernelShare;

  bool megaROM;
  std::string diagnostics;

  RomArtifact()
      : romSize(0),
        codeSize(0),
        resourcesSize(0),
        resourceMapSegment(0),
        resourceMapAddress(0),
        codeShare(0),
        resourcesShare(0),
        kernelShare(0),
        megaROM(false) {}
};

#endif  // ROM_ARTIFACT_H_INCLUDED
