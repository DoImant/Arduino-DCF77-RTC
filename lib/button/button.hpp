//////////////////////////////////////////////////////////////////////////////
/// @file Button.hpp
/// @author Kai R. 
/// @brief Simple class for handling buttons.
///        A time (in ms) can be specified after which a button press is considered "long". 
///        Correspondingly, the tic() method returns the status NONE, LONG or SHORT.
/// 
/// @date 2022-05-04
/// @version 1.0
/// 
/// @copyright Copyright (c) 2022
/// 
//////////////////////////////////////////////////////////////////////////////

#ifndef _BUTTON_HPP_
#define _BUTTON_HPP_
#include <stdint.h>
#include <Arduino.h>
#include "digitalWriteFast.h"

//////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////

constexpr uint8_t DEBOUNCE_VAL  {30};  // The value can be reduced for buttons that bounce little. Time in ms.

//////////////////////////////////////////////////
// Variables and Classdefinitions
//////////////////////////////////////////////////
enum class ButtonState {P_NONE,P_SHORT,P_LONG};

class Button {
    uint8_t _pin;               // Button PIN number
    uint16_t _isLong;           // Saves the time (in ms) from which a key press is recognized as long.
    bool _activeState;          // Saves whether the buttons active state is HIGH or LOW.
    bool _state;                // Saves the actual state of the button.
    bool _prevState;         // Saves the previous state of the button.
    uint32_t _pressingTime;     // Saves the length of time that the button was pressed (ms).
    ButtonState _buttonState;   // Saves the status depending on how long the button was pressed.
  
    public:
    //////////////////////////////////////////////////////////////////////////////
    /// @brief Construct a new Button object
    /// 
    /// @param pin            // Pin where the Button is connected
    /// @param isLong         // time from which a key press is recognized as long
    /// @param activeState    // LOW if the button is connected with a pull up, otherwise HIGH with a pull down resistor
    //////////////////////////////////////////////////////////////////////////////
    Button(uint8_t pin = 2, uint16_t isLong = 1000, bool activeState=LOW) : 
      _pin(pin), _isLong(isLong) ,_activeState(activeState) {
      _state = !_activeState;
      if (_activeState) {                // ! is LOW
        pinModeFast(_pin, INPUT);
      } else {
        pinModeFast(_pin, INPUT_PULLUP);
      }
    }
    ButtonState tic(void);
    uint32_t getDuration(void) const;
};

//////////////////////////////////////////////////////////////////////////////
/// @brief The button query. The tic() method should be called in an endless loop.
/// 
/// @return ButtonState  The states are "Not pressed", "short pressed" and "long pressed"
//////////////////////////////////////////////////////////////////////////////
ButtonState Button::tic() {
  uint32_t now = millis();

  _prevState=_state;
  _buttonState = ButtonState::P_NONE;
  _state = digitalReadFast(_pin);
    
  if (_state == _activeState && _prevState != _activeState) {
    _pressingTime = now;
  } else if (_state != _activeState && _prevState == _activeState) {
    _pressingTime = now - _pressingTime;
    if (_pressingTime >= DEBOUNCE_VAL) {       // released after debounce time?
      _buttonState = (_pressingTime >= _isLong) ? ButtonState::P_LONG : ButtonState::P_SHORT;  
    }
  }
  return _buttonState;
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Returns the amount of time the button was pressed.
/// 
/// @return uint32_t Time in milliseconds
//////////////////////////////////////////////////////////////////////////////
uint32_t Button::getDuration(void) const {
  return _pressingTime;
}
#endif