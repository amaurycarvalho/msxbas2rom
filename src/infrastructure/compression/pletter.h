/***
 * @file pletter.h
 * @brief Pletter class header (v0.5c1)
 * @author XL2S Entertainment
 * @note
 *   Adapted to MSXBAS2ROM by Amaury Carvalho (2020)
 */

#ifndef PLETTER_H
#define PLETTER_H

#include <array>
#include <vector>

/***
 * @class
 * @brief Pletter compress library wrapper
 */
class Pletter {
 public:
  Pletter();
  ~Pletter();

  /***
   * @brief Worst-case compressed size for an input of n bytes
   * @param n Source data size
   * @return Maximum number of compressed bytes emitted
   */
  static constexpr int maxPackedSize(int n) { return n + (n + 43) / 8; }

  /***
   * @brief Compress data
   * @param pData Source data to be compressed
   * @param dataSize Source data size
   * @param out Owned destination buffer for the exact compressed payload
   * @return Number of compressed bytes, or 0 for invalid input
   */
  int pack(const unsigned char* pData, int dataSize,
           std::vector<unsigned char>& out);

 protected:
 private:
  unsigned length, offset;
  bool savelength = false;

  std::vector<unsigned> last;
  std::vector<unsigned> prev;
  unsigned char* d = nullptr;

  struct metadata {
    unsigned reeks;
    unsigned cpos[7], clen[7];
  };
  std::vector<metadata> m;

  struct pakdata {
    unsigned cost, mode, mlen;
  };
  std::array<std::vector<pakdata>, 7> p;

  unsigned maxlen[7] = {128,        128 + 128,  512 + 128, 1024 + 128,
                        2048 + 128, 4096 + 128, 8192 + 128};
  std::array<unsigned, 65536> varcost;

  struct saves {
    std::vector<unsigned char> buf;
    int ep, p, e;
    void init(unsigned length);
    void add0();
    void add1();
    void addbit(int b);
    void add3(int b);
    void addvar(int i);
    void adddata(unsigned char d);
    void addevent();
    void claimevent();
    int done(std::vector<unsigned char>& out);
  } s;

  void initvarcost();
  void createmetadata();
  int getlen(pakdata* p, unsigned q);
  int save(pakdata* p, unsigned q, std::vector<unsigned char>& out);
};

#endif  // PLETTER_H
