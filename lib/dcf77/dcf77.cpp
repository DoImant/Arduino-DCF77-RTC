//////////////////////////////////////////////////////////////////////////////
/// @file dcf77.cpp
/// @author Kai R.
/// @brief Classes for handling received DCF77 signals
///
/// @date 2022-05-20
/// @version 1.0
///
/// @date 2022-06-06
/// bool DCF77Receive::wasLastSignalLong() added
///
/// @date 2022-06-06
/// Change when determining the signal length. A short signal must now be at least
/// 85ms (THRESHOLD_DUR_SHORT_SIGNAL) long to be recognized and accepted.
///
/// @copyright Copyright (c) 2022
///
//////////////////////////////////////////////////////////////////////////////

#include <Arduino.h>
#include "dcf77.hpp"
#include "digitalWriteFast.h"

//////////////////////////////////////////////////
// Initialize static class variables
//////////////////////////////////////////////////

uint8_t DCF77Receive::_intPin{PIND2};
bool DCF77Receive::_activeLow{false};
uint8_t DCF77Receive::_seconds{0};
uint16_t DCF77Receive::_duration{0};
uint32_t DCF77Receive::_lastInt{0};
bool DCF77Receive::_longSig{false};
uint64_t DCF77Receive::_sequenceBuffer{0};
DCF77Sequence DCF77Receive::_sequenceFlag{SEQ_ERROR};

// Methods of DCF77Receive  //////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
/// @brief Do basic settings. Set the default interrupt PIN. Default is INT0.
///        If a pin other than Int0 (2) or Int1 (3) is passed to the
///        begin() method, no interrupt is initialized.
///
//////////////////////////////////////////////////////////////////////////////
void DCF77Receive::begin(void) {
  pinModeFast(_intPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(_intPin), DCF77Receive::receiveSequence, CHANGE);
}

void DCF77Receive::begin(uint8_t intPin) {
  _intPin = intPin;
  if (_intPin != PIND2 && _intPin != PIND3) { return; }
  begin();
}

//////////////////////////////////////////////////////////////////////////////
/// @brief If the signals of the DCF77 module are pulled to LOW,
///        then activeLow = true must be transferred. Default is false.
///        So calling this method is only necessary if activeLow needs
///        to be set to true.
///
/// @param activeLow
//////////////////////////////////////////////////////////////////////////////
void DCF77Receive::setActiveLow(bool activeLow) { _activeLow = activeLow; }

