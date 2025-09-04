/***
 * @file symbols.cpp
 * @brief Symbols class implementation
 * @author Amaury Carvalho (2019-2025)
 * @note
 */

#include "symbols.h"

/***
 * @name FileNode class code
 */

FileNode::~FileNode() {
  close();
}

bool FileNode::open() {
  this->clear();
  handle = fopen(name.c_str(), "rb");
  return handle;
}

bool FileNode::eof() {
  if (handle) {
    return feof(handle);
  } else
    return true;
}

int FileNode::read(unsigned char* data, int max_length) {
  bytes = 0;
  memset(data, 0, max_length);
  if (handle) {
    bytes = fread(data, 1, max_length, handle);
    length += bytes;
  }
  return bytes;
}

int FileNode::read() {
  buffer = buf_plain;
  if (read(buffer, 200)) {
    if (packed) {
      memset(buf_packed, 0, 1024);
      bytes = pletter.pack(buffer, 200, &buf_packed[0]);
      buffer = buf_packed;

      packed_length += bytes;
    }
    blocks++;
  }
  return bytes;
}

int FileNode::readAsLexeme() {
  if (read(buf_plain, 200)) {
    readAsLexeme(&buf_plain[0], bytes);
  }

  return bytes;
}

int FileNode::readAsLexeme(unsigned char* data, int data_length) {
  if (packed) {
    memset(buf_packed, 0, 1024);
    bytes = pletter.pack(data, data_length, &buf_packed[0]);

    packed_length += bytes;

    if (bytes == 0 || bytes > 255) return bytes;
  }

  current_lexeme = new Lexeme(Lexeme::type_literal, Lexeme::subtype_binary_data,
                              "FB_" + name + "_" + to_string(blocks), "");
  current_lexeme->value.resize(bytes);

  s = (unsigned char*)current_lexeme->value.data();

  if (packed) {
    memcpy(s, &buf_packed[0], bytes);
  } else {
    memcpy(s, data, data_length);
  }

  if (first) {
    first_lexeme = current_lexeme;
    memcpy(&file_header[0], data, 255);
    first = false;
  }

  blocks++;

  return bytes;
}

void FileNode::close() {
  if (handle) {
    fclose(handle);
    handle = 0;
  }
}

void FileNode::clear() {
  handle = 0;
  length = 0;
  first = true;
  packed_length = 0;
  blocks = 0;
  first_lexeme = 0;
  current_lexeme = 0;
}

bool FileNode::create() {
  clear();
  handle = fopen(name.c_str(), "wb");
  return handle;
}

void FileNode::write(unsigned char* data, int data_length) {
  fwrite(data, 1, data_length, handle);
  length += data_length;
}

//----------------------------------------------------------------------------------------------

string FileNode::stripQuotes(const string& text) {
  string s = text;

  // Remove leading quote
  if (!s.empty() && s.front() == '"') {
    s.erase(s.begin());
  }

  // Remove trailing quote
  if (!s.empty() && s.back() == '"') {
    s.pop_back();
  }

  return s;
}

/*
void FileNode::stripQuotes(string text, char* buf, int buflen) {
  int tt;
  char* s;

  /// strips quotes from text
  tt = text.size();
  s = (char*)text.c_str();
  if (s[0] == '"') {
    s++;
    tt--;
  }
  strncpy(buf, s, buflen);
  if (buf[tt - 1] == '"') {
    buf[tt - 1] = 0;
    tt--;
  }
}
  */

string FileNode::toUpper(const string& input) {
  string result = input;
  transform(result.begin(), result.end(), result.begin(),
            [](unsigned char c) { return toupper(c); });
  return result;
}

string FileNode::getFileExt(string filename) {
  return toUpper(getFileExtension(filename));
}

string FileNode::getFileExt() {
  return getFileExt(name);
}

