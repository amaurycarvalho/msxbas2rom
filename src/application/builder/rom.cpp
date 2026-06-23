/***
 * @file rom.cpp
 * @brief ROM builder class implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 *   https://www.msx.org/wiki/Develop_a_program_in_cartridge_ROM
 *   https://www.msx.org/wiki/MegaROM_Mappers
 *   https://aoineko.org/msxgl/index.php?title=Create_a_plain_ROM
 *   https://aoineko.org/msxgl/index.php?title=Create_a_mapped_ROM
 */

#include "rom.h"

#include <cstring>
#include <fstream>

#include "build_options.h"
#include "compiler.h"
#include "compiler_hooks.h"
#include "header.h"
#include "logger.h"
#include "parser.h"
#include "resource_manager.h"
#include "start.h"

//----------------------------------------------------------------------------------------------

Rom::Rom() {
  logger = make_shared<Logger>();
}

Rom::~Rom() = default;

shared_ptr<Logger> Rom::getLogger() {
  return logger;
}

bool Rom::build(shared_ptr<Compiler> compiler) {
  float romSizeFloat;

  if (!compiler) return false;

  if (!compiler->isCompiled()) return false;

  /// initialize data

  this->compiler = compiler;
  this->opts = compiler->getOpts();
  this->resourceManager = compiler->getResourceManager();

  logger->setFile(opts->inputFilename);

  logger->debug("Initializing build...");

  buildInit();

  /// add kernel page

  logger->debug("Adding kernel data...");

  if (!addKernel()) return false;

  /// add code page

  logger->debug("Adding compiled code...");

  if (!addCompiledCode()) return false;

  // add resources pages

  logger->debug("Adding resources...");

  if (!addResources()) return false;

  // save ROM file

  logger->debug("Writing ROM file...");

  if (!writeRom(opts->outputFilename)) return false;

  // calculate shares

  logger->debug("Calculating ROM statistics...");

  romSizeFloat = romSize;
  kernelShare = (0x4000 / romSizeFloat) * 100;
  codeShare = (codeSize / romSizeFloat) * 100;
  resourcesShare = (resourcesSize / romSizeFloat) * 100;

  return true;
}

//----------------------------------------------------------------------------------------------

void Rom::buildInit() {
  pages.clear();

  codeSize = 0;
  resourcesSize = 0;
  codeShare = 0;
  resourcesShare = 0;

  errorFound = false;
}

bool Rom::addKernel() {
  /// add kernel code (support routines, Arkos Tracker and embedded fonts)
  /// @remark
  ///   Free space on kernel:
  ///     ARKOS_PLAYER (06481H) - BASIC_KUN_START_FILLER (05D2EH)
  ///     = 753H = 1875 bytes
  pages.emplace_back(0x4000, 0);  // add new page
  memcpy(pages[0].data(), bin_header_bin + 0x4000, 0x4000);

  /// add start code
  pages.emplace_back(0x4000, 0);  // add new page
  memcpy(pages[1].data(), bin_start_bin, bin_start_bin_len);
  pages[1][10] = (compiler->getParser() && compiler->getParser()->getHasFileSupport())
                     ? 0x01
                     : 0x00;

  if (opts->megaROM) {
    if (!fixKonamiMapper()) return false;
    if (!fixAscii16Mapper()) return false;
  }

  return true;
}

bool Rom::addResources() {
  int baseAddress;

  logger->debug("--> Calculating resource map base address...");

  if (opts->megaROM) {
    baseAddress = 0x8000;
    resourceSegment = pages.size() * 2;  //! resource map segment (128kb rom)
  } else {
    baseAddress = 0;
    resourceSegment = 0;  //! resource map segment (48kb rom)
  }
  resourceAddress = baseAddress + 0x0010;  //! resource map start address

  logger->debug("--> Building resource map...");

  if (!resourceManager->buildMap(resourceSegment, baseAddress)) {
    logger->add(resourceManager->logger);
    errorFound = true;
    return false;
  }

  if (resourceManager->logger->size()) logger->add(resourceManager->logger);

  resourcesSize = resourceManager->resourcesPackedSize;

  /// adjust resource map start address

  logger->debug("--> Adjusting resource map start address...");

  setResourceMapStartAddress();

  return true;
}

bool Rom::addCompiledCode() {
  vector<unsigned char> buf(COMPILE_CODE_SIZE);
  unsigned char* pbuf = buf.data();
  int copyTotalSize, copyChunkSize;

  /// write compiled code to temporary buffer
  codeSize = compiler->write(pbuf, 0x8000 + bin_start_bin_len);

  /// copy compiled code to rom pages
  if (codeSize) {
    copyChunkSize = min(codeSize, (int)(0x4000 - bin_start_bin_len));
    memcpy(pages[1].data() + bin_start_bin_len, pbuf, copyChunkSize);
    pbuf += copyChunkSize;
    copyTotalSize = codeSize - copyChunkSize;
    while (copyTotalSize) {
      pages.emplace_back(0x4000);
      copyChunkSize = min(copyTotalSize, 0x4000);
      memcpy(pages.back().data(), pbuf, copyChunkSize);
      pbuf += copyChunkSize;
      copyTotalSize -= copyChunkSize;
    }
    return true;
  } else {
    logger->error("Compiled code is empty!");
    errorFound = true;
    return false;
  }
}

