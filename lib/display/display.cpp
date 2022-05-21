//////////////////////////////////////////////////////////////////////////////
/// @file display.cpp
/// @author Kai R. (you@domain.com)
/// @brief Helper functions for using the DOGM display.
/// 
/// @date 2022-05-01
/// @version 1.0
/// 
/// @copyright Copyright (c) 2022
/// 
//////////////////////////////////////////////////////////////////////////////

#include "display.h"
#include "bcdconv.hpp"
#include "dogm_7036.h"
#include "DS3231Wire.h"
#include "digitalWriteFast.h"

//////////////////////////////////////////////////////////////////////////////
/// @brief Initialisiere das DOGM Display
/// 
/// @param disp 
//////////////////////////////////////////////////////////////////////////////
void initDisplay(dogm_7036 &disp) {                 
  disp.initialize(SS,0,0,PIN_RS,PIN_RST,0,DOGM081);   // SS = 10, 0,0= use Hardware SPI, 7 = RS, 8 = RESET, 0 = 3.3V, EA DOGM081-A (=1 line)
  //disp.initialize(SS,0,0,PIN_RS,PIN_RST,0,DOGM162); // SS = 10, 0,0= use Hardware SPI, 7 = RS, 8 = RESET, 0 = 3.3V, EA DOGM081-A (=1 line)
  disp.displ_onoff(true);                  // turn Display on
  disp.cursor_onoff(false);                // turn Curosor blinking off
  pinModeFast(PIN_BACKLIGHT,  OUTPUT);
  monoBacklight(BL_BRIGHTNESS_OFF);             // use monochrome backlight in this sample code. Please change it to your configuration
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
void printRtcTime(dogm_7036 &disp, uint8_t tSep, bool dateVisible) {
  static char strTimeBuff[9];
  static char strDateBuff[11];

  //Looks complicated, but it saves many flash space (-1.5Kb) compared to sprintf.
  BCDConv::bcdTochar(strTimeBuff,readRegister(DS3231_HOURS));
  *(strTimeBuff+2) = SEPARATOR_TIME[SEP_NORMAL];
  BCDConv::bcdTochar((strTimeBuff+3),readRegister(DS3231_MINUTES));
  *(strTimeBuff+5) = SEPARATOR_TIME[tSep];
  BCDConv::bcdTochar((strTimeBuff+6),readRegister(DS3231_SECONDS));
  *(strTimeBuff+8) = '\0';

  BCDConv::bcdTochar(strDateBuff,readRegister(DS3231_DATE));
  *(strDateBuff+2) = SEPARATOR_DATE;
  BCDConv::bcdTochar((strDateBuff+3),readRegister(DS3231_CEN_MONTH));
  *(strDateBuff+5) = SEPARATOR_DATE;
  //*(strDateBuff+6) = '2';                         //change it 2099 :-)
  //*(strDateBuff+7) = '0';
  //bcdTochar((strDateBuff+8),readRegister(DS3231_YEAR));
  BCDConv::bcdTochar((strDateBuff+6),readRegister(DS3231_YEAR));
  *(strDateBuff+10) = '\0';
  
  if (dateVisible) {
    disp.position(1,1);
    disp.string(strDateBuff);
  } else {
    disp.position(1,1);
    disp.string(strTimeBuff);
  }

#ifdef PRINT_TIME_SERIAL
  Serial.print("Time is ");
  Serial.print(strDateBuff);
  Serial.print(" ");
  Serial.print(strTimeBuff);
#endif
}