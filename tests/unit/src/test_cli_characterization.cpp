/***
 * @file test_cli_characterization.cpp
 * @brief CLI composition-root characterization tests
 */

// NOLINTBEGIN
// test-inventory: file-adapter

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <io.h>
#define dup _dup
#define dup2 _dup2
#define fileno _fileno
#define close _close
#else
#include <unistd.h>
#endif

#include "doctest/doctest.h"
#include "fswrapper.h"
#include "runner.h"

struct CliResult {
  int exitCode;
  std::string output;
};

static CliResult runCliCaptured(const std::vector<std::string>& args) {
  std::vector<char*> argv;
  for (const std::string& arg : args) {
    argv.push_back(const_cast<char*>(arg.c_str()));
  }
  argv.push_back(nullptr);

  std::fflush(stdout);
  FILE* capture = std::tmpfile();
  REQUIRE(capture != nullptr);
  int saved = dup(fileno(stdout));
  REQUIRE(saved >= 0);
  REQUIRE(dup2(fileno(capture), fileno(stdout)) >= 0);

  int exitCode = runCli(static_cast<int>(argv.size()) - 1, argv.data());

  std::fflush(stdout);
  dup2(saved, fileno(stdout));
  close(saved);

  std::rewind(capture);
  std::string output;
  char buffer[4096];
  size_t count;
  while ((count = std::fread(buffer, 1, sizeof(buffer), capture)) > 0) {
    output.append(buffer, count);
  }
  std::fclose(capture);

  CliResult result;
  result.exitCode = exitCode;
  result.output = output;
  return result;
}

static std::string writeProgram(const std::string& filename,
                                const std::string& content) {
  createPath("tmp/cli_characterization");
  const std::string path = "tmp/cli_characterization/" + filename;
  std::ofstream out(path.c_str());
  out << content;
  out.close();
  return path;
}

TEST_SUITE("CLI characterization") {
  TEST_CASE("Normal compilation output and exit code are preserved") {
    const std::string program =
        writeProgram("normal.bas", "10 PRINT \"HI\"\n20 END\n");
    CliResult result = runCliCaptured({"msxbas2rom", program});

    CHECK(result.exitCode == 0);
    CHECK(result.output.find("MSXBAS2ROM") != std::string::npos);
    CHECK(result.output.find("Converting ") != std::string::npos);
    CHECK(result.output.find("Compilation finished with success") !=
          std::string::npos);

    std::remove("tmp/cli_characterization/normal.rom");
  }

  TEST_CASE("Quiet mode suppresses output and preserves the exit code") {
    const std::string program =
        writeProgram("quiet.bas", "10 PRINT \"HI\"\n20 END\n");
    CliResult result = runCliCaptured({"msxbas2rom", "-q", program});

    CHECK(result.exitCode == 0);
    CHECK(result.output.empty() == true);

    std::remove("tmp/cli_characterization/quiet.rom");
  }

  TEST_CASE("Debug mode adds diagnostics") {
    const std::string program =
        writeProgram("debug.bas", "10 PRINT \"HI\"\n20 END\n");
    CliResult result = runCliCaptured({"msxbas2rom", "-d", program});

    CHECK(result.exitCode == 0);
    CHECK(result.output.find("Displaying lexical analysis") !=
          std::string::npos);

    std::remove("tmp/cli_characterization/debug.rom");
  }

  TEST_CASE("Missing input reports the existing diagnostic and exit code") {
    CliResult result = runCliCaptured(
        {"msxbas2rom", "tmp/cli_characterization/missing-input.bas"});

    CHECK(result.exitCode == 1);
    CHECK(result.output.find("ERROR: Input file not found!") !=
          std::string::npos);
  }

  TEST_CASE("Symbol output and exit code are preserved") {
    const std::string program =
        writeProgram("symbols.bas", "10 PRINT \"HI\"\n20 END\n");
    CliResult result = runCliCaptured({"msxbas2rom", "--symbol", program});

    CHECK(result.exitCode == 0);
    CHECK(result.output.find("Symbols file created") != std::string::npos);
    CHECK(fileExists("tmp/cli_characterization/symbols.symbol") == true);

    std::remove("tmp/cli_characterization/symbols.rom");
    std::remove("tmp/cli_characterization/symbols.symbol");
  }

  TEST_CASE("Automatic ASCII8 retry output and exit code are preserved") {
    std::string content;
    int line = 10;
    for (int i = 0; i < 200; i++) {
      content += std::to_string(line++) + " A=" + std::to_string(i) + "\n";
      content += std::to_string(line++) + " B=A+" + std::to_string(i) + "\n";
      content += std::to_string(line++) + " C=A*B\n";
      content += std::to_string(line++) + " PRINT C\n";
    }
    content += std::to_string(line) + " END\n";
    const std::string program = writeProgram("retry.bas", content);

    CliResult result = runCliCaptured({"msxbas2rom", "-a", program});

    CHECK(result.exitCode == 0);
    CHECK(result.output.find("Plain ROM overflow detected") !=
          std::string::npos);
    CHECK(result.output.find("ASCII8 MegaROM") != std::string::npos);
    CHECK(result.output.find("Compilation finished with success") !=
          std::string::npos);

    std::remove("tmp/cli_characterization/retry.rom");
    std::remove("tmp/cli_characterization/retry[ASCII8].rom");
  }
}

// NOLINTEND
