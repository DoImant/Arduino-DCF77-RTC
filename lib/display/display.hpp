//////////////////////////////////////////////////////////////////////////////
/// @file display.h
/// @author Kai R. (you@domain.com)
/// @brief Declaration of display helper functions
///
/// @section methods DOGM Methods available
///          description       Funktionen der DOGM_7036 Klasse
///          void initialize       (byte p_cs, byte p_si, byte p_clk, byte p_rs, byte p_res, boolean sup_5V, byte
///          lines); void string           (const char *str); void ascii            (char character); void position
///          (byte column, byte line); void displ_onoff      (boolean on); void cursor_onoff     (boolean on); void
///          define_char      (byte mem_adress, byte *dat); void clear_display    (void); void contrast         (byte
///          contr);
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
#include <digitalWriteFast.h>
#include <Button_SL.hpp>
#include "dogm_7036.h"

//////////////////////////////////////////////////
// Global constants and variables
//////////////////////////////////////////////////
constexpr uint8_t PIN_RS{7};          // RS = Read/Write Data from/to RAM (Pin 39)
constexpr uint8_t PIN_RST{8};         // Reset des Displays (Pin 40)
constexpr uint8_t PIN_BACKLIGHT{9};   // Pin (D9) for backlight brightness control
constexpr uint8_t BUTTON_BL_PIN{4};   // Pin (D4) for switching the backlight on
constexpr uint8_t BUTTON_DT_PIN{5};   // Pin (D5) for switching the date view on the Display

// PWM duty cycles for brightness: 0 = off, 255 = max. brightness
constexpr uint8_t BL_BRIGHTNESS_OFF{0};
constexpr uint8_t BL_BRIGHTNESS_ON{16};
constexpr uint8_t BL_BURN_DURATION{10};   // Time in sec

constexpr uint8_t SHOW_DATE_DURATION{10};   // Time in sec
constexpr uint8_t MINUTE{60};
constexpr uint8_t MINUTE_IMPOSSIBLE{61};

//////////////////////////////////////////////////
// Class definitions
//////////////////////////////////////////////////
enum class Separators : uint8_t { SPACE, TIME, DATE, COLUP, COLDOWN };

//////////////////////////////////////////////////////////////////////////////
/// @brief Class for handling separators in time-
///        and date display (HH:MM:SS / 01-01-22)
///
//////////////////////////////////////////////////////////////////////////////
class ClockSeparators {
private:
  Separators dateSep{Separators::DATE};
  Separators timeSep[2]{Separators::SPACE, Separators::COLDOWN};
  const uint8_t separator[5]{' ', ':', '-', 0x01, 0x02};   // 0x01 and 0x02 are self defined chars set in initDisplay

public:
  ClockSeparators() {}
  ClockSeparators(const ClockSeparators &) = delete;              // prevent copy
  ClockSeparators &operator=(const ClockSeparators &) = delete;   // prevent assignment

  template <size_t size, class T> inline size_t arraySize(T (&arr)[size]) const { return size; }

  void setTimeSeparator(Separators, uint8_t);
  void setDateSeparator(Separators);
  Separators getTimeSeparator(uint8_t idx) const;
  uint8_t getSeparatorChar(Separators sep) const;
};

//////////////////////////////////////////////////////////////////////////////
/// @brief Class to handle the time and date strings
///
//////////////////////////////////////////////////////////////////////////////
class ClockData {
private:
  ClockSeparators separator;
  char _strTimeBuff[9]{0};
  char _strDateBuff[9]{0};

public:
  ClockData() {}
  ClockData(const ClockData &) = delete;              // prevent copy
  ClockData &operator=(const ClockData &) = delete;   // prevent assignment
  void setTime(void);
  void setDate(void);
  const char *getTime() const;
  const char *getDate() const;
  ClockSeparators &clockSeparator();
};

//////////////////////////////////////////////////
// Function forward declaration
//////////////////////////////////////////////////
void initDisplay(dogm_7036 &);
void monoBacklight(byte);
void printRtcTime(dogm_7036 &, ClockData &, bool);
void switchBacklight(uint8_t, Btn::ButtonState);

#endif