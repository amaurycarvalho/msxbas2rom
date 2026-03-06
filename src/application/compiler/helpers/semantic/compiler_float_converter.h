/***
 * @file compiler_float_converter.h
 * @brief Compiler float converter
 */

#ifndef COMPILER_FLOAT_CONVERTER_H
#define COMPILER_FLOAT_CONVERTER_H

#include <string>

class CompilerContext;

using namespace std;

class CompilerFloatConverter {
 private:
  CompilerContext* context;

 public:
  /***
   * @brief Convert a double to MSX float point math pack library format
   * @param value Value to convert
   * @param words 32 bits destination buffer
   * @note
   * [Basic Kun Math
   * Pack](https://www.msx.org/wiki/Category:X-BASIC#Floating_points)
   */
  void double2FloatLib(double value, int* words);

  /***
   * @brief Convert a float to MSX float point math pack library format
   * @param value Value to convert
   * @param words 16 bits destination buffer
   * @note
   * [Basic Kun Math
   * Pack](https://www.msx.org/wiki/Category:X-BASIC#Floating_points)
   */
  void float2FloatLib(float value, int* words);

  /***
   * @brief Convert a string to MSX float point math pack library format
   * @param value String to convert
   * @return 16 bits float value
   * @note
   * [Basic Kun Math
   * Pack](https://www.msx.org/wiki/Category:X-BASIC#Floating_points)
   */
  int str2FloatLib(string value);

  /***
   * @brief Convert a string to MSX PRINT USING format flags
   * @param value String to convert
   * @return 16 bits flags value
   * @example flags = getUsingFormat("###,##0.00");
   * @note
   * [PRINT USING](https://www.msx.org/wiki/PRINT#Parameters)
   */
  int getUsingFormat(string text);

  CompilerFloatConverter(CompilerContext* context) : context(context) {};
};

#endif  // COMPILER_FLOAT_CONVERTER_H