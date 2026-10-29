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
#include <map>

#include "build_options.h"
#include "compiler.h"
#include "compiler_context.h"
#include "cpu_workspace_context.h"
#include "compiler_hooks.h"
#include "dispatch_fixup_resolver.h"
#include "kernel_image.h"
#include "kernel_module_library.h"
#include "kernel_packer.h"
#include "logger.h"
#include "parser.h"
#include "resource_manager.h"
#include "rom_artifact.h"
#include "rom_writer.h"

// Shared dispatch entry for the 14-byte AB guard at megarom_ascii8_bug_fix.
// ASCII16X and Konami use the same generated index; Konami must also NOP the
// guard so the boot bank mapping always runs.
static constexpr int DISP_MEGAROM_PATCH_BUGFIX_AB_CHECK =
    DISP_ASCII16X_PATCH_BUGFIX_AB_CHECK;

//----------------------------------------------------------------------------------------------

Rom::Rom(shared_ptr<RomWriter> outputWriter,
         shared_ptr<KernelModuleLibrary> kernelModules) {
  logger = make_shared<Logger>();
  writer = outputWriter;
  moduleLibrary = kernelModules;
}

Rom::~Rom() = default;

shared_ptr<Logger> Rom::getLogger() {
  return logger;
}

shared_ptr<RomArtifact> Rom::getArtifact() const {
  return artifact;
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

  logger->debug("Adding overflow modules...");
  if (!addOverflowModules()) return false;

  // add resources pages

  logger->debug("Adding resources...");

  if (!addResources()) return false;

  // assemble in-memory artifact

  logger->debug("Assembling ROM artifact...");

  if (!assembleArtifact()) return false;

  // persist artifact through the outer writer

  logger->debug("Writing ROM file...");

  if (!writer) {
    logger->error("ROM writer is not configured");
    errorFound = true;
    return false;
  }

  string error;
  if (!writer->write(*artifact, opts->outputFilename, error)) {
    logger->error(error.empty() ? "Cannot create output file: " +
                                      opts->outputFilename
                                : error);
    errorFound = true;
    return false;
  }

  // calculate shares

  logger->debug("Calculating ROM statistics...");

  romSize = artifact->romSize;
  codeSize = artifact->codeSize;
  resourcesSize = artifact->resourcesSize;

  romSizeFloat = romSize;
  kernelShare = (0x4000 / romSizeFloat) * 100;
  codeShare = (codeSize / romSizeFloat) * 100;
  resourcesShare = (resourcesSize / romSizeFloat) * 100;

  artifact->kernelShare = kernelShare;
  artifact->codeShare = codeShare;
  artifact->resourcesShare = resourcesShare;
  artifact->diagnostics = logger->trace().toString();

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
  shared_ptr<KernelImage> kernel = compiler->getKernelImage();
  if (!kernel || !kernel->bytes() || kernel->size() < 0x8000 ||
      !kernel->startBytes() || kernel->startSize() > 0x4000) {
    logger->error("Kernel image is unavailable");
    errorFound = true;
    return false;
  }

  pages.emplace_back(0x4000, 0);  // add new page

  std::map<unsigned int, unsigned int> stubAddresses;

  if (moduleLibrary) {
    KernelPackingRequest request;
    request.library = moduleLibrary.get();
    request.usedDispatch = compiler->getContext()->getUsedDispatch();
    request.pinGameGraphics = opts->pinGameGraphics;
    for (std::size_t i = 0; i < opts->forceOverflowModules.size(); i++) {
      request.forceOverflowModules.insert(opts->forceOverflowModules[i]);
    }
    request.dispatcherAddress = 0;
    if (opts->megaROM) {
      request.usedDispatch.insert(DISP_MR_CALL);
      request.usedDispatch.insert(DISP_MR_JUMP);
      request.usedDispatch.insert(DISP_MR_GET_DATA);
      request.requiredModules.insert("dispatcher");
    }
    request.kernelStart = 0x4000;
    request.kernelEnd = 0x8000;
    request.tableBaseAddress = 0;
    request.romType =
        opts->megaROM ? KernelRomType::MegaRom : KernelRomType::Plain;
    request.startCodeSize = static_cast<unsigned int>(kernel->startSize());
    request.userCodeSize = static_cast<unsigned int>(compiler->getCodeSize());
    if (opts->megaROM) {
      request.overflowStart = 0x8000;
      request.overflowCapacity = 0x200000;
    } else {
      request.overflowStart = 0x8000 + request.startCodeSize +
                              request.userCodeSize;
      request.overflowCapacity =
          request.overflowStart < 0xC000 ? 0xC000 - request.overflowStart : 0;
    }

    KernelPackingResult packing = KernelPacker::pack(request);
    for (std::size_t i = 0; i < packing.diagnostics.size(); i++) {
      logger->debug(packing.diagnostics[i]);
    }
    if (!packing.ok) {
      logger->error(packing.error);
      errorFound = true;
      return false;
    }

    KernelCompositionResult composition =
        KernelComposer::compose(*moduleLibrary, packing, 0x4000, 0x8000);
    if (!composition.ok) {
      logger->error(composition.error);
      errorFound = true;
      return false;
    }
    memcpy(pages[0].data(), composition.image.data(), 0x4000);

    for (std::size_t i = 0; i < packing.dispatches.size(); i++) {
      stubAddresses[packing.dispatches[i].dispatchIndex] =
          packing.dispatches[i].stubAddress;
    }
    compiler->getContext()->getDispatchAddresses() = stubAddresses;

    overflowPlacements.clear();
    for (std::size_t i = 0; i < packing.placements.size(); i++) {
      if (!packing.placements[i].resident) {
        overflowPlacements.push_back(packing.placements[i]);
      }
    }
    packingResult = packing;
    logger->debug("Dispatcher address: 0x" +
                  to_string(packing.dispatcherAddress));
    logger->debug("Central table: 0x" + to_string(packing.tableBaseAddress) +
                  " size " + to_string(packing.tableSize));
    for (std::size_t i = 0; i < packing.imports.size(); i++) {
      logger->debug("Import " + packing.imports[i].targetExport +
                    " stub 0x" + to_string(packing.imports[i].stubAddress) +
                    " target 0x" + to_string(packing.imports[i].address) +
                    (packing.imports[i].paged ? " paged" : " resident"));
    }
    for (std::size_t i = 0; i < packing.placements.size(); i++) {
      const KernelModulePlacement& p = packing.placements[i];
      logger->debug(std::string("placement ") + p.moduleId +
                    (p.resident ? " resident 0x" : " overflow 0x") +
                    to_string(p.address) + " segment " +
                    to_string(p.segment));
    }
  } else {
    memcpy(pages[0].data(), kernel->bytes() + 0x4000, 0x4000);
    const std::vector<DispatchFixup>& fixups =
        compiler->getContext()->getDispatchFixups();
    for (std::size_t i = 0; i < fixups.size(); i++) {
      stubAddresses[fixups[i].dispatchIndex] =
          kernel->dispatchAddress(fixups[i].dispatchIndex);
    }
    for (unsigned int i = 0; i < DISP_ENTRIES; i++) {
      compiler->getContext()->getDispatchAddresses()[i] =
          kernel->dispatchAddress(i);
    }
  }

  std::string fixupError;
  if (!applyDispatchFixups(compiler->getContext()->getDispatchFixups(),
                           stubAddresses,
                           compiler->getContext()->getCpu()->context->getCode(),
                           fixupError)) {
    logger->error(fixupError);
    errorFound = true;
    return false;
  }

  /// add start code
  pages.emplace_back(0x4000, 0);  // add new page
  memcpy(pages[1].data(), kernel->startBytes(), kernel->startSize());
  pages[1][10] = (compiler->getParser() && compiler->getParser()->getHasFileSupport())
                     ? 0x01
                     : 0x00;
  if (!patchStartCode()) return false;

  if (opts->megaROM) {
    if (!fixKonamiMapper()) return false;
    if (!fixAscii16Mapper()) return false;
  }

  return true;
}

