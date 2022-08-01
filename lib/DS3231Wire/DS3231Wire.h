//////////////////////////////////////////////////////////////////////////////
/// @file DS3231Wire.h
/// @author Kai R. (you@domain.com)
/// @brief Communication with the DS3231 RT clock
/// 
/// @date 2022-05-01
/// @version 1.0
/// 
/// @copyright Copyright (c) 2022
/// 
//////////////////////////////////////////////////////////////////////////////

#ifndef _DS3231_WIRE_H
#define _DS3231_WIRE_H

#include <Wire.h>

namespace DS3231 {
  //DS3231 I2C Addresses - hardwired in IC
  constexpr uint8_t ONE_BYTE  {1};
  constexpr uint8_t ADDR      {0x68};
  constexpr uint8_t WRITE     {(ADDR << 1)};
  constexpr uint8_t READ      {WRITE + 1};

  //DS3231 Registers
  constexpr uint8_t SECONDS         {0x00};            // 00-59
  constexpr uint8_t MINUTES         {0x01};            // 00-59
  constexpr uint8_t HOURS           {0x02};            // 00-23 or 0-12 + AM/PM
  constexpr uint8_t DAY             {0x03};            // Day of Week  1-7
  constexpr uint8_t DATE            {0x04};            // Day of Month 1 - 31
  constexpr uint8_t CEN_MONTH       {0x05};            // toggles if Year 99 switches to 00 -> 1-12 + Century
  constexpr uint8_t YEAR            {0x06};            // Year 00 - 99
  constexpr uint8_t CONTROL         {0x0e};
  constexpr uint8_t CTL_STATUS      {0x0f};

  /* uncomment if you want to use...
  constexpr uint8_t ALARM1_SECONDS  {0x07};
  constexpr uint8_t ALARM1_MINUTES  {0x08};
  constexpr uint8_t ALARM1_HOURS    {0x09};
  constexpr uint8_t ALARM1_DAY_DATE {0x0a};
  constexpr uint8_t ALARM2_MINUTES  {0x0b};
  constexpr uint8_t ALARM2_HOURS    {0x0c};
  constexpr uint8_t ALARM2_DAY_DATE {0x0d};
  constexpr uint8_t AGING_OFFSET    {0x10};
  constexpr uint8_t TEMP_MSB        {0x11};
  constexpr uint8_t TEMP_LSB        {0x12};
  */ 

  void enableSw1Hz(void);
  void disableSw(void);
  void disable32kHz(void); 
  uint8_t readRegister(uint8_t reg);
  void writeRegister(uint8_t reg, uint8_t data);
  void setTime(uint8_t bcdHours, uint8_t bcdMinutes, uint8_t bcdSeconds);
  void setDate(uint8_t bcdYear, uint8_t bcdMonth, uint8_t bcdDayofMonth);
  void setDateTime(uint8_t bcdYear, uint8_t bcdMonth, uint8_t bcdDayofMonth, uint8_t bcdHours, uint8_t bcdMinutes, uint8_t bcdSeconds);
}
#endif