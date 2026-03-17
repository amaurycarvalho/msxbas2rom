/***
 * @file compiler_float_converter.cpp
 * @brief Compiler float converter
 * @author Amaury Carvalho (2019-2026)
 */

#include "compiler_float_converter.h"

#include "compiler_context.h"
#include "logger.h"

void CompilerFloatConverter::double2FloatLib(double value, int* words) {
  float value2 = value;
  float2FloatLib(value2, words);
}

void CompilerFloatConverter::float2FloatLib(float value, int* words) {
  unsigned short* pi = (unsigned short*)&value;
  unsigned char* pc = (unsigned char*)&value;
  unsigned char sign = 0;

  if (value == 0.0) {
    words[0] = 0;
    words[1] = 0;
    words[2] = 0;
    words[3] = 0;
    return;
  }

  if (pc[3] & 0x80) sign = 0x80;
  pc[3] <<= 1;

  if (pc[2] & 0x80) pc[3] |= 1;

  pc[2] &= 0x7F;
  pc[2] |= sign;

  pc[3]++;

  words[0] = pi[0];
  words[1] = pi[1];
  words[2] = 0;
  words[3] = 0;
}

int CompilerFloatConverter::str2FloatLib(string text) {
  int words[4];
  float value;
  try {
    value = stof(text);
  } catch (exception& e) {
    context->logger->warning("Error while converting numeric constant " + text);
    value = 0;
  }
  float2FloatLib(value, (int*)&words);
  return ((words[1] + 0x0100) << 8) | (words[0] >> 8 & 0xff);
}

int CompilerFloatConverter::getUsingFormat(string text) {
  int c = 0x80;         // format style (7=1 6=, 5=* 4=$ 3=+ 2=- 1=0  0=^)
  int d = 0, e = 0;     // d=thousand digits, e=decimal digits
  int b = text.size();  // format string size
  int i;
  char* s = (char*)text.c_str();

  for (i = 0; i < b; i++) {
    switch (s[i]) {
      // 0, set zeros formating
      case '0': {
        c |= 1 << 1;  // set bit 1
        if (e)
          e++;
        else
          d++;
      } break;

      // # numeric format char, handle
      case '#': {
        if (e)
          e++;
        else
          d++;
      } break;

      // +, set sign flag and continue
      case '+': {
        c |= 1 << 3;  // set bit 3
        if (d)
          c |= 1 << 2;  // set bit 2
        else {
          if (e)
            e++;
          else
            d++;
        }
      } break;

      // -, set sign after number
      case '-': {
        c |= 1 << 2;  // set bit 2
      } break;

      // decimal point
      case '.': {
        e++;
      } break;

      // mark symbol
      case '*': {
        c |= 1 << 5;  // set bit 5
        if (e)
          e++;
        else
          d++;
      } break;

      // currency symbol
      case '$': {
        c |= 1 << 4;  // set bit 4
        if (e)
          e++;
        else
          d++;
      } break;

      // thousand separator
      case ',': {
        c |= 1 << 6;  // set bit 6
        if (e)
          e++;
        else
          d++;
      } break;

      // exponential representation
      case '^': {
        c |= 1;  // set bit 0
        if (e)
          e++;
        else
          d++;
      } break;
    }
  }

  return ((e & 0xF) | ((d & 0xF) << 4) | (c << 8));
}

CompilerFloatConverter::CompilerFloatConverter(CompilerContext* context)
    : context(context) {}

CompilerFloatConverter::~CompilerFloatConverter() = default;
