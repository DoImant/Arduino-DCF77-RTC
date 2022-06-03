//////////////////////////////////////////////////////////////////////////////
/// @file display.cpp
/// @author Kai R. (you@domain.com)
/// @brief Helper functions for using the DOGM display.
/// 
/// @date 2022-05-01
/// @version 1.0
/// 
/// @date 2022-06-03
/// Refactornig, added animated dots to the time display.
/// File suffix changed from .h to .hpp.
///
/// @copyright Copyright (c) 2022
/// 
//////////////////////////////////////////////////////////////////////////////

#include "display.hpp"
#include "bcdconv.hpp"
#include "dogm_7036.h"
#include "DS3231Wire.h"
#include "digitalWriteFast.h"

// Methods of ClockData //////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
/// @brief Sets the separator for the time display.
/// 
/// @param actSep 
//////////////////////////////////////////////////////////////////////////////
void ClockData::setTimeSeparator(Separators actSep) {
	_actTimeSep = actSep;
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Sets the separator for the Date display.
/// 
/// @param actSep 
//////////////////////////////////////////////////////////////////////////////
void ClockData::setDateSeparator(Separators actSep) {
	_actDateSep = actSep;
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Reads the time from the RTC and copies the data into a 
///        string for the time display.
/// 
//////////////////////////////////////////////////////////////////////////////
void ClockData::setTime() {
	//Looks complicated, but it saves many flash space (-1.5Kb) compared to sprintf.
  BCDConv::bcdTochar(_strTimeBuff,readRegister(DS3231_HOURS));
  *(_strTimeBuff+2) = _separator[static_cast<uint8_t>(Separators::SEP_TIME)];
  BCDConv::bcdTochar((_strTimeBuff+3),readRegister(DS3231_MINUTES));
  *(_strTimeBuff+5) = _separator[static_cast<uint8_t>(_actTimeSep)];
  BCDConv::bcdTochar((_strTimeBuff+6),readRegister(DS3231_SECONDS));
  *(_strTimeBuff+8) = '\0';
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Reads the date from the RTC and copies the data into a 
///        string for the date display.
/// 
//////////////////////////////////////////////////////////////////////////////
void ClockData::setDate() {
	 BCDConv::bcdTochar(_strDateBuff,readRegister(DS3231_DATE));
  *(_strDateBuff+2) = _separator[static_cast<uint8_t>(Separators::SEP_DATE)];
  BCDConv::bcdTochar((_strDateBuff+3),readRegister(DS3231_CEN_MONTH));
  *(_strDateBuff+5) = _separator[static_cast<uint8_t>(Separators::SEP_DATE)];
  // *(strDateBuff+6) = '2';                         //change it 2099 :-)
  // *(strDateBuff+7) = '0';
  //bcdTochar((strDateBuff+8),readRegister(DS3231_YEAR));
  BCDConv::bcdTochar((_strDateBuff+6),readRegister(DS3231_YEAR));
  *(_strDateBuff+10) = '\0';
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Returns the address of the string for the time display.
/// 
/// @return const char*   Addresss of the String for the time display
//////////////////////////////////////////////////////////////////////////////
const char* ClockData::getTime() const {
  return _strTimeBuff;
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Returns the address of the string for the date display.
/// 
/// @return const char*   Address of the String for the date display
//////////////////////////////////////////////////////////////////////////////
const char* ClockData::getDate() const {
  return _strDateBuff;
}
// ClockData End /////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
/// @brief Initialize the DOGM display
/// 
/// @param disp 
//////////////////////////////////////////////////////////////////////////////
void initDisplay(dogm_7036 &disp) {     
  uint8_t halfColonUp[8]   = {0x00, 0x0C, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00};
  uint8_t halfColonDown[8] = {0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00, 0x00};
 
  disp.initialize(SS,0,0,PIN_RS,PIN_RST,0,DOGM081);   // SS = 10, 0,0= use Hardware SPI, 7 = RS, 8 = RESET, 0 = 3.3V, EA DOGM081-A (=1 line)
  //disp.initialize(SS,0,0,PIN_RS,PIN_RST,0,DOGM162); // SS = 10, 0,0= use Hardware SPI, 7 = RS, 8 = RESET, 0 = 3.3V, EA DOGM081-A (=1 line)
  disp.displ_onoff(true);                  // turn Display on
  disp.cursor_onoff(false);                // turn Curosor blinking off
  disp.define_char(0x01, halfColonUp);        //define own char on memory adress 1
  disp.define_char(0x02, halfColonDown);      //define own char on memory adress 2
  pinModeFast(PIN_BACKLIGHT,  OUTPUT);
  monoBacklight(BL_BRIGHTNESS_OFF);        // use monochrome backlight in this sample code. Please change it to your configuration
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Controlling the backlight brightness.
/// 
/// @param brightness 
//////////////////////////////////////////////////////////////////////////////
void monoBacklight(byte brightness)
{
  analogWrite(PIN_BACKLIGHT, brightness);  
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Check whether the backlight should be switched on.
///        When turned on, it glows for a predetermined period of time. 
///        It is then switched off again as long as the button is no longer pressed.
/// 
/// @param second Second of RT-Clock at which the button was pressed
//////////////////////////////////////////////////////////////////////////////
void switchBacklight(uint8_t second, uint8_t blButtonPressed ) {     
  static bool backlightOn = false;
  static uint8_t lightOffTime;

  if (blButtonPressed && !backlightOn) {
    backlightOn = true;
    monoBacklight(BL_BRIGHTNESS_ON);
    lightOffTime = (second + BL_BURN_DURATION + 1) % 60;    // Mod 60 Seconds
  } else if (second == lightOffTime && backlightOn) {
    backlightOn = false;
    monoBacklight(BL_BRIGHTNESS_OFF);
  }
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Output of time and date on the serial console
/// 
/// @param rtcTime 
//////////////////////////////////////////////////////////////////////////////
void printRtcTime(dogm_7036 &disp, Separators& tSep, bool dateVisible) {
  ClockData cd;
	static bool switchSep=true;

  if (dateVisible) {
		cd.setDate();
    disp.position(1,1);
    disp.string(cd.getDate());
  } else {
    cd.setTimeSeparator((switchSep) ? tSep : Separators::SEP_COLDOWN);
    switchSep = !switchSep;
		cd.setTime();
    disp.position(1,1);
    disp.string(cd.getTime()); 
  }

#ifdef PRINT_TIME_SERIAL
  Serial.print("Time is ");
  Serial.print(cd.getDate());
  Serial.print(" ");
  Serial.print(cd.getTime());
#endif
}
