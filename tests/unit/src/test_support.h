/***
 * @file test_support.h
 * @brief Shared in-memory compilation harness for unit tests
 */

#ifndef TEST_SUPPORT_H_INCLUDED
#define TEST_SUPPORT_H_INCLUDED

#include <memory>
#include <string>
#include <vector>

#include "build_options.h"
#include "build_options_resolver.h"
#include "compiler.h"
#include "compiler_context.h"
#include "cpu_opcode_writer.h"
#include "file_source_resolver.h"
#include "generated_kernel_image.h"
#include "in_memory_kernel_image.h"
#include "in_memory_resource_content_source.h"
#include "in_memory_rom_writer.h"
#include "in_memory_source_resolver.h"
#include "in_memory_symbol_output_sink.h"
#include "kernel_image.h"
#include "lexer.h"
#include "parser.h"
#include "resource_content_source.h"
#include "rom.h"
#include "rom_artifact.h"
#include "rom_writer.h"
#include "source_resolver.h"
#include "symbol_output_sink.h"
#include "z80.h"

inline std::shared_ptr<InMemoryKernelImage> makeTestKernelImage() {
  GeneratedKernelImage generated;
  std::vector<unsigned char> bytes(generated.bytes(),
                                   generated.bytes() + generated.size());
  std::vector<unsigned char> start(
      generated.startBytes(), generated.startBytes() + generated.startSize());
  return std::make_shared<InMemoryKernelImage>(bytes, start);
}

/***
 * @brief Owns a complete set of in-memory compilation adapters.
 *
 * Every test constructs its own harness so compilation tests never rely on
 * process-wide adapter installation or on temporary source files.
 */

/***
 * @brief Per-test source resolver with in-memory sources and an explicit
 * file fallback for adapter/diagnostic tests.
 */
class TestSourceResolver : public SourceResolver {
 public:
  InMemorySourceResolver memory;
  FileSourceResolver files;
  std::string error;

  void add(const std::string& name, const std::string& content) {
    memory.add(name, content);
  }

  bool resolve(const std::string& name, SourceDocument& document) override {
    if (memory.resolve(name, document)) {
      error.clear();
      return true;
    }
    if (memory.lastError() != "File doesn't exist") {
      error = memory.lastError();
      return false;
    }
    if (files.resolve(name, document)) {
      error.clear();
      return true;
    }
    error = files.lastError();
    return false;
  }

  std::string lastError() const override { return error; }
};

inline std::shared_ptr<Compiler> makeTestCompiler(
    std::shared_ptr<ICpuOpcodeWriter> cpu = nullptr) {
  if (!cpu) cpu = std::make_shared<Z80OpcodeWriter>();
  return std::make_shared<Compiler>(
      cpu, makeTestKernelImage(),
      std::make_shared<InMemoryResourceContentSource>(),
      std::make_shared<InMemorySymbolOutputSink>());
}

inline std::shared_ptr<CompilerContext> makeTestCompilerContext() {
  return std::make_shared<CompilerContext>(
      makeTestKernelImage(),
      std::make_shared<InMemoryResourceContentSource>(),
      std::make_shared<InMemorySymbolOutputSink>());
}

class CompilationHarness {
 public:
  std::shared_ptr<InMemorySourceResolver> source;
  std::shared_ptr<InMemoryKernelImage> kernel;
  std::shared_ptr<InMemoryResourceContentSource> resources;
  std::shared_ptr<InMemorySymbolOutputSink> symbols;
  std::shared_ptr<InMemoryRomWriter> romWriter;

  CompilationHarness()
      : source(std::make_shared<InMemorySourceResolver>()),
        kernel(makeTestKernelImage()),
        resources(std::make_shared<InMemoryResourceContentSource>()),
        symbols(std::make_shared<InMemorySymbolOutputSink>()),
        romWriter(std::make_shared<InMemoryRomWriter>()) {}

  std::shared_ptr<Lexer> makeLexer() {
    return std::make_shared<Lexer>(source);
  }

  std::shared_ptr<Compiler> makeCompiler(
      std::shared_ptr<ICpuOpcodeWriter> cpu = nullptr) {
    if (!cpu) cpu = std::make_shared<Z80OpcodeWriter>();
    return std::make_shared<Compiler>(cpu, kernel, resources, symbols);
  }

  std::shared_ptr<CompilerContext> makeContext() {
    return std::make_shared<CompilerContext>(kernel, resources, symbols);
  }

  std::shared_ptr<Compiler> compileSource(
      const std::string& logicalName, const std::string& content,
      std::shared_ptr<BuildOptions> opts = nullptr,
      std::shared_ptr<Parser>* parserOut = nullptr) {
    source->add(logicalName, content);
    if (!opts) {
      opts = std::make_shared<BuildOptions>();
      build_options::setInputFilename(opts, logicalName);
    }

    auto lexer = makeLexer();
    if (!lexer->load(opts)) return nullptr;
    if (!lexer->evaluate()) return nullptr;

    auto parser = std::make_shared<Parser>();
    if (!parser->evaluate(lexer)) return nullptr;

    auto compiler = makeCompiler();
    if (!compiler->build(parser)) return nullptr;

    if (parserOut) *parserOut = parser;
    return compiler;
  }

  std::shared_ptr<RomArtifact> buildRom(
      std::shared_ptr<Compiler> compiler) {
    Rom rom(romWriter);
    if (!rom.build(compiler)) return nullptr;
    return rom.getArtifact();
  }
};


inline std::shared_ptr<KernelImage> kernelImage() {
  return makeTestKernelImage();
}

inline std::shared_ptr<Compiler> compileSource(
    const std::string& logicalName, const std::string& source,
    std::shared_ptr<BuildOptions> opts = nullptr) {
  CompilationHarness harness;
  return harness.compileSource(logicalName, source, opts);
}

inline std::vector<unsigned char> artifactBytes(const RomArtifact& artifact) {
  std::vector<unsigned char> bytes;
  for (const auto& page : artifact.pages) {
    bytes.insert(bytes.end(), page.begin(), page.end());
  }
  return bytes;
}

#endif  // TEST_SUPPORT_H_INCLUDED