/// @todo move to ResourceAkmReader in resources.cpp
void FileNode::fixAKM(unsigned char* data, int address, int length) {
  int t, i, k, w, current, previous, start, tracks, linker;
  int instrumentIndexTable, arpeggioIndexTable, pitchIndexTable;
  int subsongIndexTable;
  int noteBlockIndexTable, trackIndexTable;
  bool first, track_annotation;

  /// adjust header

  instrumentIndexTable = data[0] | (data[1] << 8);
  arpeggioIndexTable = data[2] | (data[3] << 8);
  pitchIndexTable = data[4] | (data[5] << 8);

  start = instrumentIndexTable & 0xFF00;
  instrumentIndexTable -= start;

  if (arpeggioIndexTable) {
    current = arpeggioIndexTable - start + address;
    data[2] = current & 0xFF;
    data[3] = (current >> 8) & 0xFF;
  }

  if (pitchIndexTable) {
    current = pitchIndexTable - start + address;
    data[4] = current & 0xFF;
    data[5] = (current >> 8) & 0xFF;
  }

  /// adjust subsongs table

  first = true;

  for (i = 6; i < instrumentIndexTable; i += 2) {
    subsongIndexTable = data[i] | (data[i + 1] << 8);
    if (subsongIndexTable) {
      subsongIndexTable -= start;

      // printf("===> subsong 0x%02X\n", subsongIndexTable);

      if (first) {
        first = false;
        previous =
            data[subsongIndexTable - 2] |
            (data[subsongIndexTable - 1] << 8);  // adjust the last instrument
        if (previous) {                          // before first subsong
          previous -= start;
          previous += address;
          if (subsongIndexTable > 1 && subsongIndexTable < length) {
            data[subsongIndexTable - 2] = previous & 0xFF;
            data[subsongIndexTable - 1] = (previous >> 8) & 0xFF;
          }
        }
      }

      noteBlockIndexTable =
          data[subsongIndexTable] | (data[subsongIndexTable + 1] << 8);
      if (noteBlockIndexTable) {
        noteBlockIndexTable -= start;
        noteBlockIndexTable += address;
        if (subsongIndexTable >= 0 && subsongIndexTable < (length - 1)) {
          data[subsongIndexTable] = noteBlockIndexTable & 0xFF;
          data[subsongIndexTable + 1] = (noteBlockIndexTable >> 8) & 0xFF;
        }
      }

      trackIndexTable =
          data[subsongIndexTable + 2] | (data[subsongIndexTable + 3] << 8);
      if (trackIndexTable) {
        trackIndexTable -= start;

        /// read subsong linker

        tracks = 0x80;
        track_annotation = false;

        /// @brief linker state machine

        for (k = subsongIndexTable + 13; k < trackIndexTable;) {
          linker = data[k++];  //! linker code

          if (linker & 1) {  //! if speed change/end song...
            k++;             //! ...skip speed change/end song
          }

          if (linker == 1) {  //! if end of song...
            current = data[k] | (data[k + 1] << 8);
            current -= start;
            current += address;
            if (k >= 0 && k < (length - 1)) {
              data[k] = current & 0xFF;
              data[k + 1] = (current >> 8) & 0xFF;
            }
            k += 2;  //! ...skip loop address
                     // printf("   end of song\n");
          } else {
            linker >>= 1;
            if (linker & 1) {  //! if line count...
              k++;             //! ...skip line count
            }
            for (w = 0; w < 3; w++) {
              linker >>= 1;
              if (linker & 1) {  //! if transposition w...
                k++;             //! ...skip transp w
              }
              linker >>= 1;
              if (linker & 1) {     //! if channel w...
                current = data[k];  //! ...check channel w
                k++;
                if (current & 0x80) {
                  if (current >= 0x80 && current <= 0x83) {
                    track_annotation = true;
                    if (current > tracks) tracks = current;
                  }
                  // printf("   track c %i: 0x%02X\n", w, current);
                } else {
                  /// @remark
                  /// "current" value needs to be calculated to keep
                  /// the state machine loop consistency
                  /// NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores)
                  current = (current << 8) | data[k];
                  k++;
                  // printf("   track c %i: 0x%04X\n", w, current);
                }
              }
            }
          }
        }

        if (track_annotation) {
          tracks = ((tracks & 3) + 1) * 2;
          // printf("   tracks: %i\n", tracks / 2);

          t = trackIndexTable + tracks;

          for (k = trackIndexTable; k < t; k += 2) {
            current =
                data[k] | (data[k + 1] << 8);  //! adjust the track address
            if (current) {
              current -= start;
              current += address;
              if (k >= 0 && k < (length - 1)) {
                data[k] = current & 0xFF;
                data[k + 1] = (current >> 8) & 0xFF;
              }
            }
          }
        }

        trackIndexTable += address;
        if (subsongIndexTable >= 0 && subsongIndexTable < (length - 3)) {
          data[subsongIndexTable + 2] = trackIndexTable & 0xFF;
          data[subsongIndexTable + 3] = (trackIndexTable >> 8) & 0xFF;
        }
      }

      subsongIndexTable += address;
      if (i >= 0 && i < (length - 1)) {
        data[i] = subsongIndexTable & 0xFF;
        data[i + 1] = (subsongIndexTable >> 8) & 0xFF;
      }
    }
  }

  /// adjust instruments table

  t = data[instrumentIndexTable] | (data[instrumentIndexTable + 1] << 8);
  t -= start;
  first = true;

  for (i = instrumentIndexTable; i < t; i += 2) {
    current = data[i] | (data[i + 1] << 8);
    if (current) {
      current -= start;

      if (first)
        first = false;
      else {
        previous = data[current - 2] |
                   (data[current - 1] << 8);  //! adjust the previous instrument
        if (previous) {
          previous -= start;
          previous += address;
          if (current > 1 && current < length) {
            data[current - 2] = previous & 0xFF;
            data[current - 1] = (previous >> 8) & 0xFF;
          }
        }
      }

      current += address;
      if (i >= 0 && i < (length - 1)) {
        data[i] = current & 0xFF;
        data[i + 1] = (current >> 8) & 0xFF;
      }
    }
  }

  instrumentIndexTable += address;
  data[0] = instrumentIndexTable & 0xFF;
  data[1] = (instrumentIndexTable >> 8) & 0xFF;

  /// @note debugging helper code
  // FileNode *file = new FileNode();
  // file->writeToFile((char *) "song.dat", data, length);
  // delete file;
}

