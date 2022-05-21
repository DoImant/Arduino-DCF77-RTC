//////////////////////////////////////////////////////////////////////////////
/// @file bcdconv.hpp
/// @author Kai R. 
/// @brief Class with methods to convert BCD to Decimal and vice versa.
/// 
/// @date 2022-05-20
/// @version 1.0
/// 
/// @copyright Copyright (c) 2022
/// 
//////////////////////////////////////////////////////////////////////////////

#ifndef _BCDCONV_HPP_ 
#define _BCDCONV_HPP_

#include <stdint.h>

class BCDConv {
public:
  BCDConv(void) {};
  static inline uint8_t bcdToDec(uint8_t);
  static inline uint8_t decToBcd(uint8_t);
  static inline void bcdTochar(char* const, const char);
};

//////////////////////////////////////////////////////////////////////////////
/// @brief Converts a bcd value to a decimal value
/// 
/// @param bcd
/// @return uint8_t
//////////////////////////////////////////////////////////////////////////////
inline uint8_t BCDConv::bcdToDec(uint8_t bcd) {
  return bcd - 6 * (bcd >> 4);
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Converts an decimal value to a bcd value
/// 
/// @param dec 
/// @return uint8_t 
//////////////////////////////////////////////////////////////////////////////
inline uint8_t BCDConv::decToBcd(uint8_t dec) {
  return (dec + 6 * (dec / 10)); 
}

//////////////////////////////////////////////////////////////////////////////
/// @brief copies an 8-bit BDC number to a string of two ASCII characters. 
///        No string end character is written!
/// 
/// @param str 
/// @param bcdVal   
//////////////////////////////////////////////////////////////////////////////
inline void BCDConv::bcdTochar(char* const str, const char bcdVal) {
  *(str)   = (bcdVal >> 4)   + 0x30;
  *(str+1) = (bcdVal & 0x0F) + 0x30;
}
#endif
