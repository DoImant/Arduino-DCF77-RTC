//////////////////////////////////////////////////////////////////////////////
/// @file bcdconv.hpp
/// @author Kai R. 
/// @brief Declaration of free functions to convert values in BCD format to 
///        decimal format and vice versa.
/// 
/// @date 2022-05-22
/// @version 1.0
/// 
/// @copyright Copyright (c) 2022
/// 
//////////////////////////////////////////////////////////////////////////////

#ifndef _BCDCONV_HPP_ 
#define _BCDCONV_HPP_

#include <stdint.h>
namespace BCDConv {
  uint8_t bcdToDec(uint8_t);
  uint8_t decToBcd(uint8_t);
  void bcdTochar(char* const, const char);
}
#endif
