//////////////////////////////////////////////////////////////////////////////
/// @file main.cpp
/// @author Kai R.
/// @brief Clock with DCF77 receiver and DS3231 RTC module.
///        The RTC module is synchronized using the DCF77 receiver.
///
///        Used PINS:
///        Display
///          Pin D13: SCK/SCLK = CLK (Pin 29)
///          Pin D10: ChipSelect (CS/SS) = CSB (Pin 38)
///          Pin D11: MOSI = SI (Pin 28)
///          Pin D12: MISO - not used
///
///          Pin 07: RS = Read/Write Data from/to RAM (Pin 39)
///          Pin 08: RST = Reset des Displays (Pin 40)
///          Pin 09: Brightness
///
///        I2C - RTC
///          Pin 21: (A4) I2C SDA
///          Pin 22: (A5) I2C SCL
///
///        Other Control Pins
///          Pin 02: Interrupt Pin 0 = Processing of dcf77 signal.
///          Pin 03: Interrupt Pin 1 = evaluate the 1Hz signal of the RTC.
///          Pin 04: Button for switching the backlight   
///          Pin 05: Button to switch on the date         
///          PIN 06 if not ATtiny88 
///     else PIN 14:                 Switch DCF77 Receiver on or off
///          Pin 06: Reserved for debug output (Serial.print - Only Attiny88)
/// 
/// 
/// @date 2022-05-20
/// @version 1.1
/// 
/// @date 2022-06-03
/// Refactornig, added animated dots to the time display.
///
/// @copyright Copyright (c) 2022
/// 
//////////////////////////////////////////////////////////////////////////////

#include <Arduino.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include "bcdconv.hpp"
#include "dcf77.hpp"
#include "button.hpp"
#include "display.hpp"
#include "DS3231Wire.h"
#include "digitalWriteFast.h"

//////////////////////////////////////////////////
// Definitions
//////////////////////////////////////////////////

#if !( defined(__AVR_ATtinyX8__) || \
       defined(__AVR_ATmega328P__) || \
       defined(__AVR_ATmega328PB__) || \
       defined(__AVR_ATmega88P__) || \
       defined(__AVR_ATmega8__) \
      )
#error Software not suitable for the microcontroller
#endif

// Uncomment for debug output on the serial console or to switch on I2C/Wire Fast Mode
// PlatformIO: Set it in platform.ini (debug_flags)
//#define WIRE_FAST_MODE	
//#define PRINT_TIME_SERIAL
//#define DEBUG_DCF77CONTROL
//#define DEBUG_INT0_ERR                 
//#define DEBUG_INT0
//#define DEBUG_INT1                 
//#define DEBUG_DCF77_SEQ_ADD_CHECK
//#define DEBUG_DCF77_SEQ
//#define SET_TEST_TIME

#ifdef WIRE_FAST_MODE
  #define WIRE_SPEED 400000U          // I2C Fast Mode
#else
  #define WIRE_SPEED 100000U          // I2C Normal Mode
#endif

//////////////////////////////////////////////////
// Global constants and variables
//////////////////////////////////////////////////

#if defined(__AVR_ATtinyX8__)
constexpr uint8_t DCF77_ON_OFF_PIN = 14;            // Switch DCF77 Receiver on or off
#else
constexpr uint8_t DCF77_ON_OFF_PIN = 6;             // Switch DCF77 Receiver on or off
#endif
constexpr uint32_t DCF77_SLEEP = 1790U;             // Period (in seconds) for which the radio clock is switched off. Here 1790 Seconds.

// int1_second is just a counter that increases every second.
// It is not necessarily in sync with the RTC seconds
volatile uint8_t  int1_second=0;                    // Second Tick in loop(), set in INT1

DCF77Clock dcf77;
dogm_7036 lcd;

//////////////////////////////////////////////////
// Function forward declaration
//////////////////////////////////////////////////

void optimizePowerConsumption(void);
bool rtcNeedsSync(void);
void check1HzSig(void);

