//////////////////////////////////////////////////////////////////////////////
/// @file display.h
/// @author Kai R. (you@domain.com)
/// @brief Declaration of display helper functions 
/// 
/// @section methods DOGM Methods available
///          description       Funktionen der DOGM_7036 Klasse
///          void initialize       (byte p_cs, byte p_si, byte p_clk, byte p_rs, byte p_res, boolean sup_5V, byte lines);
///          void string           (const char *str);
///          void ascii            (char character);
///          void position         (byte column, byte line);
///          void displ_onoff      (boolean on);
///          void cursor_onoff     (boolean on);
///          void define_char      (byte mem_adress, byte *dat);
///          void clear_display    (void);
///          void contrast         (byte contr);
///
/// @section pins Pins used for display control
///          SPI implementation: hardware
///          Pin D13: SCK/SCLK = CLK (Pin 29)
///          Pin D10: ChipSelect (CS/SS) = CSB (Pin 38)
///          Pin D11: MOSI = SI (Pin 28)
///          Pin D12: MISO - Nicht verwendet
///
///          Pin 07: RS = Read/Write Data from/to RAM (Pin 39)
///          Pin 08: RST = Reset des Displays (Pin 40)
///          Pin 09: Brightness
///
///          Pin 04: Button for switching the backlight  4
///          Pin 05: Button to switch on the date        5
/// 
/// @date 2022-05-01
/// @version 1.0
///
/// @date 2022-06-03
/// Refactornig, added animated dots to the time display.
/// File suffix changed from .h to .hpp.
/// The behavior of the backlight button has been changed. Two switching modes are now possible.
/// 
/// @copyright Copyright (c) 2022
/// 
//////////////////////////////////////////////////////////////////////////////

#ifndef _DISPLAY_HPP_
#define _DISPLAY_HPP_

#include <stdint.h>
#include <SPI.h>
#include <dogm_7036.h>
#include "button.hpp"

//////////////////////////////////////////////////
// Global constants and variables
//////////////////////////////////////////////////
constexpr uint8_t PIN_RS = 7;                           // RS = Read/Write Data from/to RAM (Pin 39)
constexpr uint8_t PIN_RST = 8;                          // Reset des Displays (Pin 40)
constexpr uint8_t PIN_BACKLIGHT = 9;                    // Pin (D9) for backlight brightness control
constexpr uint8_t BUTTON_BL_PIN = 4;                    // Pin (D4) for switching the backlight on
constexpr uint8_t BUTTON_DT_PIN = 5;                    // Pin (D5) for switching the date view on the Display

// PWM duty cycles for brightness: 0 = off, 255 = max. brightness 
constexpr uint8_t BL_BRIGHTNESS_OFF = 0;    
constexpr uint8_t BL_BRIGHTNESS_ON = 16;    
constexpr uint8_t BL_BURN_DURATION = 10;                // Time in sec

constexpr uint8_t SHOW_DATE_DURATION = 10;              // Time in sec 
constexpr uint8_t MINUTE             = 60;              
constexpr uint8_t MINUTE_IMPOSSIBLE  = 61;              

//////////////////////////////////////////////////
// Class definitions
//////////////////////////////////////////////////
enum class Separators {SEP_SPACE, SEP_TIME, SEP_DATE, SEP_COLUP, SEP_COLDOWN};

class ClockData {
private:
  char _strTimeBuff[9];
  char _strDateBuff[9];
	unsigned char _separator[5] = {' ',':','-',0x01,0x02}; 
  Separators _actTimeSep;
  Separators _actDateSep;	
	
public:
  ClockData(Separators actTimeSep = Separators::SEP_TIME, Separators actDateSep = Separators::SEP_DATE)
    : _actTimeSep(actTimeSep), _actDateSep(actDateSep) {}
	
  void setTimeSeparator(Separators);
  void setDateSeparator(Separators);
  void setTime(void);
  void setDate(void);
  const char* getTime() const; 
  const char* getDate() const;
};

//////////////////////////////////////////////////
// Function forward declaration
//////////////////////////////////////////////////
void initDisplay(dogm_7036& disp);
void monoBacklight(byte brightness);
void printRtcTime(dogm_7036& disp, Separators& tSep, bool dateVisible);
void switchBacklight(uint8_t second, ButtonState blButtonPressed);

#endif