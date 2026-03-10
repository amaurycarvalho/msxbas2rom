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
#include <cstring>
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
   * @brief Compress data
   * @param pData Source data to be compressed
   * @param dataSize Source data size
   * @param pDest Destination data buffer
   */
  int pack(const unsigned char* pData, int dataSize, unsigned char* pDest);

 protected:
 private:
  unsigned length, offset;
  bool savelength = false;

  std::vector<unsigned> last;
  std::vector<unsigned> prev;
  unsigned char* d = NULL;

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
    int ep, dp, p, e;
    void init(unsigned length);
    void add0();
    void add1();
    void addbit(int b);
    void add3(int b);
    void addvar(int i);
    void adddata(unsigned char d);
    void addevent();
    void claimevent();
    int done(unsigned char* pDest);
  } s;

  void initvarcost();
  void createmetadata();
  int getlen(pakdata* p, unsigned q);
  int save(pakdata* p, unsigned q, unsigned char* pDest);
};

#endif  // PLETTER_H