//////////////////////////////////////////////////////////////////////////////
/// @brief Initialize the program.
/// 
//////////////////////////////////////////////////////////////////////////////
void setup () {
  optimizePowerConsumption();

  // With ATtiny controllers, the serial output is via the Serial -TX pin. An FTD232 adapter is required.
#if defined(PRINT_TIME_SERIAL) ||defined(DEBUG_DCF77CONTROL) || defined(DEBUG_INT0_ERR) \
  || defined(DEBUG_INT0) || defined(DEBUG_INT1) || defined(DEBUG_DCF77_SEQ_ADD_CHECK) \
  || defined(DEBUG_DCF77_SEQ) 
    Serial.begin(9600);
  #ifndef ESP8266
    while (!Serial); // wait for serial port to connect. Needed for native USB
  #endif
#endif

  pinModeFast(BUTTON_BL_PIN,INPUT_PULLUP);
  pinModeFast(DCF77_ON_OFF_PIN,OUTPUT);
  digitalWriteFast(DCF77_ON_OFF_PIN,LOW);               // Switch DCF77 receiver on (P-Channel MOSFet as switch)
  
  // init DOGM-LCD
  initDisplay(lcd);

  // init DCF77
  dcf77.begin();
  dcf77.setActiveLow(true);                             // ELV DCF77 Modul works with active low signals

  // Init RTC
  Wire.begin();  
  Wire.setClock(WIRE_SPEED);
  disable32kHz();
  enableSw1Hz();
  attachInterrupt(digitalPinToInterrupt(PIND3),check1HzSig,RISING);
#ifdef SET_TEST_TIME
  setDateTime(BCDConv::decToBcd(0),
              BCDConv::decToBcd(1),
              BCDConv::decToBcd(1),
              BCDConv::decToBcd(17),
              BCDConv::decToBcd(1),
              BCDConv::decToBcd(15)
              ); // Reset RTC for testing purposes
#endif
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Time display and time synchronization.
/// 
//////////////////////////////////////////////////////////////////////////////
void loop () {
  static bool dcf77PoweredOn = true;
  static bool showDate = false;
  static uint8_t tickSecond = 61;                       // 61 to prevent initial tickSecond = int1_second.
  static uint8_t dateVisibleOffTime = 0;
  static uint32_t dcf77SleepCounter = 0;
  static Button dtButton(BUTTON_DT_PIN);
  static Button blButton(BUTTON_BL_PIN);
  Separators timeSeparator;                             // Index for separator chars (display).
  
  if (dcf77PoweredOn) {
    if (!rtcNeedsSync()) {                              // If rtcHasToSync() returns 0 (false) both clocks are synchronous.
      digitalWriteFast(DCF77_ON_OFF_PIN,HIGH);          // If both clocks synchronous switch dcf77 clock off for the DCF77_SLEEP time.
      dcf77PoweredOn = false;
      timeSeparator = Separators::SEP_COLUP;
    } else {
      timeSeparator = Separators::SEP_SPACE;
    }
  } 
    
  if (dtButton.tic() != ButtonState::P_NONE) {
    showDate = true;
    printRtcTime(lcd, timeSeparator, showDate);         // Don't wait until the next second after the button is pressed to show the date.
  }            
  
  // Do the following every second.
  // To limit the read operations to the RTC (read the second), use the count variable of INT1. 
  // Using millis() is too imprecise.
  // The clock comes from the 1Hz signal of the RTC which is present at the INT1 pin.
  if ( int1_second != tickSecond) {
    tickSecond = int1_second;
    if (!dcf77PoweredOn) {                              
      ++dcf77SleepCounter;
      if (dcf77SleepCounter == DCF77_SLEEP) {
        digitalWriteFast(DCF77_ON_OFF_PIN, LOW);        // Switch DCFAvtive-Pin - Clock ON
        dcf77PoweredOn = true;
        dcf77SleepCounter = 0;
      }
    } 
   
    if (showDate) {
      if (dateVisibleOffTime < SHOW_DATE_DURATION) {
         ++dateVisibleOffTime;
      } else {
        showDate = false;                               // showDate becomes false when the display time for the date has passed.
        dateVisibleOffTime = 0;
      }
    }
    printRtcTime(lcd,timeSeparator, showDate);
  } 
  switchBacklight(int1_second, static_cast<uint8_t>(blButton.tic())); // Switch backlight on if button has been pressed.
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Control the synchronization between the two clocks
/// 
/// @return true        There is a time difference between the dcf77 and the RTC time.
/// @return false       There is no time difference. Both clocks are synchronous.
//////////////////////////////////////////////////////////////////////////////
bool rtcNeedsSync() {
  bool rtcSetTime = true;
  //
  // If the sequenceflag != MAX_SECOND  then the sequence was not received correctly, 
  // unless it is a leap second sequence.
  // In this case, the second counter must not be unequal to MAX_SECONDS + 1.
  //
  DCF77Sequence state = dcf77.getSequenceFlag(); 
  if (state == MAX_SECONDS || (state == LEAP_SECOND && dcf77.getLeapSecond()) ) {
    dcf77.decodeSequence();
    // Compare the DCF77 time with the RTC time. The RTC will only be set if there is a time difference.
    // Because only every full minute is checked, the dcf77 seconds are always 0.
    uint8_t dcf77Compare = dcf77.getMinutes() + dcf77.getHours();     
    uint8_t rtcCompare =  BCDConv::bcdToDec(readRegister(DS3231_SECONDS)) + 
                          BCDConv::bcdToDec(readRegister(DS3231_MINUTES)) + 
                          BCDConv::bcdToDec(readRegister(DS3231_HOURS));
    uint8_t timeCompareDiff = rtcCompare - dcf77Compare;

#ifdef DEBUG_DCF77CONTROL       
    Serial.println("");
    Serial.print(F("int1_second        : "));
    Serial.println(int1_second);
    Serial.print(F("RTC   Compare Value: "));
    Serial.println(rtcCompare);
    Serial.print(F("DCF77 Compare Value: "));
    Serial.println(dcf77Compare);
    Serial.print(F("Diff. RTC to DCF77 : "));
    Serial.println(timeCompareDiff);
#endif

    // When timeCompareDiff is non-zero and not 59 at an hour change
    // there is a time difference -> set RTC Clock.
    if (timeCompareDiff && timeCompareDiff != HOUR_CHANGE) {
      rtcSetTime = true;
      setDateTime(dcf77.getBcdYear(),
                  dcf77.getBcdMonth(),
                  dcf77.getBcdDay(),
                  dcf77.getBcdHours(), 
                  dcf77.getBcdMinutes(), 
                  1
      );
#ifdef DEBUG_DCF77CONTROL
      Serial.println(F("set RTC"));
#endif
    } else {
      rtcSetTime = false;
    }
  } 
  return rtcSetTime;
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Count the seconds using the 1Hz signal from the RTC.
/// 
//////////////////////////////////////////////////////////////////////////////
void check1HzSig() {
  int1_second = (int1_second + 1) % 60;
#ifdef DEBUG_INT1
  Serial.println(int1_second);
#endif
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Disable unused peripherals and set unused Pins to input with internal 
///        pullups
/// 
//////////////////////////////////////////////////////////////////////////////
void optimizePowerConsumption(void) {
 
    //Disable digital input buffer on ADC pins 
    DIDR0 = bit(ADC5D) | bit(ADC4D) | bit(ADC3D) | bit(ADC2D) | bit(ADC1D) | bit(ADC0D);
    DIDR0 |= 0xC0 ;   //ADC7D and ADC6D are undefined in header file so set bits this way
 
    // Disable digital input buffer on Analog comparator pins 
    DIDR1 |= (1 << AIN1D) | (1 << AIN0D);
    // Disable Analog Comparator 
    ACSR |= (1 << ACD);
 
    // Power shutdown to unused peripherals (Only Atmega328PB / ATTINY88)
    #if defined(__AVR_ATtinyX8__) || defined(__AVR_ATmega328P__)
    power_adc_disable();
    #endif
    
    // Unused and unconnected pins consume power. The un-needed power load of floating pins 
    // can be avoided by using the AVR's internal pull-up resistors on the unused pins. 
    // Port pins can be set to input pull-ups by setting the DDxn bit to 0 and PORTxn bit 
    // to 1. (where x is PORT B,C,D,E and n is 0 to 7)
    // Unused pins set as input pull up
    
    // Pins Used
    // Pin 02 PD2    Used PD -> 1011 1100  
    // Pin 03 PD3            -> 1111 1100 If NOT ATtiny88
    // Pin 04 PD4
    // Pin 05 PD5   
    // Pin 06 PD6 only if NOT ATtiny88
    // Pin 07 PD7
    //
    // Pin 08 PB0    Used PB -> 0010 1111  
    // Pin 09 PB1               0110 1111 If ATtiny88
    // Pin 10 PB2
    // Pin 11 PB3
    // Pin 13 PB5
    // PIN 14 PB6 only if ATtiny88 
    //
    // Pin A4 PC4    Used PC -> 0011 0000  
    // Pin A5 PC5

#if defined(__AVR_ATtinyX8__)
    #define PORTSB 0x6F
#else 
    #define PORTSB 0x2F
#endif
    #define PORTSC 0x30  

#if defined(__AVR_ATtinyX8__)
    #define PORTSD 0xBC 
#else
    #define PORTSD 0xFC
#endif 

    DDRB &= (PORTSB);  
    DDRC &= (PORTSC); 
    DDRD &= (PORTSD);

    PORTB |= ~(PORTSB);
    PORTC |= ~(PORTSC);
    PORTD |= ~(PORTSD);
    
    cli();                             // Disable interrupts
    // turn off brown-out enable in software
    MCUCR = bit (BODS) | bit (BODSE);  // turn on brown-out enable select
    MCUCR = bit (BODS);                // this must be done within 4 clock cycles of above

    // Watchdog Timer OFF
    wdt_reset();                       // Reset watchdog timer 
    MCUSR &= ~_BV(WDRF);               // Clear WDRF in MCUSR
    WDTCSR = 0x00;                     // Turn off WDT 
    CLKPR |= 0x80;                     // Prescaler change enable to 1Mhz
    CLKPR = bit(CLKPS1) | bit(CLKPS0); // set prescaler to  8
    sei(); 
}