bool Rom::patchStartCode() {
  if (!moduleLibrary) return true;

  std::map<std::string, unsigned int> stubs;
  for (std::size_t i = 0; i < packingResult.dispatches.size(); i++) {
    stubs[packingResult.dispatches[i].exportName] =
        packingResult.dispatches[i].stubAddress;
  }
  for (std::size_t i = 0; i < packingResult.imports.size(); i++) {
    stubs[packingResult.imports[i].targetExport] =
        packingResult.imports[i].stubAddress;
  }

  const KernelModule* screenModule = nullptr;
  for (std::size_t i = 0; i < moduleLibrary->modules().size(); i++) {
    if (moduleLibrary->modules()[i].findExport("C7369")) {
      screenModule = &moduleLibrary->modules()[i];
      break;
    }
  }
  if (!screenModule) return true;
  if (screenModule->hasFixedAddress) {
    logger->debug(
        "Start code C7369 call remains at its fixed kernel address");
    return true;
  }

  shared_ptr<KernelImage> kernel = compiler->getKernelImage();
  std::size_t startSize = kernel ? kernel->startSize() : 0;
  std::size_t callOffset = 0;
  unsigned int matches = 0;
  for (std::size_t i = 0; i + 2 < startSize; i++) {
    if (pages[1][i] != 0xCD || pages[1][i + 1] != 0x69 ||
        pages[1][i + 2] != 0x73) {
      continue;
    }
    callOffset = i;
    matches++;
  }
  if (matches == 0) {
    logger->debug(
        "Start code uses BIOS CHGMOD; no C7369 call to patch");
    return true;
  }
  if (matches > 1) {
    logger->error(
        "start code must not contain more than one call to 'C7369'");
    errorFound = true;
    return false;
  }

  std::map<std::string, unsigned int>::const_iterator stub =
      stubs.find("C7369");
  if (stub == stubs.end()) {
    logger->error(
        "start code requires an import stub for 'C7369' (game graphics)");
    errorFound = true;
    return false;
  }

  pages[1][callOffset + 1] = static_cast<unsigned char>(stub->second & 0xFF);
  pages[1][callOffset + 2] =
      static_cast<unsigned char>((stub->second >> 8) & 0xFF);
  logger->debug("Start code C7369 call patched to stub 0x" +
                to_string(stub->second));
  return true;
}