void Rom::setResourceMapStartAddress() {
  pages[1][11] = resourceAddress & 0xFF;  // resource map start address
  pages[1][12] = (resourceAddress >> 8) & 0xFF;
  pages[1][13] = resourceSegment;  // resource map segment
}

/***
 * @remarks
 * https://www.msx.org/wiki/MegaROM_Mappers
 *   ASCII 8K
 *       Page (8kB)                              Switching address
 *       8000h~9FFFh (mirror: 0000h~1FFFh)       7000h (mirrors:
 *       7001h~77FFh) A000h~BFFFh (mirror: 2000h~3FFFh)       7800h
 *       (mirrors: 7801h~7FFFh)
 *   Konami MegaROMs (with/without SCC)
 *       8000h~9FFFh (mirror: 0000h~1FFFh)	    8000h (base)
 *       A000h~BFFFh (mirror: 2000h~3FFFh)	    A000h (base)
 *
 * Uses dispatch table entries (DISP_KONAMI_PATCH_*) to locate exact
 * instruction addresses for targeted patching instead of byte scanning.
 */
bool Rom::fixKonamiMapper() {
  if (opts->compileMode == BuildOptions::CompileMode::KonamiSCC ||
      opts->compileMode == BuildOptions::CompileMode::Konami4) {
    struct KonamiPatch {
      int dispIndex;
      unsigned char newByte;
    };
    static const KonamiPatch patches[] = {
        {DISP_KONAMI_PATCH_SGM_8000, 0x80},
        {DISP_KONAMI_PATCH_SGM_A000, 0xA0},
        {DISP_KONAMI_PATCH_OMSX_0, 0x80},
        {DISP_KONAMI_PATCH_OMSX_1, 0x80},
        {DISP_KONAMI_PATCH_OMSX_2, 0x80},
        {DISP_KONAMI_PATCH_OMSX_3, 0x80},
        {DISP_KONAMI_PATCH_OMSX_4, 0x80},
        {DISP_KONAMI_PATCH_BUGFIX_6800, 0x70},
        {DISP_KONAMI_PATCH_BUGFIX_8000, 0x80},
        {DISP_KONAMI_PATCH_BUGFIX_A000, 0xA0},
        {DISP_KONAMI_PATCH_VERIFY_READ, 0x80},
        {DISP_KONAMI_PATCH_VERIFY_WR0, 0x80},
        {DISP_KONAMI_PATCH_VERIFY_WR2, 0x80},
        {DISP_KONAMI_PATCH_VERIFY_RESTORE, 0x80},
    };
    static const int patchCount =
        sizeof(patches) / sizeof(patches[0]);

    unsigned char* page = pages[0].data();
    int patched = 0;

    for (int i = 0; i < patchCount; i++) {
      int tableAddr =
          def_wrapper_routines_map_table + patches[i].dispIndex * 2;
      int kernelAddr =
          bin_header_bin[tableAddr] | (bin_header_bin[tableAddr + 1] << 8);
      int offset = kernelAddr - 0x4000;

      if (offset < 0 || offset > 0x3FFF - 2) {
        logger->error("ERROR: Konami ROM format adjust failed (invalid offset " +
                      to_string(offset) + " for patch " + to_string(i) + ")");
        errorFound = true;
        return false;
      }

      page[offset + 2] = patches[i].newByte;
      patched++;
    }

    if (patched != patchCount) {
      logger->error("ERROR: Konami ROM format adjust failed (" +
                    to_string(patched) + " of " + to_string(patchCount) +
                    " locations)");
      errorFound = true;
      return false;
    }
  }
  return true;
}

//----------------------------------------------------------------------------------------------

