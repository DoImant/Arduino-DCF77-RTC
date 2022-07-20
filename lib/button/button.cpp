//////////////////////////////////////////////////////////////////////////////
/// @file button.cpp
/// @author Kai R. (you@domain.com)
/// @brief Simple class for handling buttons.
///        A time (in ms) can be specified after which a button press is considered "long". 
///        Correspondingly, the tic() method returns the status NONE, LONG or SHORT.
/// 
/// @date 2022-06-03
/// @version 1.0
/// 
/// @copyright Copyright (c) 2022
/// 
//////////////////////////////////////////////////////////////////////////////
#include "button.hpp"

//////////////////////////////////////////////////////////////////////////////
/// @brief Initialize the pin to which the pushbutton is connected
/// 
/// @param pinnr 
//////////////////////////////////////////////////////////////////////////////
void Button::begin(uint8_t pinnr) {
  _pin = pinnr;
  if (_activeState) {                // ! is LOW
    pinModeFast(_pin, INPUT);
  } else {
    pinModeFast(_pin, INPUT_PULLUP);
  }
}

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