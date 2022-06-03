//////////////////////////////////////////////////////////////////////////////
/// @file dcf77.hpp
/// @author Kai R.
/// @brief Declaration of classes for handling received DCF77 signals
/// 
/// @date 2022-05-20
/// @version 1.0
/// 
/// @date 2022-06-03
/// bool DCF77Receive::wasLastSignalLong() added
/// 
/// @copyright Copyright (c) 2022
/// 
//////////////////////////////////////////////////////////////////////////////

#ifndef _DCF77_HPP_ 
#define _DCF77_HPP_

#include <stdint.h>

constexpr uint16_t THRESHOLD_DUR_MINUTE      = 1500;
constexpr uint8_t  THRESHOLD_DUR_LONG_SIGNAL =  150;
constexpr uint8_t  HOUR_CHANGE = 59;

enum DCF77Sequence {SEQ_ERROR,
                    MAX_SECONDS=59U,
                    LEAP_SECOND=60U};

class DCF77Receive {
private:  
  static bool _activeLow;
  static uint16_t _duration;
  static uint32_t _lastInt;
protected:
  static uint8_t _intPin;
  static uint8_t _seconds;
  static uint64_t _sequenceBuffer;
  static DCF77Sequence _sequenceFlag;

private:
  static void receiveSequence(void);
protected:
  DCF77Receive(void) {};
public:
  void begin(void);
  void begin(uint8_t);
  void setActiveLow(bool);
  DCF77Sequence getSequenceFlag(void);
  bool wasLastSignalLong(void);
};

class DCF77Clock : public DCF77Receive { 
private:
  uint8_t  _oldMinutes;
  uint8_t  _oldHours;

  //uint8_t switchMEZ;
  //uint8_t summertime;
  uint8_t _leapSecond;
  uint8_t _startBit;
  uint8_t _year;
  uint8_t _month;
  uint8_t _dayOfWeek;
  uint8_t _dayOfMonth; 
  uint8_t _hours;
  uint8_t _minutes;
  bool _parityBitMinutes;
  bool _parityBitHours;
  bool _parityBitDate;
  bool _parityTimeOK;
  bool _parityDateOK;
private:
  uint8_t bcdToDec(uint8_t bcd) const { // inline 
     return bcd - 6 * (bcd >> 4);
  }

public:
  DCF77Clock(void) : DCF77Receive() {};

  bool decodeSequence(void);
  bool getLeapSecond(void) const;
  uint8_t getSeconds(void) const;
  uint8_t getMinutes(void) const;
  uint8_t getHours(void) const;
  uint8_t getDay(void) const;
  uint8_t getMonth(void) const;
  uint8_t getYear(void) const;

  uint8_t getBcdMinutes(void) const;
  uint8_t getBcdHours(void) const;
  uint8_t getBcdDay(void) const;
  uint8_t getBcdMonth(void) const;
  uint8_t getBcdYear(void) const;
};
#endif