/// @todo move to ResourceAkxReader in resources.cpp
void FileNode::fixAKX(unsigned char* data, int address, int length) {
  int t = length;
  int i = 0, current, start = 0;
  bool first = true;

  // loop the effects list

  while (i < t) {
    current = data[i] | (data[i + 1] << 8);
    if (current) {
      if (first) {
        start = current & 0xFF00;
        t = current - start;
        first = false;
      }

      current = current - start + address;

      if (i >= 0 && i < (length - 1)) {
        data[i] = current & 0xFF;
        data[i + 1] = (current >> 8) & 0xFF;
      }
    }

    i += 2;
  }

  /// @note debugging helper code
  // FileNode *file = new FileNode();
  // file->writeToFile((char *) "effect.dat", data, length);
  // delete file;
}

//----------------------------------------------------------------------------------------------

bool FileNode::writeToFile(string filename, unsigned char* data,
                           int data_length) {
  this->name = filename;

  if (!create()) {
    return false;
  }

  write(data, data_length);

  close();

  return true;
}

int FileNode::readFromFile(string filename, unsigned char* data, int maxlen) {
  FILE* file;
  int total_bytes = 0;

  if ((file = fopen(filename.c_str(), "rb"))) {
    total_bytes = fread(data, 1, maxlen, file);
    fclose(file);
  }

  length = total_bytes;

  return total_bytes;
}