bool Rom::fixAscii16Mapper() {
  if (opts->compileMode == BuildOptions::CompileMode::ASCII16 ||
      opts->compileMode == BuildOptions::CompileMode::ASCII16X) {
    struct Ascii16Patch {
      int dispIndex;
      unsigned char replaceByte;
      int seqLen;
      const unsigned char* seqBytes;
    };
    static const unsigned char seq_mr_change_sgm[] = {
        0xF5,                    // push af
        0xCB, 0x3F,              // srl a   (8KB seg / 2 = 16KB page)
        0x32, 0x00, 0x70,        // ld (0x7000), a
        0xF1,                    // pop af
        0xC9,                    // ret
        0x00                     // padding
    };
    static const unsigned char seq_omsx_77ff[] = {0x32, 0xFF, 0x77};
    static const Ascii16Patch patches[] = {
        {DISP_KONAMI_PATCH_SGM_8000, 0, (int)sizeof(seq_mr_change_sgm), seq_mr_change_sgm},
        {DISP_KONAMI_PATCH_BUGFIX_6800, 0x70, 0, nullptr},
        {DISP_KONAMI_PATCH_BUGFIX_8000, 0, 3, nullptr},
        {DISP_ASCII16_PATCH_BUGFIX_INC1, 0, 1, nullptr},
        {DISP_ASCII16_PATCH_BUGFIX_NOPSEQ, 0, 4, nullptr},
        {DISP_KONAMI_PATCH_OMSX_3, 0, (int)sizeof(seq_omsx_77ff), seq_omsx_77ff},
        {DISP_KONAMI_PATCH_OMSX_4, 0, (int)sizeof(seq_omsx_77ff), seq_omsx_77ff},
    };
    static const int patchCount =
        sizeof(patches) / sizeof(patches[0]);

    unsigned char* page = pages[0].data();
    int patched = 0;

    for (int i = 0; i < patchCount; i++) {
      int tableAddr =
          def_wrapper_routines_map_table + patches[i].dispIndex * 2;
      int kernelAddr =
          bin_header_bin[tableAddr] | (bin_header_bin[tableAddr + 1] << 8);
      int offset = kernelAddr - 0x4000;

      if (offset < 0 || offset > 0x3FFF) {
        logger->error("ERROR: ASCII16 ROM format adjust failed (invalid offset " +
                      to_string(offset) + " for patch " + to_string(i) + ")");
        errorFound = true;
        return false;
      }

      if (patches[i].seqBytes) {
        for (int j = 0; j < patches[i].seqLen; j++) {
          page[offset + j] = patches[i].seqBytes[j];
        }
      } else if (patches[i].replaceByte) {
        page[offset + 2] = patches[i].replaceByte;
      } else {
        for (int j = 0; j < patches[i].seqLen; j++) {
          page[offset + j] = 0x00;
        }
        // seqLen controls NOP count when seqBytes is nullptr
      }
      patched++;
    }

    if (patched != patchCount) {
      logger->error("ERROR: ASCII16 ROM format adjust failed (" +
                    to_string(patched) + " of " + to_string(patchCount) +
                    " locations)");
      errorFound = true;
      return false;
    }

    if (opts->compileMode == BuildOptions::CompileMode::ASCII16X) {
      memcpy(pages[0].data() + 0x0010, "ASCII16X", 8);

      int tableAddr =
          def_wrapper_routines_map_table + DISP_ASCII16X_PATCH_BUGFIX_AB_CHECK * 2;
      int kernelAddr =
          bin_header_bin[tableAddr] | (bin_header_bin[tableAddr + 1] << 8);
      int offset = kernelAddr - 0x4000;

      for (int j = 0; j < 14; j++) {
        pages[0].data()[offset + j] = 0x00;
      }
    }
  }
  return true;
}

//----------------------------------------------------------------------------------------------

bool Rom::writeRom(string filename) {
  ofstream file(filename, ios::binary);
  vector<unsigned char> fillerPage(0x4000, 0);
  int i, pageCount, fillerCount;

  if (!file) {
    logger->error("Cannot create output file: " + filename);
    errorFound = true;
    return false;
  }

  romSize = 0;
  pageCount = 0;

  /// write plain ROM resources page
  if (!opts->megaROM) {
    if ((pageCount = (int)resourceManager->pages.size())) {
      if (pageCount > 1) {
        logger->error(
            "Resources exceeded 16k plain ROM limit\n"
            "Try to compile it in MegaROM format by adding the -x parameter");
        errorFound = true;
        file.close();
        remove(filename.c_str());
        return false;
      }
      file.write((char*)resourceManager->pages[0].data(),
                 resourceManager->pages[0].size());
      romSize += resourceManager->pages[0].size();
    }
  }

  /// write code pages
  if (!pages.size()) {
    logger->error(
        "Code is empty!!!\nTry to write some code in your program....");
    errorFound = true;
    file.close();
    remove(filename.c_str());
    return false;
  }
  if (!opts->megaROM) {
    if (pages.size() > 2) {
      logger->error(
          "Code exceeded 16k plain ROM limit\n"
          "Try to compile it in MegaROM format by adding the -x parameter");
      errorFound = true;
      file.close();
      remove(filename.c_str());
      return false;
    }
  }

  for (i = 0; i < (int)pages.size(); i++) {
    file.write((char*)pages[i].data(), pages[i].size());
    romSize += pages[i].size();
    pageCount++;
  }

  /// write MegaROM resources pages
  if (opts->megaROM) {
    for (i = 0; i < (int)resourceManager->pages.size(); i++) {
      file.write((char*)resourceManager->pages[i].data(),
                 resourceManager->pages[i].size());
      romSize += resourceManager->pages[i].size();
      pageCount++;
    }
    /// write filler pages if necessary
    fillerCount = (pageCount % 8);
    if (fillerCount) {
      fillerCount = 8 - fillerCount;
      while (fillerCount--) {
        file.write((char*)fillerPage.data(), fillerPage.size());
        romSize += fillerPage.size();
      }
    }
  }

  file.close();

  return true;
}
