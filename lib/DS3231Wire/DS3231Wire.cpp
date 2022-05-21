//////////////////////////////////////////////////////////////////////////////
/// @file DS3231Wire.cpp
/// @author Kai R. (you@domain.com)
/// @brief Communication with the DS3231 RT clock
/// 
/// @date 2022-05-01
/// @version 1.0
/// 
/// @copyright Copyright (c) 2022
/// 
//////////////////////////////////////////////////////////////////////////////

#include "DS3231Wire.h"
//////////////////////////////////////////////////////////////////////////////
/// @brief enable the 1Hz square wave signal of the RTC
/// 
//////////////////////////////////////////////////////////////////////////////
void enableSw1Hz(void) {
  uint8_t data = readRegister(DS3231_CONTROL);
  data &= 0xF8;     // set to 1Hz
  data |= 0x40;     // enable square wave
  writeRegister(DS3231_CONTROL, data);
}

//////////////////////////////////////////////////////////////////////////////
/// @brief disable the square wave signal of the RTC
/// 
//////////////////////////////////////////////////////////////////////////////
void disableSw(void) {
  uint8_t data = readRegister(DS3231_CONTROL);
  data &= ~(0x44);  // disable square wave
  writeRegister(DS3231_CONTROL, data);
}

//////////////////////////////////////////////////////////////////////////////
/// @brief By default the 32kHz Signal of the DS3231 RTC is enabled at start.
///        With this function you can switch the signal off.
/// 
//////////////////////////////////////////////////////////////////////////////
void disable32kHz() {
  uint8_t data = readRegister(DS3231_CTL_STATUS);
  data &= ~(0x08);
  writeRegister(DS3231_CTL_STATUS, data);
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Reads the content of an RTC register via I2C
/// 
/// @param reg Registeraddress
/// @return uint8_t reads the received byte from the buffer and returns it to whoever called this function
//////////////////////////////////////////////////////////////////////////////
uint8_t readRegister(uint8_t reg){
	Wire.beginTransmission(DS3231_ADDR);	
	Wire.write(reg);						          
	Wire.endTransmission();					      
	Wire.requestFrom(DS3231_ADDR,1);		  
  return Wire.read();						        
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Writes a value to an RTC register via I2C
/// 
/// @param reg Registeraddress
/// @param data Value
//////////////////////////////////////////////////////////////////////////////
void writeRegister(uint8_t reg, uint8_t data) {
	Wire.beginTransmission(DS3231_ADDR);	//Sends start bit, slave address, and write bit, waits for ack from device
	Wire.write(reg);						          //Writes the first passed parameter value to the device, hopefully a register address
	Wire.write(data);						          //Writes the second passed parameter, the data for that register
	Wire.endTransmission();					      //Completes the transaction by sending stop bit
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Set the time
/// 
/// @param bcdHours 
/// @param bcdMinutes 
/// @param bcdSeconds 
//////////////////////////////////////////////////////////////////////////////
void setTime(uint8_t bcdHours, uint8_t bcdMinutes, uint8_t bcdSeconds) {
  writeRegister(DS3231_SECONDS, bcdSeconds);            // Seconds MUST written at first!
  writeRegister(DS3231_HOURS, bcdHours);
  writeRegister(DS3231_MINUTES, bcdMinutes);
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Set the date
/// 
/// @param bcdYear 
/// @param bcdMonth 
/// @param bcdDayofMonth 
//////////////////////////////////////////////////////////////////////////////
void setDate(uint8_t bcdYear, uint8_t bcdMonth, uint8_t bcdDayofMonth) {
  writeRegister(DS3231_DATE, bcdDayofMonth);
  writeRegister(DS3231_CEN_MONTH, bcdMonth);
  writeRegister(DS3231_YEAR, bcdYear);
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Set the date and time
/// 
/// @param bcdYear 
/// @param bcdMonth 
/// @param bcdDayofMonth 
/// @param bcdHours 
/// @param bcdMinutes 
/// @param bcdSeconds 
//////////////////////////////////////////////////////////////////////////////
void setDateTime(uint8_t bcdYear, uint8_t bcdMonth, uint8_t bcdDayofMonth, uint8_t bcdHours, uint8_t bcdMinutes, uint8_t bcdSeconds) {
  writeRegister(DS3231_SECONDS, bcdSeconds);            // Seconds MUST written at first!
  writeRegister(DS3231_MINUTES, bcdMinutes);
  writeRegister(DS3231_HOURS, bcdHours);
  writeRegister(DS3231_DATE, bcdDayofMonth);
  writeRegister(DS3231_CEN_MONTH, bcdMonth);
  writeRegister(DS3231_YEAR, bcdYear);
}

