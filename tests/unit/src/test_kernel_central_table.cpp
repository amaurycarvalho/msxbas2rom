/***
 * @file test_kernel_central_table.cpp
 * @brief Runtime central jump table stub generation tests
 */

// NOLINTBEGIN

#include <map>
#include <string>
#include <vector>

#include "doctest/doctest.h"
#include "kernel_central_table.h"

TEST_SUITE("Kernel central table") {
  TEST_CASE("Resident stub is a three-byte jp") {
    std::vector<unsigned char> bytes =
        KernelCentralTable::residentStub(0x4567);

    REQUIRE(bytes.size() == 3);
    CHECK(bytes[0] == 0xC3);
    CHECK(bytes[1] == 0x67);
    CHECK(bytes[2] == 0x45);
  }

  TEST_CASE("Paged stub carries dispatcher, segment, and address") {
    std::vector<unsigned char> bytes =
        KernelCentralTable::pagedStub(0x4200, 3, 0x8123);

    REQUIRE(bytes.size() == 6);
    CHECK(bytes[0] == 0xCD);
    CHECK(bytes[1] == 0x00);
    CHECK(bytes[2] == 0x42);
    CHECK(bytes[3] == 0x03);
    CHECK(bytes[4] == 0x23);
    CHECK(bytes[5] == 0x81);
  }

  TEST_CASE("Table assigns stub addresses in dispatch order") {
    std::vector<KernelDispatchStub> stubs;
    stubs.push_back({7, false, 0x5000, 0});
    stubs.push_back({2, true, 0x9000, 4});

    std::vector<unsigned char> bytes;
    std::map<unsigned int, unsigned int> addresses;
    std::string error;

    REQUIRE(KernelCentralTable::build(stubs, 0x4200, 0x4100, bytes,
                                      addresses, error) == true);
    REQUIRE(bytes.size() == 9);
    CHECK(addresses[2] == 0x4100);
    CHECK(addresses[7] == 0x4106);
    CHECK(bytes[0] == 0xCD);
    CHECK(bytes[3] == 0x04);
    CHECK(bytes[6] == 0xC3);
    CHECK(bytes[7] == 0x00);
    CHECK(bytes[8] == 0x50);
  }

  TEST_CASE("Unused entries produce no stub bytes") {
    std::vector<KernelDispatchStub> all;
    all.push_back({1, false, 0x5000, 0});
    all.push_back({2, false, 0x5010, 0});
    all.push_back({3, false, 0x5020, 0});

    std::vector<KernelDispatchStub> used;
    used.push_back(all[2]);

    std::vector<unsigned char> bytes;
    std::map<unsigned int, unsigned int> addresses;
    std::string error;

    REQUIRE(KernelCentralTable::build(used, 0x4200, 0x4100, bytes,
                                      addresses, error) == true);
    CHECK(bytes.size() == 3);
    CHECK(addresses.size() == 1);
    CHECK(addresses[3] == 0x4100);
  }

  TEST_CASE("Table rejects duplicates and page overflow") {
    std::vector<KernelDispatchStub> duplicate;
    duplicate.push_back({1, false, 0x5000, 0});
    duplicate.push_back({1, false, 0x5010, 0});

    std::vector<unsigned char> bytes;
    std::map<unsigned int, unsigned int> addresses;
    std::string error;

    CHECK(KernelCentralTable::build(duplicate, 0x4200, 0x4100, bytes,
                                    addresses, error) == false);
    CHECK(error.find("duplicate") != std::string::npos);

    std::vector<KernelDispatchStub> overflow;
    overflow.push_back({1, true, 0x9000, 0});
    CHECK(KernelCentralTable::build(overflow, 0x4200, 0x7FFE, bytes,
                                    addresses, error) == false);
    CHECK(error.find("exceeds") != std::string::npos);
  }
}

// NOLINTEND
