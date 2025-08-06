/***
 * @file pletter.h
 * @brief Pletter class header (v0.5c1)
 * @author XL2S Entertainment
 * @note
 *   Adapted to MSXBAS2ROM by Amaury Carvalho (2020)
 */

#ifndef PLETTER_H
#define PLETTER_H

#include <string.h>

/***
 * @class
 * @brief Pletter compress library wrapper
 */
class Pletter {
 public:
  Pletter();
  virtual ~Pletter();

  /***
   * @brief Compress data
   * @param pData Source data to be compressed
   * @param dataSize Source data size
   * @param pDest Destination data buffer
   */
  int pack(unsigned char *pData, int dataSize, unsigned char *pDest);

 protected:
 private:
  unsigned length, offset;
  bool savelength = false;

  unsigned *last = NULL;
  unsigned *prev = NULL;
  unsigned char *d = NULL;

  struct metadata {
    unsigned reeks;
    unsigned cpos[7], clen[7];
  } *m = NULL;

  struct pakdata {
    unsigned cost, mode, mlen;
  } *p[7];

  unsigned maxlen[7] = {128,        128 + 128,  512 + 128, 1024 + 128,
                        2048 + 128, 4096 + 128, 8192 + 128};
  unsigned varcost[65536];

  struct saves {
    unsigned char *buf = NULL;
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
    int done(unsigned char *pDest);
  } s;

  void initvarcost();
  void createmetadata();
  int getlen(pakdata *p, unsigned q);
  int save(pakdata *p, unsigned q, unsigned char *pDest);
  void clean();
};

#endif  // PLETTER_H
