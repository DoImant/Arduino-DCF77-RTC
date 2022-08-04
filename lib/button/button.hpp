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
enum class ButtonState : uint8_t {P_NONE,P_SHORT,P_LONG};

class Button {
    uint32_t _isLong;           // Saves the time (in ms) from which a key press is recognized as long.
    bool _activeState;          // Saves whether the buttons active state is HIGH or LOW.
    uint8_t _pin;               // Button PIN number
    bool _state;                // Saves the actual state of the button.
    bool _prevState;            // Saves the previous state of the button.
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
    Button(decltype(_isLong) isLong = 1000, decltype(_activeState) activeState=LOW) : 
      _isLong(isLong), _activeState(activeState) {
      _state = !_activeState;
    }
    void begin(uint8_t);
    ButtonState tic(void);
    uint32_t getDuration(void) const;
};

#endif