//////////////////////////////////////////////////////////////////////////////
/// @brief Method called by ISR to receive the DCF-77 time signals.
///
//////////////////////////////////////////////////////////////////////////////
void DCF77Receive::receiveSequence() {
  _duration = millis() - _lastInt;

  if (digitalReadFast(_intPin) ^ _activeLow) {
#ifdef DEBUG_ISR
    Serial.println(_duration);
#endif
    if (_duration > THRESHOLD_DUR_MINUTE) {
      switch (_seconds) {
        case MAX_SECONDS: _sequenceFlag = MAX_SECONDS; break;
        case LEAP_SECOND: _sequenceFlag = LEAP_SECOND; break;
        default:
          _sequenceFlag = SEQ_ERROR;
          _sequenceBuffer = 0;
          break;
      }
      _seconds = 0;
    }
  } else {
#ifdef DEBUG_ISR
    Serial.print(_seconds);
    Serial.print(F(". "));
    Serial.print(_duration);
    Serial.print(F("  /  "));
#endif
    // Signals arround 200ms are a logical 1 / 100ms are logical 0. So if (duration > THRESHOLD_DUR_LONG_SIGNAL) comes
    // true set a bit.
    if (_duration > THRESHOLD_DUR_SHORT_SIGNAL) {
      _longSig = false;
      if (_duration > THRESHOLD_DUR_LONG_SIGNAL) {
        _sequenceBuffer |= ((uint64_t)1 << _seconds);
        _longSig = true;
      }
      _seconds++;
    }
    _sequenceFlag = SEQ_ERROR;
  }
  _lastInt = millis();
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Returns a flag. This flag provides information as to whether
///        a complete data reception sequence has been completed or not.
///        MAX_SECONDS or LEAP_SECOND = OK
///        SEQ_ERROR = Not OK or not yet complete.
///
/// @return DCF77Sequence MAX_SECONDS or LEAP_SECOND or SEQ_ERROR
//////////////////////////////////////////////////////////////////////////////
DCF77Sequence DCF77Receive::getSequenceFlag() { return _sequenceFlag; }

//////////////////////////////////////////////////////////////////////////////
/// @brief This method returns whether the last signal received
///        was long or short.
///
/// @return true     long signal
/// @return false    short signal
//////////////////////////////////////////////////////////////////////////////
bool DCF77Receive::wasLastSignalLong() { return _longSig; }

// Methods of DCF77Clock //////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
/// @brief Decode the Dcf77 Sequence
///
/// @return true  The reception of the time sequence was correct.
/// @return false The reception of the time sequence was faulty.
//////////////////////////////////////////////////////////////////////////////
bool DCF77Clock::decodeSequence() {

  if (_sequenceFlag == SEQ_ERROR) { return false; }
  _parityTimeOK = false;
  _parityDateOK = false;
  //
  // There are two consecutive correct receive sequences are required to set the RTC.
  //
  _oldMinutes = _minutes;
  _oldHours = _hours;
  // dcf77Seq->switchMEZ = (int0_dcf77DataBuffer >> 16) & 0x01;        // Time changes to MEZ/MESZ after actal hour
  // dcf77Seq->summertime = (int0_dcf77DataBuffer >> 17) & 0x03;       // MEZ: b17=0 b18=1 / MESZ = b17=1 b18=0  bit
  // 17-18
  _leapSecond = (_sequenceBuffer >> 19) & 0x01;         // If true a leap second is set in the following hour
  _startBit = (_sequenceBuffer >> 20) & 0x01;           // startbit = 20 must be one!
  _minutes = (_sequenceBuffer >> 21) & 0x7F;            // minute = 21-27
  _parityBitMinutes = (_sequenceBuffer >> 28) & 0x01;   // parity bit minutes
  _hours = (_sequenceBuffer >> 29) & 0x3F;              // hour = bit 29-34
  _parityBitHours = (_sequenceBuffer >> 35) & 0x01;     // parity bit hours
  _dayOfMonth = (_sequenceBuffer >> 36) & 0x3F;         // day of the month = bit 36-41
  _dayOfWeek = (_sequenceBuffer >> 42) & 0x07;          // day of the week = bit 42-44
  _month = (_sequenceBuffer >> 45) & 0x1F;              // month = bit 45-49
  _year = (_sequenceBuffer >> 50) & 0xFF;               // year = bit 50-57
  _parityBitDate = (_sequenceBuffer >> 58) & 0x01;      // parity bit date

  //
  // if startbit is zero anything went wrong.
  //
  if (_startBit) {
    //
    // If startbit is set (OK) do some parity checks.
    //
    if ((__builtin_parity(_minutes) == _parityBitMinutes) && (__builtin_parity(_hours) == _parityBitHours)) {
      //
      // It is possible that nonsensical time values will also result in correct parity.
      // That's why an additional check is made.
      //
      _parityTimeOK = !(abs(bcdToDec(_minutes) - ((bcdToDec(_oldMinutes) + 1) % 60)) ||
                        abs(bcdToDec(_hours) -
                            ((bcdToDec(_oldHours) + (_minutes ? 0 : 1)) % 24))   // Add 1 if an hour change has occurred
      );
    }

    if (__builtin_parityl((_sequenceBuffer >> 36) & 0x3FFFFF) == _parityBitDate) {   // parity of Date bit 36-57
      _parityDateOK = true;
    }

#ifdef DEBUG_DCF77_SEQ_ADD_CHECK
    Serial.print(F("Par Time: "));
    Serial.println(_parityTimeOK);
    Serial.print(F("Par Date: "));
    Serial.println(_parityDateOK);
    Serial.print(F("hours: "));
    Serial.println((bcdToDec(_hours)));
    Serial.print(F("oldHours: "));
    Serial.println((bcdToDec(_oldHours)));
    Serial.print(F("minutes: "));
    Serial.println((bcdToDec(_minutes)));
    Serial.print(F("oldMinutes: "));
    Serial.println((bcdToDec(_oldMinutes)));
    Serial.print(F("Diff. hours   - oldHours + (0|1) (should be zero) : "));
    Serial.println(abs(bcdToDec(_hours) - ((bcdToDec(_oldHours) + (_minutes ? 0 : 1)) % 24)));
    Serial.print(F("Diff. minutes - oldMinutes + 1   (should be zero) : "));
    Serial.println(abs(bcdToDec(_minutes) - ((bcdToDec(_oldMinutes) + 1) % 60)));
#endif
  }
  _sequenceBuffer = 0;
  _sequenceFlag =
      DCF77Sequence::SEQ_ERROR;   // Prevents multiple evaluation of the time sequence in too short time intervals

#ifdef DEBUG_DCF77_SEQ
  Serial.print(F("Ls: "));
  Serial.println(bcdToDec(_leapSecond));
  Serial.println(F("Date and Time:"));
  Serial.print(F("Yr : "));
  Serial.println(bcdToDec(_year));
  Serial.print(F("Mon: "));
  Serial.println(bcdToDec(_month));
  Serial.print(F("DoM: "));
  Serial.println(bcdToDec(_dayOfMonth));
  Serial.print(F("DoW: "));
  Serial.println(_dayOfWeek);
  Serial.print(F("Hr : "));
  Serial.println(bcdToDec(_hours));
  Serial.print(F("Min: "));
  Serial.println(bcdToDec(_minutes));
#endif
  return (_parityTimeOK && _parityDateOK);
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Return LeapSecond
///
/// @return true    A leap second is inserted at the end of the hour.
/// @return false   No leap second expected
//////////////////////////////////////////////////////////////////////////////
bool DCF77Clock::getLeapSecond() const { return _leapSecond; }

//////////////////////////////////////////////////////////////////////////////
/// @brief get methods.
///
/// @return uint8_t The data elements of the DCF77 clock are supplied
///                 in decimal and in bcd format.
//////////////////////////////////////////////////////////////////////////////
uint8_t DCF77Clock::getSeconds() const { return _seconds; }

uint8_t DCF77Clock::getMinutes() const { return bcdToDec(_minutes); }

uint8_t DCF77Clock::getHours() const { return bcdToDec(_hours); }

uint8_t DCF77Clock::getDay() const { return bcdToDec(_dayOfMonth); }

uint8_t DCF77Clock::getMonth() const { return bcdToDec(_month); }

uint8_t DCF77Clock::getYear() const { return bcdToDec(_year); }

uint8_t DCF77Clock::getBcdMinutes() const { return _minutes; }

uint8_t DCF77Clock::getBcdHours() const { return _hours; }

uint8_t DCF77Clock::getBcdDay() const { return _dayOfMonth; }

uint8_t DCF77Clock::getBcdMonth() const { return _month; }

uint8_t DCF77Clock::getBcdYear() const { return _year; }