int FileNode::ParseTinySpriteFile(string filename, unsigned char* data,
                                  int maxlen) {
  FILE* file;
  char line[255];
  int len, size_read, state;
  int sprite_type = 0;
  int sprite_x = 0, sprite_y = 0, sprite_c = 0;
  int sprite_k, sprite_w, sprite_n;
  unsigned char sprite_data[16][4][8];
  unsigned char sprite_attr[16 * 255];
  unsigned char sprite_color[16];
  unsigned char* sprite_count = &sprite_color[0];
  bool ok = false, found;

  *sprite_count = 0;

  /// @note structure
  // for msx1 tiny sprite format file:
  //   [type=0][sprite count][sprites patterns=collection of 32 bytes][sprites
  //   colors=1 byte per sprite]
  // for msx2 format file:
  //   [type=1][sprite count][sprites patterns=collection of 32 bytes][sprites
  //   colors=16 bytes per sprite]

  if ((file = fopen(filename.c_str(), "r"))) {
    size_read = 0;
    state = 0;
    memset(sprite_attr, 0, 64);
    ok = true;

    while (fgets(line, sizeof(line), file)) {
      len = strlen(line);

      // strip CR LF of the line
      while (len) {
        if (line[len - 1] < 0x20) {
          line[len - 1] = 0;
          len--;
        } else
          break;
      }

      if (len) {
        switch (state) {
          // file header
          case 0: {
            if (strcasecmp(line, "!type") == 0) {
              state++;
            } else {
              ok = false;
            }
          } break;

          // sprite type
          case 1: {
            if (strcasecmp(line, "msx1") == 0) {
              sprite_type = 0;  // msx 1 (screen mode <= 3)
            } else if (strcasecmp(line, "msx2") == 0) {
              sprite_type = 1;  // msx 2 (screen mode >= 4)
            } else {
              ok = false;
              break;
            }
            data[0] = sprite_type;  // MSX 1
            data[1] = 0;
            sprite_count = &data[1];
            data += 2;
            size_read += 2;
            state++;
          } break;

          // slot number
          case 2: {
            if (line[0] == '#') {
              sprite_y = 0;
              memset(sprite_data, 0, 16 * 4 * 8);
              memset(sprite_color, 0, 16);
              state++;
            } else {
              ok = false;
            }
          } break;

          // sprite data
          case 3: {
            for (sprite_x = 0; sprite_x < 16; sprite_x++) {
              sprite_c = line[sprite_x];

              if (sprite_c >= '0' && sprite_c <= '9') {
                sprite_c -= '0';
              } else if (sprite_c >= 'A' && sprite_c <= 'F') {
                sprite_c -= 'A';
                sprite_c += 10;
              } else if (sprite_c >= 'a' && sprite_c <= 'f') {
                sprite_c -= 'a';
                sprite_c += 10;
              } else {
                sprite_c = 0;
              }

              if (sprite_c) {
                if (sprite_y < 8) {
                  if (sprite_x < 8) {
                    sprite_k = 0;
                  } else {
                    sprite_k = 2;
                  }
                } else {
                  if (sprite_x < 8) {
                    sprite_k = 1;
                  } else {
                    sprite_k = 3;
                  }
                }
                sprite_w = sprite_y % 8;
                sprite_n = (1 << (7 - (sprite_x % 8)));
                sprite_data[sprite_c][sprite_k][sprite_w] |= sprite_n;
                sprite_color[sprite_c] = 1;
              }
            }

            sprite_y++;

            if (sprite_y >= 16) {
              if (sprite_type) {
                // msx 2
                do {
                  found = false;
                  for (sprite_y = 0; sprite_y < 16; sprite_y++) {
                    if (sprite_y < 8) {
                      sprite_k = 0;
                    } else {
                      sprite_k = 1;
                    }
                    sprite_w = sprite_y % 8;
                    sprite_n = sprite_k + 2;
                    sprite_data[0][sprite_k][sprite_w] = 0;
                    sprite_data[0][sprite_n][sprite_w] = 0;
                    sprite_color[sprite_y] = 0;
                    for (sprite_c = 1; sprite_c < 16; sprite_c++) {
                      if (sprite_data[sprite_c][sprite_k][sprite_w] |
                          sprite_data[sprite_c][sprite_n][sprite_w]) {
                        sprite_data[0][sprite_k][sprite_w] =
                            sprite_data[sprite_c][sprite_k][sprite_w];
                        sprite_data[0][sprite_n][sprite_w] =
                            sprite_data[sprite_c][sprite_n][sprite_w];
                        sprite_data[sprite_c][sprite_k][sprite_w] = 0;
                        sprite_data[sprite_c][sprite_n][sprite_w] = 0;
                        sprite_color[sprite_y] = sprite_c;
                        found = true;
                        break;
                      }
                    }
                  }
                  if (found) {
                    sprite_n = *sprite_count * 16;
                    memcpy(&sprite_attr[sprite_n], &sprite_color[0], 16);
                    memcpy(data, sprite_data[0], 32);
                    *sprite_count += 1;
                    data += 32;
                    size_read += 32;
                  }
                } while (found);
              } else {
                // msx 1
                for (sprite_c = 0; sprite_c < 16; sprite_c++) {
                  if (sprite_color[sprite_c]) {
                    sprite_attr[*sprite_count] = sprite_c;
                    memcpy(data, sprite_data[sprite_c], 32);
                    *sprite_count += 1;
                    data += 32;
                    size_read += 32;
                  }
                }
              }
              state = 2;
            }
          } break;
        }

        if (!ok) break;
      }
    }

    fclose(file);

    if (ok) {
      if (*sprite_count) {
        if (sprite_type) {
          // msx 2
          sprite_n = *sprite_count * 16;
          memcpy(data, sprite_attr, sprite_n);
          size_read += sprite_n;
        } else {
          // msx 1
          memcpy(data, sprite_attr, *sprite_count);
          size_read += *sprite_count;
        }
      }
      return size_read;
    } else
      return 0;

  } else {
    return -1;
  }
}
