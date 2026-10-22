/***
 * @file test_temp_dir.h
 * @brief Cross-platform RAII temporary directory for file-adapter tests
 */

#ifndef TEST_TEMP_DIR_H_INCLUDED
#define TEST_TEMP_DIR_H_INCLUDED

#include <cstdio>
#include <cstdlib>
#include <string>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

inline bool testFileExists(const std::string& path) {
  FILE* file = std::fopen(path.c_str(), "rb");
  if (!file) return false;
  std::fclose(file);
  return true;
}

inline std::string testFixturePath(const std::string& relative) {
  const char* prefixes[] = {"", "../", "../../", "../../../"};
  for (const char* prefix : prefixes) {
    std::string candidate = std::string(prefix) + relative;
    if (testFileExists(candidate)) return candidate;
  }
  return relative;
}

class TestTempDir {
 public:
  TestTempDir() { create(); }
  ~TestTempDir() { cleanup(); }

  TestTempDir(const TestTempDir&) = delete;
  TestTempDir& operator=(const TestTempDir&) = delete;

  const std::string& path() const { return path_; }

  std::string file(const std::string& name) const {
    return path_ + separator() + name;
  }

 private:
  std::string path_;

  static char separator() {
#ifdef _WIN32
    return '\\';
#else
    return '/';
#endif
  }

  static std::string baseDir() {
    const char* env = std::getenv("TMPDIR");
#ifdef _WIN32
    if (!env || !*env) env = std::getenv("TEMP");
    if (!env || !*env) env = std::getenv("TMP");
    if (env && *env) return std::string(env);
    char buffer[MAX_PATH + 1] = {0};
    DWORD n = GetTempPathA(MAX_PATH + 1, buffer);
    if (n > 0) return std::string(buffer);
    return std::string(".");
#else
    if (env && *env) return std::string(env);
    return std::string("/tmp");
#endif
  }

  static bool makeDir(const std::string& path) {
#ifdef _WIN32
    return CreateDirectoryA(path.c_str(), NULL) != 0;
#else
    return mkdir(path.c_str(), 0700) == 0;
#endif
  }

  void create() {
    std::string base = baseDir();
#ifdef _WIN32
    unsigned long pid = GetCurrentProcessId();
    for (int i = 0; i < 1000; i++) {
      char suffix[64];
      std::snprintf(suffix, sizeof(suffix), "msxbas2rom_test_%lu_%d", pid, i);
      std::string candidate = base + separator() + suffix;
      if (makeDir(candidate)) {
        path_ = candidate;
        return;
      }
    }
#else
    std::string tmpl = base + "/msxbas2rom_test_XXXXXX";
    char buffer[4096];
    std::snprintf(buffer, sizeof(buffer), "%s", tmpl.c_str());
    char* made = mkdtemp(buffer);
    if (made) {
      path_ = made;
      return;
    }
    unsigned long pid = static_cast<unsigned long>(getpid());
    for (int i = 0; i < 1000; i++) {
      char suffix[64];
      std::snprintf(suffix, sizeof(suffix), "msxbas2rom_test_%lu_%d", pid, i);
      std::string candidate = base + "/" + suffix;
      if (makeDir(candidate)) {
        path_ = candidate;
        return;
      }
    }
#endif
    path_ = base;
  }

  static bool isDirectory(const std::string& path) {
#ifdef _WIN32
    DWORD attr = GetFileAttributesA(path.c_str());
    return attr != INVALID_FILE_ATTRIBUTES &&
           (attr & FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat info;
    return stat(path.c_str(), &info) == 0 && S_ISDIR(info.st_mode);
#endif
  }

  void cleanup() {
    if (path_.empty()) return;
#ifdef _WIN32
    std::string pattern = path_ + "\\*";
    WIN32_FIND_DATAA data;
    HANDLE handle = FindFirstFileA(pattern.c_str(), &data);
    if (handle != INVALID_HANDLE_VALUE) {
      do {
        if (std::string(data.cFileName) == "." ||
            std::string(data.cFileName) == "..") {
          continue;
        }
        std::string child = path_ + "\\" + data.cFileName;
        if (isDirectory(child)) {
          RemoveDirectoryA(child.c_str());
        } else {
          DeleteFileA(child.c_str());
        }
      } while (FindNextFileA(handle, &data));
      FindClose(handle);
    }
    RemoveDirectoryA(path_.c_str());
#else
    DIR* dir = opendir(path_.c_str());
    if (dir) {
      struct dirent* entry;
      while ((entry = readdir(dir)) != NULL) {
        std::string name(entry->d_name);
        if (name == "." || name == "..") continue;
        std::string child = path_ + "/" + name;
        if (isDirectory(child)) {
          rmdir(child.c_str());
        } else {
          std::remove(child.c_str());
        }
      }
      closedir(dir);
    }
    rmdir(path_.c_str());
#endif
    path_.clear();
  }
};

#endif  // TEST_TEMP_DIR_H_INCLUDED
