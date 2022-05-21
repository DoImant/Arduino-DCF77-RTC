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

//DS3231 I2C Addresses - hardwired in IC
#define DS3231_ADDR  0x68
#define DS3231_WRITE (DS3231_ADDR << 1)
#define DS3231_READ (DS3231_WRITE + 1)

//DS3231 Registers
#define DS3231_SECONDS  0x00           // 00-59
#define DS3231_MINUTES  0x01           // 00-59
#define DS3231_HOURS  0x02             // 00-23 or 0-12 + AM/PM
#define DS3231_DAY 0x03                // Day of Week  1-7
#define DS3231_DATE 0x04               // Day of Month 1 - 31
#define DS3231_CEN_MONTH 0x05          // toggles if Year 99 switches to 00 -> 1-12 + Century
#define DS3231_YEAR 0x06               // Year 00 - 99
#define DS3231_CONTROL 0x0e
#define DS3231_CTL_STATUS 0x0f

/* uncomment if you want to use...
#define DS3231_ALARM1_SECONDS 0x07
#define DS3231_ALARM1_MINUTES 0x08
#define DS3231_ALARM1_HOURS 0x09
#define DS3231_ALARM1_DAY_DATE 0x0a
#define DS3231_ALARM2_MINUTES 0x0b
#define DS3231_ALARM2_HOURS 0x0c
#define DS3231_ALARM2_DAY_DATE 0x0d
#define DS3231_AGING_OFFSET 0x10
#define DS3231_TEMP_MSB 0x11
#define DS3231_TEMP_LSB 0x12
*/ 

void enableSw1Hz(void);
void disableSw(void);
void disable32kHz(void); 
uint8_t readRegister(uint8_t reg);
void writeRegister(uint8_t reg, uint8_t data);
void setTime(uint8_t bcdHours, uint8_t bcdMinutes, uint8_t bcdSeconds);
void setDate(uint8_t bcdYear, uint8_t bcdMonth, uint8_t bcdDayofMonth);
void setDateTime(uint8_t bcdYear, uint8_t bcdMonth, uint8_t bcdDayofMonth, uint8_t bcdHours, uint8_t bcdMinutes, uint8_t bcdSeconds);
#endif