bool Rom::addOverflowModules() {
  if (overflowPlacements.empty()) return true;

  if (pages.size() < 2) {
    logger->error("Code page is unavailable for overflow modules");
    errorFound = true;
    return false;
  }

  unsigned int baseSegment =
      opts->megaROM ? static_cast<unsigned int>(pages.size()) * 2 : 0;

  std::map<std::string, unsigned int> importStubs;
  for (std::size_t i = 0; i < packingResult.dispatches.size(); i++) {
    importStubs[packingResult.dispatches[i].exportName] =
        packingResult.dispatches[i].stubAddress;
  }
  for (std::size_t i = 0; i < packingResult.imports.size(); i++) {
    importStubs[packingResult.imports[i].targetExport] =
        packingResult.imports[i].stubAddress;
  }

  for (std::size_t i = 0; i < overflowPlacements.size(); i++) {
    const KernelModulePlacement& placement = overflowPlacements[i];
    const KernelModule* module = nullptr;
    for (std::size_t j = 0; j < moduleLibrary->modules().size(); j++) {
      if (moduleLibrary->modules()[j].id == placement.moduleId) {
        module = &moduleLibrary->modules()[j];
        break;
      }
    }
    if (!module) {
      logger->error("Overflow module is missing: " + placement.moduleId);
      errorFound = true;
      return false;
    }

    if (placement.address < 0x8000 ||
        placement.address + module->size() > 0xC000) {
      logger->error("Overflow module is outside the 0x8000 window: " +
                    placement.moduleId);
      errorFound = true;
      return false;
    }

    std::vector<unsigned char> bytes = module->bytes;
    if (module->assemblyBase != placement.address) {
      std::string error;
      if (!applyRelocations(bytes, module->assemblyBase, placement.address,
                            module->relocations, error)) {
        logger->error("Overflow module '" + module->id + "': " + error);
        errorFound = true;
        return false;
      }
    }
    if (!module->imports.empty()) {
      std::string error;
      if (!applyModuleImports(bytes, module->imports, importStubs, error)) {
        logger->error("Overflow module '" + module->id + "': " + error);
        errorFound = true;
        return false;
      }
    }

    std::size_t offset = placement.address - 0x8000;
    if (opts->megaROM) {
      unsigned int segment = baseSegment + placement.segment;
      logger->debug("MegaROM overflow module '" + placement.moduleId +
                    "' at address 0x" + to_string(placement.address) +
                    " segment " + to_string(segment) +
                    " (offset " + to_string(offset) + ")");
      if (segment >= 256) {
        logger->error("MegaROM overflow exceeds the 2 MB segment limit");
        errorFound = true;
        return false;
      }
      std::size_t pageIndex = segment / 2;
      while (pages.size() <= pageIndex) pages.emplace_back(0x4000, 0);
      offset += (segment % 2) * 0x2000;
      if (offset + bytes.size() > pages[pageIndex].size()) {
        logger->error("Overflow module exceeds its MegaROM segment: " +
                      placement.moduleId);
        errorFound = true;
        return false;
      }
      memcpy(pages[pageIndex].data() + offset, bytes.data(), bytes.size());

      for (std::size_t k = 0; k < packingResult.dispatches.size(); k++) {
        const KernelDispatchPlacement& dispatch =
            packingResult.dispatches[k];
        if (!dispatch.paged || dispatch.moduleId != placement.moduleId) {
          continue;
        }
        std::size_t stubOffset = dispatch.stubAddress - 0x4000;
        pages[0][stubOffset + 3] = static_cast<unsigned char>(segment);
        logger->debug("Paged dispatch " + to_string(dispatch.dispatchIndex) +
                      " stub 0x" + to_string(dispatch.stubAddress) +
                      " target 0x" + to_string(dispatch.address) +
                      " segment " + to_string(segment));
      }
      for (std::size_t k = 0; k < packingResult.imports.size(); k++) {
        const KernelImportPlacement& imported = packingResult.imports[k];
        if (!imported.paged || imported.moduleId != placement.moduleId) {
          continue;
        }
        std::size_t stubOffset = imported.stubAddress - 0x4000;
        pages[0][stubOffset + 3] = static_cast<unsigned char>(segment);
        logger->debug("Paged import " + imported.targetExport +
                      " stub 0x" + to_string(imported.stubAddress) +
                      " target 0x" + to_string(imported.address) +
                      " segment " + to_string(segment));
      }
    } else {
      if (offset + bytes.size() > pages[1].size()) {
        logger->error("Overflow module exceeds the Plain ROM code page: " +
                      placement.moduleId);
        errorFound = true;
        return false;
      }
      memcpy(pages[1].data() + offset, bytes.data(), bytes.size());
    }
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

  shared_ptr<KernelImage> kernel = compiler->getKernelImage();
  int startSize = kernel ? static_cast<int>(kernel->startSize()) : 0;

  /// write compiled code to temporary buffer
  codeSize = compiler->write(pbuf, 0x8000 + startSize);

  /// copy compiled code to rom pages
  if (codeSize) {
    copyChunkSize = min(codeSize, (int)(0x4000 - startSize));
    memcpy(pages[1].data() + startSize, pbuf, copyChunkSize);
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
    shared_ptr<KernelImage> kernel = compiler->getKernelImage();
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
      int kernelAddr = kernel->dispatchAddress(patches[i].dispIndex);
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

    // NOP the shared AB guard so the boot bank writes run unconditionally.
    int guardOffset =
        kernel->dispatchAddress(DISP_MEGAROM_PATCH_BUGFIX_AB_CHECK) - 0x4000;
    if (guardOffset < 0 || guardOffset > 0x3FFF - 13) {
      logger->error(
          "ERROR: Konami ROM format adjust failed (invalid offset " +
          to_string(guardOffset) + " for AB guard)");
      errorFound = true;
      return false;
    }
    for (int i = 0; i < 14; i++) {
      page[guardOffset + i] = 0x00;
    }
    patched++;

    if (patched != patchCount + 1) {
      logger->error("ERROR: Konami ROM format adjust failed (" +
                    to_string(patched) + " of " + to_string(patchCount + 1) +
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
    shared_ptr<KernelImage> kernel = compiler->getKernelImage();
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
      int kernelAddr = kernel->dispatchAddress(patches[i].dispIndex);
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

      int kernelAddr =
          kernel->dispatchAddress(DISP_ASCII16X_PATCH_BUGFIX_AB_CHECK);
      int offset = kernelAddr - 0x4000;

      for (int j = 0; j < 14; j++) {
        pages[0].data()[offset + j] = 0x00;
      }
    }
  }
  return true;
}

//----------------------------------------------------------------------------------------------

bool Rom::assembleArtifact() {
  int pageCount = 0;
  vector<unsigned char> fillerPage(0x4000, 0);

  artifact = make_shared<RomArtifact>();
  artifact->megaROM = opts->megaROM;
  artifact->codeSize = codeSize;
  artifact->resourcesSize = resourcesSize;
  artifact->resourceMapSegment = resourceSegment;
  artifact->resourceMapAddress = resourceAddress;

  /// plain ROM resources page
  if (!opts->megaROM) {
    int resourcePages = (int)resourceManager->pages.size();
    if (resourcePages) {
      if (resourcePages > 1) {
        logger->error(
            "Resources exceeded 16k plain ROM limit\n"
            "Try to compile it in MegaROM format by adding the -x parameter");
        errorFound = true;
        return false;
      }
      artifact->pages.push_back(resourceManager->pages[0]);
      artifact->romSize += resourceManager->pages[0].size();
      pageCount++;
    }
  }

  if (pages.empty()) {
    logger->error(
        "Code is empty!!!\nTry to write some code in your program....");
    errorFound = true;
    return false;
  }

  if (!opts->megaROM && pages.size() > 2) {
    logger->error(
        "Code exceeded 16k plain ROM limit\n"
        "Try to compile it in MegaROM format by adding the -x parameter");
    errorFound = true;
    return false;
  }

  for (std::size_t i = 0; i < pages.size(); i++) {
    artifact->pages.push_back(pages[i]);
    artifact->romSize += pages[i].size();
    pageCount++;
  }

  /// MegaROM resources pages
  if (opts->megaROM) {
    for (std::size_t i = 0; i < resourceManager->pages.size(); i++) {
      artifact->pages.push_back(resourceManager->pages[i]);
      artifact->romSize += resourceManager->pages[i].size();
      pageCount++;
    }
    int fillerCount = pageCount % 8;
    if (fillerCount) {
      fillerCount = 8 - fillerCount;
      while (fillerCount--) {
        artifact->pages.push_back(fillerPage);
        artifact->romSize += fillerPage.size();
      }
    }
  }

  artifact->diagnostics = logger->trace().toString();
  return true;
}
