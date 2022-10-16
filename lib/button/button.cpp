//////////////////////////////////////////////////////////////////////////////
/// @file button.cpp
/// @author Kai R.
/// @brief Simple class for handling buttons.
///        A time (in ms) can be specified after which a button press is considered "long".
///        Correspondingly, the tic() method returns the status NONE, LONG or SHORT.
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
void Button::begin() {
  switch (activeState) {
    case HIGH: pinModeFast(pin, INPUT); break;
    case LOW: pinModeFast(pin, INPUT_PULLUP); break;
  }
}

void Button::begin(uint8_t pinnr) {
  pin = pinnr;
  begin();
}

//////////////////////////////////////////////////////////////////////////////
/// @brief The button query. The tic() method should be called in an endless loop.
///
/// @return ButtonState  The states are "Not pressed", "short pressed" and "long pressed"
//////////////////////////////////////////////////////////////////////////////
ButtonState Button::tic() {
  uint32_t now = millis();

  prevState = state;
  buttonState = ButtonState::notPressed;
  state = digitalReadFast(pin);

  if (state == activeState && prevState != activeState) {
    pressingTime = now;
  } else if (state != activeState && prevState == activeState) {
    pressingTime = now - pressingTime;
    if (pressingTime >= DEBOUNCE_VAL) {   // released after debounce time?
      buttonState = (pressingTime >= isLong) ? ButtonState::longPressed : ButtonState::shortPressed;
    }
  }
  return buttonState;
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Returns the amount of time the button was pressed.
///
/// @return uint32_t Time in milliseconds
//////////////////////////////////////////////////////////////////////////////
uint32_t Button::getDuration(void) const { return pressingTime; }