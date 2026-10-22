/***
 * @file test_compilation_use_case.cpp
 * @brief Application compilation workflow characterization tests
 */

// NOLINTBEGIN

#include <memory>
#include <string>

#include "build_options.h"
#include "build_options_resolver.h"
#include "compilation_request.h"
#include "compilation_result.h"
#include "compilation_use_case.h"
#include "doctest/doctest.h"
#include "in_memory_resource_content_source.h"
#include "in_memory_rom_writer.h"
#include "in_memory_source_resolver.h"
#include "in_memory_symbol_output_sink.h"
#include "test_support.h"
#include "rom_artifact.h"
#include "symbol_output_sink.h"

static std::shared_ptr<BuildOptions> makeOptions(
    const std::string& filename, bool autoROM = false) {
  auto opts = std::make_shared<BuildOptions>();
  build_options::setInputFilename(opts, filename);
  opts->autoROM = autoROM;
  return opts;
}

TEST_SUITE("Compilation use case") {
  TEST_CASE("Successful compilation uses only in-memory adapters") {
    auto source = std::make_shared<InMemorySourceResolver>();
    source->add("memory.bas", "10 PRINT \"OK\"\n20 END\n");
    auto resources = std::make_shared<InMemoryResourceContentSource>();
    auto sink = std::make_shared<InMemorySymbolOutputSink>();
    auto writer = std::make_shared<InMemoryRomWriter>();

    auto opts = makeOptions("memory.bas");
    opts->symbols = BuildOptions::SymbolsMode::Symbol;

    CompilationRequest request(opts, source, makeTestKernelImage(), resources,
                               sink, writer);

    CompilationUseCase useCase;
    CompilationResult result = useCase.execute(request);

    CHECK(result.success == true);
    CHECK(result.exitCode == 0);
    CHECK(result.retriedWithAscii8 == false);
    CHECK(result.output.find("Compilation finished with success") !=
          std::string::npos);
    REQUIRE(result.artifact.get() != nullptr);
    CHECK(result.artifact->pages.size() >= 2);
    CHECK(writer->hasArtifact() == true);
    CHECK(writer->artifact().pages.size() == result.artifact->pages.size());
    CHECK(sink->hasText("memory.symbol") == true);
    CHECK(result.diagnosticsText.size() > 0);
  }

  TEST_CASE("Failed source resolution returns structured failure") {
    auto source = std::make_shared<InMemorySourceResolver>();
    auto resources = std::make_shared<InMemoryResourceContentSource>();
    auto sink = std::make_shared<InMemorySymbolOutputSink>();
    auto writer = std::make_shared<InMemoryRomWriter>();

    CompilationRequest request(makeOptions("missing.bas"), source,
                               makeTestKernelImage(), resources, sink, writer);

    CompilationUseCase useCase;
    CompilationResult result = useCase.execute(request);

    CHECK(result.success == false);
    CHECK(result.exitCode == 1);
    CHECK(result.output.find("File doesn't exist") != std::string::npos);
    CHECK(writer->hasArtifact() == false);
    CHECK(sink->hasText("missing.symbol") == false);
  }

  TEST_CASE("Automatic ASCII8 retry is exercised without the CLI") {
    std::string content;
    int line = 10;
    for (int i = 0; i < 200; i++) {
      content += std::to_string(line++) + " A=" + std::to_string(i) + "\n";
      content += std::to_string(line++) + " B=A+" + std::to_string(i) + "\n";
      content += std::to_string(line++) + " C=A*B\n";
      content += std::to_string(line++) + " PRINT C\n";
    }
    content += std::to_string(line) + " END\n";

    auto source = std::make_shared<InMemorySourceResolver>();
    source->add("retry.bas", content);
    auto resources = std::make_shared<InMemoryResourceContentSource>();
    auto sink = std::make_shared<InMemorySymbolOutputSink>();
    auto writer = std::make_shared<InMemoryRomWriter>();

    CompilationRequest request(makeOptions("retry.bas", true), source,
                               makeTestKernelImage(), resources, sink, writer);

    CompilationUseCase useCase;
    CompilationResult result = useCase.execute(request);

    CHECK(result.success == true);
    CHECK(result.retriedWithAscii8 == true);
    CHECK(result.output.find("Plain ROM overflow detected") !=
          std::string::npos);
    REQUIRE(result.artifact.get() != nullptr);
    CHECK(result.artifact->megaROM == true);
    CHECK(writer->hasArtifact() == true);
  }
}

// NOLINTEND
