// ============================================================
// Example:     LCDML DisplayType - LiquidCrystal 
// ============================================================
// Description:
// This example includes the complete functionality over some
// tabs. All Tabs which are started with "LCDML_display_.."
// generates an output on the display / console / ....
// This example is for the author to test the complete functionality
// ============================================================
// *********************************************************************
// special settings
// *********************************************************************
// enable this line when you are not usigng a standard arduino
// for example when your chip is an ESP or a STM or SAM or something else

#define _LCDML_cfg_use_ram 

// *********************************************************************
// includes
// *********************************************************************
  #include <LiquidCrystal.h>
  #include <LCDMenuLib2.h>
  #include <global.h>

// *********************************************************************
// LCDML display settings
// *********************************************************************
  // settings for LCD
  #define _LCDML_DISP_cols  16
  #define _LCDML_DISP_rows  2

  #define _LCDML_DISP_cfg_cursor                     0x7E   // cursor Symbol
  #define _LCDML_DISP_cfg_scrollbar                  1      // enable a scrollbar

  // LCD object
  // liquid crystal needs (rs, e, dat4, dat5, dat6, dat7)
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

  const uint8_t scroll_bar[5][8] = {
    {B10001, B10001, B10001, B10001, B10001, B10001, B10001, B10001}, // scrollbar top
    {B11111, B11111, B10001, B10001, B10001, B10001, B10001, B10001}, // scroll state 1
    {B10001, B10001, B11111, B11111, B10001, B10001, B10001, B10001}, // scroll state 2
    {B10001, B10001, B10001, B10001, B11111, B11111, B10001, B10001}, // scroll state 3
    {B10001, B10001, B10001, B10001, B10001, B10001, B11111, B11111}  // scrollbar bottom
  };

// *********************************************************************
// Prototypes
// *********************************************************************
  void lcdml_menu_display();
  void lcdml_menu_clear();
  void lcdml_menu_control();

// *********************************************************************
// Global variables
// *********************************************************************


// *********************************************************************
// Objects
// *********************************************************************
  LCDMenuLib2_menu LCDML_0 (255, 0, 0, NULL, NULL); // root menu element (do not change)
  LCDMenuLib2 LCDML(LCDML_0, _LCDML_DISP_rows, _LCDML_DISP_cols, lcdml_menu_display, lcdml_menu_clear, lcdml_menu_control);


  /* ===================================================================== *
 *                                                                       *
 * Conditions to show or hide a menu element on the display              *
 *                                                                       *
 * ===================================================================== *
 */



// *********************************************************************
boolean COND_hide()  // hide a menu element
// *********************************************************************
{
  return false;  // hidden
}

// =====================================================================
//
// CONTROL v2.2.0
//
// =====================================================================
// *********************************************************************
// Features 
// - max 6 Buttons with special names (enter, quit, up, down, left, right)
// new Features on v2.2.0
// - max 64 Events, this could be a button ore something (Counter 0 - 63) 
// - standard buttons and events can be used at the same time
// - Event 0 - 3 can be used with a menu callback function (when set this event, the function is called)
// - The range from 0 - 3 can be changed in LCDMenuLib2.h
// Attention!!
// - events have to be reset manual over LCDML.CE_reset(number) ore LCDML.CE_resetAll();
// - they will not be reseted from the menu library
// *********************************************************************
// content:
// (0) Control over serial interface  with asdw_e_q
// (1) Control over one analog input
// (2) Control over 4 - 6 digital input pins (internal pullups enabled)
// (3) Control over encoder [third party lib] (Download: https://github.com/PaulStoffregen/Encoder)
// (4) Control with Keypad  [third party lib] (Download: http://playground.arduino.cc/Main/KeypadTutorial )
// (5) Control with an IRMP remote [third party lib] (Download: https://github.com/ukw100/IRMP )
// (6) Control with a joystick
// (7) Control over I2C PCF8574
// *********************************************************************

#define _LCDML_CONTROL_cfg      0

// theory:
// "#if" is a preprocessor directive and no error, look here:
// (English) https://en.wikipedia.org/wiki/C_preprocessor
// (German)  https://de.wikipedia.org/wiki/C-Pr%C3%A4prozessor

// *********************************************************************
// *************** (0) CONTROL OVER SERIAL INTERFACE *******************
// *********************************************************************
#if(_LCDML_CONTROL_cfg == 0)
// settings
  # define _LCDML_CONTROL_serial_enter           'e'
  # define _LCDML_CONTROL_serial_up              'w'
  # define _LCDML_CONTROL_serial_down            's'
  # define _LCDML_CONTROL_serial_left            'a'
  # define _LCDML_CONTROL_serial_right           'd'
  # define _LCDML_CONTROL_serial_quit            'q'

  // example for the useage of events (not needed everywhere)
  // this defines are only for examples and can be renamed
  # define _LCDML_EVENT_command                'c'
  # define _LCDML_EVENT_char_0                 '0'
  # define _LCDML_EVENT_char_1                 '1'
  # define _LCDML_EVENT_char_2                 '2'
  # define _LCDML_EVENT_char_3                 '3'
  # define _LCDML_EVENT_char_4                 '4'
  # define _LCDML_EVENT_char_5                 '5'
  # define _LCDML_EVENT_char_6                 '6'
  # define _LCDML_EVENT_char_7                 '7'
  # define _LCDML_EVENT_char_8                 '8'
  # define _LCDML_EVENT_char_9                 '9'
// *********************************************************************

void lcdml_menu_control(void)
{
  // If something must init, put in in the setup condition
  if(LCDML.BT_setup()) {
    // runs only once 
  }

  if(LCDML.CE_setup()) {
    // runs only once
  }

  // check if new serial input is available
  if (Serial.available()) {
    // read one char from input buffer    
    switch (Serial.read())
    {
      case _LCDML_CONTROL_serial_enter:  LCDML.BT_enter(); break;
      case _LCDML_CONTROL_serial_up:     LCDML.BT_up();    break;
      case _LCDML_CONTROL_serial_down:   LCDML.BT_down();  break;
      case _LCDML_CONTROL_serial_left:   LCDML.BT_left();  break;
      case _LCDML_CONTROL_serial_right:  LCDML.BT_right(); break;
      case _LCDML_CONTROL_serial_quit:   LCDML.BT_quit();  break;
      // example for event handling 
      // custom event handling
      // is is also possible to enable more the one event on the same time
      // but when more then one events with callback functions are active 
      // only the first callback function is called. (first = by number)
      case _LCDML_EVENT_command:  LCDML.CE_set(0);   break;
      case _LCDML_EVENT_char_0:   LCDML.CE_set(1);   break;
      case _LCDML_EVENT_char_1:   LCDML.CE_set(2);   break;
      case _LCDML_EVENT_char_2:   LCDML.CE_set(3);   break;
      case _LCDML_EVENT_char_3:   LCDML.CE_set(4);   break;
      case _LCDML_EVENT_char_4:   LCDML.CE_set(5);   break;
      case _LCDML_EVENT_char_5:   LCDML.CE_set(6);   break;
      case _LCDML_EVENT_char_6:   LCDML.CE_set(7);   break;
      case _LCDML_EVENT_char_7:   LCDML.CE_set(8);   break;
      case _LCDML_EVENT_char_8:   LCDML.CE_set(9);   break;
      case _LCDML_EVENT_char_9:   LCDML.CE_set(10);  break;
      default: break;
    }
  }
}

// *********************************************************************
// ******************************* END *********************************
// *********************************************************************





// *********************************************************************
// *************** (1) CONTROL OVER ONE ANALOG PIN *********************
// *********************************************************************
#elif(_LCDML_CONTROL_cfg == 1)

  unsigned long g_LCDML_DISP_press_time = 0;
// settings
  #define _LCDML_CONTROL_analog_pin              0
  // when you did not use a button set the value to zero
  #define _LCDML_CONTROL_analog_enter_min        850     // Button Enter
  #define _LCDML_CONTROL_analog_enter_max        920
  #define _LCDML_CONTROL_analog_up_min           520     // Button Up
  #define _LCDML_CONTROL_analog_up_max           590
  #define _LCDML_CONTROL_analog_down_min         700     // Button Down
  #define _LCDML_CONTROL_analog_down_max         770
  #define _LCDML_CONTROL_analog_back_min         950     // Button Back
  #define _LCDML_CONTROL_analog_back_max         1020
  #define _LCDML_CONTROL_analog_left_min         430     // Button Left
  #define _LCDML_CONTROL_analog_left_max         500
  #define _LCDML_CONTROL_analog_right_min        610     // Button Right
  #define _LCDML_CONTROL_analog_right_max        680
// *********************************************************************

void lcdml_menu_control(void)
{
  // If something must init, put in in the setup condition
  if(LCDML.BT_setup()) {
    // runs only once
  }
  // check debounce timer
  if((millis() - g_LCDML_DISP_press_time) >= 200) {
    g_LCDML_DISP_press_time = millis(); // reset debounce timer

    uint16_t value = analogRead(_LCDML_CONTROL_analog_pin);  // analog pin for keypad

    if (value >= _LCDML_CONTROL_analog_enter_min && value <= _LCDML_CONTROL_analog_enter_max) { LCDML.BT_enter(); }
    if (value >= _LCDML_CONTROL_analog_up_min    && value <= _LCDML_CONTROL_analog_up_max)    { LCDML.BT_up();    }
    if (value >= _LCDML_CONTROL_analog_down_min  && value <= _LCDML_CONTROL_analog_down_max)  { LCDML.BT_down();  }
    if (value >= _LCDML_CONTROL_analog_left_min  && value <= _LCDML_CONTROL_analog_left_max)  { LCDML.BT_left();  }
    if (value >= _LCDML_CONTROL_analog_right_min && value <= _LCDML_CONTROL_analog_right_max) { LCDML.BT_right(); }
    if (value >= _LCDML_CONTROL_analog_back_min  && value <= _LCDML_CONTROL_analog_back_max)  { LCDML.BT_quit();  }
  }
}
// *********************************************************************
// ******************************* END *********************************
// *********************************************************************






// *********************************************************************
// *************** (2) CONTROL OVER DIGITAL PINS ***********************
// *********************************************************************
#elif(_LCDML_CONTROL_cfg == 2)
// settings
  unsigned long g_LCDML_DISP_press_time = 0;

  #define _LCDML_CONTROL_digital_low_active      0    // 0 = high active (pulldown) button, 1 = low active (pullup)
                                                      // http://playground.arduino.cc/CommonTopics/PullUpDownResistor
  #define _LCDML_CONTROL_digital_enable_quit     1
  #define _LCDML_CONTROL_digital_enable_lr       1
  #define _LCDML_CONTROL_digital_enter           8
  #define _LCDML_CONTROL_digital_up              9
  #define _LCDML_CONTROL_digital_down            10
  #define _LCDML_CONTROL_digital_quit            11
  #define _LCDML_CONTROL_digital_left            12
  #define _LCDML_CONTROL_digital_right           13
// *********************************************************************
void lcdml_menu_control(void)
{
  // If something must init, put in in the setup condition
  if(LCDML.BT_setup()) {
    // runs only once
    // init buttons
    pinMode(_LCDML_CONTROL_digital_enter      , INPUT_PULLUP);
    pinMode(_LCDML_CONTROL_digital_up         , INPUT_PULLUP);
    pinMode(_LCDML_CONTROL_digital_down       , INPUT_PULLUP);
    # if(_LCDML_CONTROL_digital_enable_quit == 1)
      pinMode(_LCDML_CONTROL_digital_quit     , INPUT_PULLUP);
    # endif
    # if(_LCDML_CONTROL_digital_enable_lr == 1)
      pinMode(_LCDML_CONTROL_digital_left     , INPUT_PULLUP);
      pinMode(_LCDML_CONTROL_digital_right    , INPUT_PULLUP);
    # endif
  }

  #if(_LCDML_CONTROL_digital_low_active == 1)
  #  define _LCDML_CONTROL_digital_a !
  #else
  #  define _LCDML_CONTROL_digital_a
  #endif

  uint8_t but_stat = 0x00;

  bitWrite(but_stat, 0, _LCDML_CONTROL_digital_a(digitalRead(_LCDML_CONTROL_digital_enter)));
  bitWrite(but_stat, 1, _LCDML_CONTROL_digital_a(digitalRead(_LCDML_CONTROL_digital_up)));
  bitWrite(but_stat, 2, _LCDML_CONTROL_digital_a(digitalRead(_LCDML_CONTROL_digital_down)));
  #if(_LCDML_CONTROL_digital_enable_quit == 1)
  bitWrite(but_stat, 3, _LCDML_CONTROL_digital_a(digitalRead(_LCDML_CONTROL_digital_quit)));
  #endif
  #if(_LCDML_CONTROL_digital_enable_lr == 1)
  bitWrite(but_stat, 4, _LCDML_CONTROL_digital_a(digitalRead(_LCDML_CONTROL_digital_left)));
  bitWrite(but_stat, 5, _LCDML_CONTROL_digital_a(digitalRead(_LCDML_CONTROL_digital_right)));
  #endif

  if (but_stat > 0) {
    if((millis() - g_LCDML_DISP_press_time) >= 200) {
      g_LCDML_DISP_press_time = millis(); // reset press time

      if (bitRead(but_stat, 0)) { LCDML.BT_enter(); }
      if (bitRead(but_stat, 1)) { LCDML.BT_up();    }
      if (bitRead(but_stat, 2)) { LCDML.BT_down();  }
      if (bitRead(but_stat, 3)) { LCDML.BT_quit();  }
      if (bitRead(but_stat, 4)) { LCDML.BT_left();  }
      if (bitRead(but_stat, 5)) { LCDML.BT_right(); }
    }
  }
}
// *********************************************************************
// ******************************* END *********************************
// *********************************************************************






// *********************************************************************
// *************** (3) CONTROL WITH ENCODER ****************************
// *********************************************************************
#elif(_LCDML_CONTROL_cfg == 3)
  /*
   * Thanks to "MuchMore" (Arduino forum) to add this encoder functionality
   *
   * rotate left = Up
   * rotate right = Down
   * push = Enter
   * push long = Quit
   * push + rotate left = Left
   * push + rotate right = Right
   */

  /* encoder connection
   * button * (do not use an external resistor, the internal pullup resistor is used)
   * .-------o  Arduino Pin
   * |
   * |
   * o  /
   *   /
   *  /
   * o
   * |
   * '-------o   GND
   *
   * encoder * (do not use an external resistor, the internal pullup resistors are used)
   *
   * .---------------o  Arduino Pin A
   * |        .------o  Arduino Pin B
   * |        |
   * o  /     o  /
   *   /        /
   *  /        /
   * o        o
   * |        |
   * '--------o----o   GND (common pin)
   */

  // global defines
  #define encoder_A_pin       20    // physical pin has to be 2 or 3 to use interrupts (on mega e.g. 20 or 21), use internal pullups
  #define encoder_B_pin       21    // physical pin has to be 2 or 3 to use interrupts (on mega e.g. 20 or 21), use internal pullups
  #define encoder_button_pin  49    // physical pin , use internal pullup

  #define g_LCDML_CONTROL_button_long_press    800   // ms
  #define g_LCDML_CONTROL_button_short_press   120   // ms

  #define ENCODER_OPTIMIZE_INTERRUPTS //Only when using pin2/3 (or 20/21 on mega)
  #include <Encoder.h> //for Encoder    Download:  https://github.com/PaulStoffregen/Encoder

  Encoder ENCODER(encoder_A_pin, encoder_B_pin);

  unsigned long  g_LCDML_CONTROL_button_press_time = millis();
  bool  g_LCDML_CONTROL_button_prev       = HIGH;

// *********************************************************************
void lcdml_menu_control(void)
// *********************************************************************
{
  // declare variable for this function
  int32_t g_LCDML_CONTROL_Encoder_position = ENCODER.read();
  bool g_LCDML_button                      = digitalRead(encoder_button_pin);
  
  // If something must init, put in in the setup condition
  if(LCDML.BT_setup())
  {
    // runs only once

    // init pins, enable pullups
    pinMode(encoder_A_pin      , INPUT_PULLUP);
    pinMode(encoder_B_pin      , INPUT_PULLUP);
    pinMode(encoder_button_pin  , INPUT_PULLUP);
  }

  // check if encoder is rotated on direction A
  if(g_LCDML_CONTROL_Encoder_position <= -3) 
  {
    // check if the button is pressed and the encoder is rotated
    // the button is low active
    if(g_LCDML_button == LOW)
    {
      // button is pressed
      LCDML.BT_left();  

      // reset button press time for next detection
      g_LCDML_CONTROL_button_prev = HIGH;
    }
    else
    {
      LCDML.BT_down();
    }    

    // init encoder for the next step
    ENCODER.write(g_LCDML_CONTROL_Encoder_position+4);
  }
  
  // check if encoder is rotated on direction B
  else if(g_LCDML_CONTROL_Encoder_position >= 3)
  {    
    // check if the button is pressed and the encoder is rotated
    // the button is low active
    if(g_LCDML_button == LOW)
    {
      // button is pressed
      LCDML.BT_right(); 

      // reset button press time for next detection
      g_LCDML_CONTROL_button_prev = HIGH;
    }
    else
    {
      LCDML.BT_up();
    }   

    // init encoder for the next step
    ENCODER.write(g_LCDML_CONTROL_Encoder_position-4);
  }
  else
  {
    // check if the button was pressed for a shortly time or a long time
    
    //falling edge, button pressed, no action
    if(g_LCDML_button == LOW && g_LCDML_CONTROL_button_prev == HIGH)  
    {
      g_LCDML_CONTROL_button_prev = LOW;
      g_LCDML_CONTROL_button_press_time = millis();
    }

    // rising edge, button not pressed, check how long was it pressed    
    else if(g_LCDML_button == HIGH && g_LCDML_CONTROL_button_prev == LOW) 
    {
      g_LCDML_CONTROL_button_prev = HIGH;
      
      // check how long was the button pressed and detect a long press or a short press

      // check long press situation
      if((millis() - g_LCDML_CONTROL_button_press_time) >= g_LCDML_CONTROL_button_long_press)
      {
        // long press detected
        LCDML.BT_quit();
      }
      // check short press situation
      else if((millis() - g_LCDML_CONTROL_button_press_time) >= g_LCDML_CONTROL_button_short_press)
      {
        // short press detected
        LCDML.BT_enter();
      }
    }

    // do nothing
    else 
    {
      // do nothing
    }
  }
}
// *********************************************************************
// ******************************* END *********************************
// *********************************************************************





// *********************************************************************
// *************** (4) CONTROL WITH A KEYPAD ***************************
// *********************************************************************
#elif(_LCDML_CONTROL_cfg == 4)
// include
  // more information under http://playground.arduino.cc/Main/KeypadTutorial
  #include <Keypad.h>
// settings
  #define _LCDML_CONTROL_keypad_rows 4 // Four rows
  #define _LCDML_CONTROL_keypad_cols 3 // Three columns
// global vars
  char keys[_LCDML_CONTROL_keypad_rows][_LCDML_CONTROL_keypad_cols] = {
    {'1','2','3'},
    {'4','5','6'},
    {'7','8','9'},
    {'#','0','*'}
  };
  byte rowPins[_LCDML_CONTROL_keypad_rows] = { 9, 8, 7, 6 };  // Connect keypad COL0, COL1 and COL2 to these Arduino pins.
  byte colPins[_LCDML_CONTROL_keypad_cols] = { 12, 11, 10 };  // Create the Keypad
// objects
  Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, _LCDML_CONTROL_keypad_rows, _LCDML_CONTROL_keypad_cols );
// *********************************************************************
void lcdml_menu_control(void)
{
  // If something must init, put in in the setup condition
  if(LCDML.BT_setup()) {
    // runs only once
  }
  char key = kpd.getKey();
  if(key)  // Check for a valid key.
  {
    switch (key)
    {
      // this is the default configuration 
      case '#': LCDML.BT_enter(); break;
      case '2': LCDML.BT_up();    break;
      case '8': LCDML.BT_down();  break;
      case '4': LCDML.BT_left();  break;
      case '6': LCDML.BT_right(); break;
      case '*': LCDML.BT_quit();  break;

      // when you want to use all characters you have to use the CE_ functionality
      // CE stands for "custom event" and you can define 64 evetns
      // the following code is only an example      
      /*
      case '1': LCDML.CE_set(2); break;
      case '2': LCDML.CE_set(3); LCDML.BT_up();    break;
      case '3': LCDML.CE_set(4); break; 
      case '4': LCDML.CE_set(5); LCDML.BT_left();  break;
      case '5': LCDML.CE_set(6); break; 
      case '6': LCDML.CE_set(7); LCDML.BT_right(); break;
      case '7': LCDML.CE_set(8); break; 
      case '8': LCDML.CE_set(9); LCDML.BT_down();  break;
      case '9': LCDML.CE_set(10); break; 
      case '0': LCDML.CE_set(1); break; 
      case '#': LCDML.CE_set(12); LCDML.BT_enter(); break;
      case '*': LCDML.CE_set(11); LCDML.BT_quit();  break;
      */      
      default: break;
    }
  }
}
// *********************************************************************
// ******************************* END *********************************
// *********************************************************************


// *********************************************************************
// *************** (5) CONTROL WITH IR REMOTE ***************************
// *********************************************************************
#elif(_LCDML_CONTROL_cfg == 5)
    // IR include (this lib have to be installed)
    // Download path: https://github.com/ukw100/IRMP
    #define IRMP_INPUT_PIN PA0
    #define IRMP_PROTOCOL_NAMES 1 // Enable protocol number mapping to protocol strings - needs some FLASH. Must before #include <irmp*>
    #include <irmpSelectMain15Protocols.h> // This enables 15 main protocols

    #include <irmp.c.h>

    IRMP_DATA irmp_data[1];

    #define STR_HELPER(x) #x
    #define STR(x) STR_HELPER(x)

    void handleReceivedIRData();

// *********************************************************************
// change in this function the IR values to your values
void lcdml_menu_control(void)
{
  // If something must init, put in in the setup condition
  if(LCDML.BT_setup()) {
    // runs only once
    irmp_init();
  }

  if (irmp_get_data(&irmp_data[0]))
  {
    // comment this line out, to check the correct code
    //Serial.println(results.value, HEX);

    // in this switch case you have to change the value 0x...1 to the correct IR code
    switch (irmp_data[0].command)
    {
      case 0x52: LCDML.BT_enter(); break;
      case 0x50: LCDML.BT_up();    break;
      case 0x51: LCDML.BT_down();  break;
      case 0x55: LCDML.BT_left();  break;
      case 0x56: LCDML.BT_right(); break;
      case 0x23: LCDML.BT_quit();  break;
      default: break;
    }
  }
}
// *********************************************************************
// ******************************* END *********************************
// *********************************************************************



// *********************************************************************
// *************** (6) CONTROL OVER JOYSTICK ***************************
// *********************************************************************
#elif(_LCDML_CONTROL_cfg == 6)
  unsigned long g_LCDML_DISP_press_time = 0;
    // settings
    #define _LCDML_CONTROL_analog_pinx A0
    #define _LCDML_CONTROL_analog_piny A1
    #define _LCDML_CONTROL_digitalread 33 //don't work with u8glib

    // when you did not use a button set the value to zero
    #define _LCDML_CONTROL_analog_up_min 612 // Button Up
    #define _LCDML_CONTROL_analog_up_max 1023
    #define _LCDML_CONTROL_analog_down_min 0 // Button Down
    #define _LCDML_CONTROL_analog_down_max 412
    #define _LCDML_CONTROL_analog_left_min 612 // Button Left
    #define _LCDML_CONTROL_analog_left_max 1023
    #define _LCDML_CONTROL_analog_right_min 0 // Button Right
    #define _LCDML_CONTROL_analog_right_max 412
// *********************************************************************
void lcdml_menu_control(void)
{
  // If something must init, put in in the setup condition
  if(LCDML.BT_setup()) {
    // runs only once
    pinMode (_LCDML_CONTROL_digitalread, INPUT);
  }
  // check debounce timer
  if((millis() - g_LCDML_DISP_press_time) >= 200) {
    g_LCDML_DISP_press_time = millis(); // reset debounce timer

    uint16_t valuex = analogRead(_LCDML_CONTROL_analog_pinx);  // analogpinx
    uint16_t valuey = analogRead(_LCDML_CONTROL_analog_piny);  // analogpinx
    uint16_t valuee = digitalRead(_LCDML_CONTROL_digitalread);  //digitalpinenter


    if (valuey >= _LCDML_CONTROL_analog_up_min && valuey <= _LCDML_CONTROL_analog_up_max) { LCDML.BT_up(); }        // up
    if (valuey >= _LCDML_CONTROL_analog_down_min && valuey <= _LCDML_CONTROL_analog_down_max) { LCDML.BT_down(); }    // down
    if (valuex >= _LCDML_CONTROL_analog_left_min && valuex <= _LCDML_CONTROL_analog_left_max) { LCDML.BT_left(); }     // left
    if (valuex >= _LCDML_CONTROL_analog_right_min && valuex <= _LCDML_CONTROL_analog_right_max) { LCDML.BT_right(); }    // right

    if(valuee == true) {LCDML.BT_enter();}    // enter

    // back buttons have to be included as menu item
    // lock at the example LCDML_back_button
  }
}
// *********************************************************************
// ******************************* END *********************************
// *********************************************************************

// *********************************************************************
// *************** (7) CONTROL OVER PCF8574 ****************************
// *********************************************************************

#elif(_LCDML_CONTROL_cfg == 7)
  unsigned long g_LCDML_DISP_press_time = 0;
  #define PCF8574_1 0x26 // I2C address for the buttons

  #define PCF8574_Pin0 254
  #define PCF8574_Pin1 253
  #define PCF8574_Pin2 251
  #define PCF8574_Pin3 247
  #define PCF8574_Pin4 239
  #define PCF8574_Pin5 223
  #define PCF8574_Pin6 191
  #define PCF8574_Pin7 127

  // Specify the PCF8574 pins here
  #define _LCDML_CONTROL_PCF8574_enable_quit    0
  #define _LCDML_CONTROL_PCF8574_enable_lr      0
  
  #define _LCDML_CONTROL_PCF8574_enter          PCF8574_Pin0
  #define _LCDML_CONTROL_PCF8574_up             PCF8574_Pin1
  #define _LCDML_CONTROL_PCF8574_down           PCF8574_Pin2
  #define _LCDML_CONTROL_PCF8574_left           PCF8574_Pin3
  #define _LCDML_CONTROL_PCF8574_right          PCF8574_Pin4
  #define _LCDML_CONTROL_PCF8574_quit           PCF8574_Pin5
// **********************************************************
void lcdml_menu_control(void)
{
  // If something must init, put in in the setup condition
  if(LCDML.BT_setup()) {
    // runs only once
  }

  if((millis() - g_LCDML_DISP_press_time) >= 200) {
      g_LCDML_DISP_press_time = millis(); // reset press time

    Wire.write(0xff); // All pins as input?
    Wire.requestFrom(PCF8574_1, 1);
   if (Wire.available()) {
    switch (Wire.read())
    {
      case _LCDML_CONTROL_PCF8574_enter:  LCDML.BT_enter(); break;
      case _LCDML_CONTROL_PCF8574_up:     LCDML.BT_up();    break;
      case _LCDML_CONTROL_PCF8574_down:   LCDML.BT_down();  break;
    #if(_LCDML_CONTROL_PCF8574_enable_quit == 1)
      case _LCDML_CONTROL_PCF8574_quit:   LCDML.BT_quit();  break;
    #endif
    
    #if(_LCDML_CONTROL_PCF8574_enable_lr   == 1)
      case _LCDML_CONTROL_PCF8574_left: LCDML.BT_left(); break;
      case _LCDML_CONTROL_PCF8574_right:   LCDML.BT_right();  break;
    #endif
    
      default: break;
    }
  }
 }
}
// *********************************************************************
// ******************************* END *********************************
// *********************************************************************


#else
  #error _LCDML_CONTROL_cfg is not defined or not in range
#endif

/* ===================================================================== *
 *                                                                       *
 * Dynamic content                                                       *
 *                                                                       *
 * ===================================================================== *
 */


uint8_t g_dynParam = 100; // when this value comes from an EEPROM, load it in setup
                          // at the moment here is no setup function (To-Do)
void mDyn_para(uint8_t line)
// *********************************************************************
{
  // check if this function is active (cursor stands on this line)
  if (line == LCDML.MENU_getCursorPos())
  {
    // make only an action when the cursor stands on this menu item
    //check Button
    if(LCDML.BT_checkAny())
    {
      if(LCDML.BT_checkEnter())
      {
        // this function checks returns the scroll disable status (0 = menu scrolling enabled, 1 = menu scrolling disabled)
        if(LCDML.MENU_getScrollDisableStatus() == 0)
        {
          // disable the menu scroll function to catch the cursor on this point
          // now it is possible to work with BT_checkUp and BT_checkDown in this function
          // this function can only be called in a menu, not in a menu function
          LCDML.MENU_disScroll();
        }
        else
        {
          // enable the normal menu scroll function
          LCDML.MENU_enScroll();
        }

        // do something
        // ...
      }

      // This check have only an effect when MENU_disScroll is set
      if(LCDML.BT_checkUp())
      {
        g_dynParam++;
      }

      // This check have only an effect when MENU_disScroll is set
      if(LCDML.BT_checkDown())
      {
        g_dynParam--;
      }


      if(LCDML.BT_checkLeft())
      {
        g_dynParam++;
      }
      if(LCDML.BT_checkRight())
      {
        g_dynParam--;
      }
    }
  }

  char buf[20];
  sprintf (buf, "dynValue: %d", g_dynParam);

  // use the line from function parameters
  lcd.setCursor(1, line);
  lcd.print(buf);

}




// =====================================================================
//
// Output function
//
// =====================================================================

/* ******************************************************************** */
void lcdml_menu_clear()
/* ******************************************************************** */
{
  lcd.clear();
  lcd.setCursor(0, 0);
}

/* ******************************************************************** */
void lcdml_menu_display()
/* ******************************************************************** */
{
  // update content
  // ***************
  if (LCDML.DISP_checkMenuUpdate()) {
    // clear menu
    // ***************
    LCDML.DISP_clear();

    // declaration of some variables
    // ***************
    // content variable
    char content_text[_LCDML_DISP_cols];  // save the content text of every menu element
    // menu element object
    LCDMenuLib2_menu *tmp;
    // some limit values
    uint8_t i = LCDML.MENU_getScroll();
    uint8_t maxi = _LCDML_DISP_rows + i;
    uint8_t n = 0;

    // check if this element has children
    if ((tmp = LCDML.MENU_getDisplayedObj()) != NULL)
    {
      // loop to display lines
      do
      {
        // check if a menu element has a condition and if the condition be true
        if (tmp->checkCondition())
        {
          // check the type off a menu element
          if(tmp->checkType_menu() == true)
          {
            // display normal content
            LCDML_getContent(content_text, tmp->getID());
            lcd.setCursor(1, n);
            lcd.print(content_text);
          }
          else
          {
            if(tmp->checkType_dynParam()) {
              tmp->callback(n);
            }
          }
          // increment some values
          i++;
          n++;
        }
      // try to go to the next sibling and check the number of displayed rows
      } while (((tmp = tmp->getSibling(1)) != NULL) && (i < maxi));
    }
  }

  if(LCDML.DISP_checkMenuCursorUpdate())
  {
    // init vars
    uint8_t n_max             = (LCDML.MENU_getChilds() >= _LCDML_DISP_rows) ? _LCDML_DISP_rows : (LCDML.MENU_getChilds());
    uint8_t scrollbar_min     = 0;
    uint8_t scrollbar_max     = LCDML.MENU_getChilds();
    uint8_t scrollbar_cur_pos = LCDML.MENU_getCursorPosAbs();
    uint8_t scroll_pos        = ((1.*n_max * _LCDML_DISP_rows) / (scrollbar_max - 1) * scrollbar_cur_pos);


    // display rows
    for (uint8_t n = 0; n < n_max; n++)
    {
      //set cursor
      lcd.setCursor(0, n);

      //set cursor char
      if (n == LCDML.MENU_getCursorPos()) {
        lcd.write(_LCDML_DISP_cfg_cursor);
      } else {
        lcd.write(' ');
      }

      // delete or reset scrollbar
      if (_LCDML_DISP_cfg_scrollbar == 1) {
        if (scrollbar_max > n_max) {
          lcd.setCursor((_LCDML_DISP_cols - 1), n);
          lcd.write((uint8_t)0);
        }
        else {
          lcd.setCursor((_LCDML_DISP_cols - 1), n);
          lcd.print(' ');
        }
      }
    }

    // display scrollbar
    if (_LCDML_DISP_cfg_scrollbar == 1) {
      if (scrollbar_max > n_max) {
        //set scroll position
        if (scrollbar_cur_pos == scrollbar_min) {
          // min pos
          lcd.setCursor((_LCDML_DISP_cols - 1), 0);
          lcd.write((uint8_t)1);
        } else if (scrollbar_cur_pos == (scrollbar_max - 1)) {
          // max pos
          lcd.setCursor((_LCDML_DISP_cols - 1), (n_max - 1));
          lcd.write((uint8_t)4);
        } else {
          // between
          lcd.setCursor((_LCDML_DISP_cols - 1), scroll_pos / n_max);
          lcd.write((uint8_t)(scroll_pos % n_max) + 1);
        }
      }
    }
  }
}




/* ===================================================================== *
 *                                                                       *
 * Menu Callback Function                                                *
 *                                                                       *
 * ===================================================================== *
 *
 * EXAMPLE CODE:

// *********************************************************************
void your_function_name(uint8_t param)
// *********************************************************************
{
  if(LCDML.FUNC_setup())          // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    //LCDML_UNUSED(param);
    // setup
    // is called only if it is started

    // starts a trigger event for the loop function every 100 milliseconds
    LCDML.FUNC_setLoopInterval(100);

    // uncomment this line when the menu should go back to the last called position
    // this could be a cursor position or the an active menu function
    // GBA means => go back advanced
    //LCDML.FUNC_setGBA() 

    //
  }

  if(LCDML.FUNC_loop())           // ****** LOOP *********
  {
    // loop
    // is called when it is triggered
    // - with LCDML_DISP_triggerMenu( milliseconds )
    // - with every button or event status change

    // uncomment this line when the screensaver should not be called when this function is running
    // reset screensaver timer
    //LCDML.SCREEN_resetTimer();

    // check if any button is pressed (enter, up, down, left, right)
    if(LCDML.BT_checkAny()) {
      LCDML.FUNC_goBackToMenu();
    }
  }

  if(LCDML.FUNC_close())      // ****** STABLE END *********
  {
    // loop end
    // you can here reset some global vars or delete it
    // this function is always called when the functions ends.
    // this means when you are calling a jumpTo ore a goRoot function
    // that this part is called before a function is closed
  }
}


 * ===================================================================== *
 */


// *********************************************************************
void mFunc_information(uint8_t param)
// *********************************************************************
{
  if(LCDML.FUNC_setup())          // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);

    // setup function
    lcd.setCursor(0, 0);
    lcd.print(F("To close this"));
    lcd.setCursor(0, 1);
    lcd.print(F("function press"));
    lcd.setCursor(0, 2);
    lcd.print(F("any button or use"));
    lcd.setCursor(0, 3);
    lcd.print(F("back button"));
  }

  if(LCDML.FUNC_loop())           // ****** LOOP *********
  {
    // loop function, can be run in a loop when LCDML_DISP_triggerMenu(xx) is set
    // the quit button works in every DISP function without any checks; it starts the loop_end function
    if(LCDML.BT_checkAny()) { // check if any button is pressed (enter, up, down, left, right)
      // LCDML_goToMenu stops a running menu function and goes to the menu
      LCDML.FUNC_goBackToMenu();
    }
  }

  if(LCDML.FUNC_close())      // ****** STABLE END *********
  {
    // you can here reset some global vars or do nothing
  }
}


// *********************************************************************
uint8_t g_func_timer_info = 0;  // time counter (global variable)
unsigned long g_timer_1 = 0;    // timer variable (global variable)
void mFunc_timer_info(uint8_t param)
// *********************************************************************
{
  if(LCDML.FUNC_setup())          // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);

    lcd.print(F("20 sec wait")); // print some content on first row
    g_func_timer_info = 20;       // reset and set timer
    LCDML.FUNC_setLoopInterval(100);  // starts a trigger event for the loop function every 100 milliseconds

    LCDML.TIMER_msReset(g_timer_1);
  }


  if(LCDML.FUNC_loop())           // ****** LOOP *********
  {
    // loop function, can be run in a loop when LCDML_DISP_triggerMenu(xx) is set
    // the quit button works in every DISP function without any checks; it starts the loop_end function

    // reset screensaver timer
    LCDML.SCREEN_resetTimer();

    // this function is called every 100 milliseconds

    // this method checks every 1000 milliseconds if it is called
    if(LCDML.TIMER_ms(g_timer_1, 1000)) {
      g_func_timer_info--;                // increment the value every second
      lcd.setCursor(0, 0);                // set cursor pos
      lcd.print(F("  "));
      lcd.setCursor(0, 0);                // set cursor pos
      lcd.print(g_func_timer_info);       // print the time counter value
    }

    // this function can only be ended when quit button is pressed or the time is over
    // check if the function ends normally
    if (g_func_timer_info <= 0)
    {
      // leave this function
      LCDML.FUNC_goBackToMenu();
    }
  }

  if(LCDML.FUNC_close())      // ****** STABLE END *********
  {
    // you can here reset some global vars or do nothing
  }
}


// *********************************************************************
uint8_t g_button_value = 0; // button value counter (global variable)
void mFunc_p2(uint8_t param)
// *********************************************************************
{
  if(LCDML.FUNC_setup())          // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);

    // setup function
    // print LCD content
    // print LCD content
    lcd.setCursor(0, 0);
    lcd.print(F("press left or up"));
    lcd.setCursor(0, 1);
    lcd.print(F("count: 0 of 3"));
    // Reset Button Value
    g_button_value = 0;

    // Disable the screensaver for this function until it is closed
    LCDML.FUNC_disableScreensaver();

  }

  if(LCDML.FUNC_loop())           // ****** LOOP *********
  {
    // the quit button works in every DISP function without any checks; it starts the loop_end function
    if (LCDML.BT_checkAny()) // check if any button is pressed (enter, up, down, left, right)
    {
      if (LCDML.BT_checkLeft() || LCDML.BT_checkUp()) // check if button left is pressed
      {
        LCDML.BT_resetLeft(); // reset the left button
        LCDML.BT_resetUp(); // reset the left button
        g_button_value++;

        // update LCD content
        // update LCD content
        lcd.setCursor(7, 1); // set cursor
        lcd.print(g_button_value); // print change content
      }
    }

    // check if button count is three
    if (g_button_value >= 3) {
      LCDML.FUNC_goBackToMenu();      // leave this function
    }
  }

  if(LCDML.FUNC_close())     // ****** STABLE END *********
  {
    // you can here reset some global vars or do nothing
  }
}



// *********************************************************************
void mFunc_screensaver(uint8_t param)
// *********************************************************************
{
  if(LCDML.FUNC_setup())          // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);

    // update LCD content
    lcd.setCursor(0, 0); // set cursor
    lcd.print(F("screensaver")); // print change content
    lcd.setCursor(0, 1); // set cursor
    lcd.print(F("press any key"));
    lcd.setCursor(0, 2); // set cursor
    lcd.print(F("to leave it"));
    LCDML.FUNC_setLoopInterval(100);  // starts a trigger event for the loop function every 100 milliseconds
  }

  if(LCDML.FUNC_loop())
  {
    if (LCDML.BT_checkAny()) // check if any button is pressed (enter, up, down, left, right)
    {
      LCDML.FUNC_goBackToMenu();  // leave this function
    }
  }

  if(LCDML.FUNC_close())
  {
    // The screensaver go to the root menu
    LCDML.MENU_goRoot();
  }
}



// *********************************************************************
void mFunc_back(uint8_t param)
// *********************************************************************
{
  if(LCDML.FUNC_setup())          // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);

    // end function and go an layer back
    LCDML.FUNC_goBackToMenu(1);      // leave this function and go a layer back
  }
}


// *********************************************************************
void mFunc_goToRootMenu(uint8_t param)
// *********************************************************************
{
  if(LCDML.FUNC_setup())          // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);

    // go to root and display menu
    LCDML.MENU_goRoot();
  }
}

// *********************************************************************
void mFunc_jumpTo_timer_info(uint8_t param)
// *********************************************************************
{
  if(LCDML.FUNC_setup())          // ****** SETUP *********
  {
    // remmove compiler warnings when the param variable is not used:
    LCDML_UNUSED(param);
    
    // Jump to main screen
    LCDML.OTHER_jumpToFunc(mFunc_timer_info);
  }
}


// *********************************************************************
void mFunc_para(uint8_t param)
// *********************************************************************
{
  if(LCDML.FUNC_setup())          // ****** SETUP *********
  {

    char buf[20];
    sprintf (buf, "parameter: %d", param);

    lcd.setCursor(0, 0);
    lcd.print(buf);
    lcd.setCursor(0, 1);
    lcd.print(F("press any key"));
    lcd.setCursor(0, 2);
    lcd.print(F("to leave it"));

    LCDML.FUNC_setLoopInterval(100);  // starts a trigger event for the loop function every 100 milliseconds
  }

  if(LCDML.FUNC_loop())          // ****** LOOP *********
  {
    // For example
    switch(param)
    {
      case 10:
        // do something
        break;

      case 20:
        // do something
        break;

      case 30:
        // do something
        break;

      default:
        // do nothing
        break;
    }


    if (LCDML.BT_checkAny()) // check if any button is pressed (enter, up, down, left, right)
    {
      LCDML.FUNC_goBackToMenu();  // leave this function
    }
  }

  if(LCDML.FUNC_close())        // ****** STABLE END *********
  {
    // you can here reset some global vars or do nothing
  }
}





// *********************************************************************
// LCDML MENU/DISP
// *********************************************************************
  // LCDML_0        => layer 0
  // LCDML_0_X      => layer 1
  // LCDML_0_X_X    => layer 2
  // LCDML_0_X_X_X  => layer 3
  // LCDML_0_...      => layer ...

  // For beginners
  // LCDML_add(id, prev_layer, new_num, lang_char_array, callback_function)
  LCDML_add         (0  , LCDML_0         , 1  , "Information"      , mFunc_information);       // this menu function can be found on "LCDML_display_menuFunction" tab
  LCDML_add         (1  , LCDML_0         , 2  , "Time info"        , mFunc_timer_info);        // this menu function can be found on "LCDML_display_menuFunction" tab
  LCDML_add         (2  , LCDML_0         , 3  , "Program"          , NULL);                    // NULL = no menu function
  LCDML_add         (3  , LCDML_0_3       , 1  , "Program 1"        , NULL);                    // NULL = no menu function
  LCDML_add         (4  , LCDML_0_3_1     , 1  , "P1 dummy"         , NULL);                    // NULL = no menu function
  LCDML_add         (5  , LCDML_0_3_1     , 2  , "P1 Settings"      , NULL);                    // NULL = no menu function
  LCDML_add         (6  , LCDML_0_3_1_2   , 1  , "Warm"             , NULL);                    // NULL = no menu function
  LCDML_add         (7  , LCDML_0_3_1_2   , 2  , "Cold"             , NULL);                    // NULL = no menu function
  LCDML_add         (8  , LCDML_0_3_1_2   , 3  , "Back"             , mFunc_back);              // this menu function can be found on "LCDML_display_menuFunction" tab
  LCDML_add         (9  , LCDML_0_3_1     , 3  , "Back"             , mFunc_back);              // this menu function can be found on "LCDML_display_menuFunction" tab
  LCDML_add         (10 , LCDML_0_3       , 2  , "Program 2"        , mFunc_p2);                // this menu function can be found on "LCDML_display_menuFunction" tab
  LCDML_add         (11 , LCDML_0_3       , 3  , "Back"             , mFunc_back);              // this menu function can be found on "LCDML_display_menuFunction" tab
  LCDML_add         (12 , LCDML_0         , 4  , "Special"          , NULL);                    // NULL = no menu function
  LCDML_add         (13 , LCDML_0_4       , 1  , "Go to Root"       , mFunc_goToRootMenu);      // this menu function can be found on "LCDML_display_menuFunction" tab
  LCDML_add         (14 , LCDML_0_4       , 2  , "Jump to Time info", mFunc_jumpTo_timer_info); // this menu function can be found on "LCDML_display_menuFunction" tab
  LCDML_add         (15 , LCDML_0_4       , 3  , "Back"             , mFunc_back);              // this menu function can be found on "LCDML_display_menuFunction" tab


  // Advanced menu (for profit) part with more settings
  // Example for one function and different parameters
  // It is recommend to use parameters for switching settings like, (small drink, medium drink, big drink) or (200ml, 400ml, 600ml, 800ml) ...
  // the parameter change can also be released with dynParams on the next example
  // LCDMenuLib_addAdvanced(id, prev_layer,     new_num, condition,   lang_char_array, callback_function, parameter (0-255), menu function type  )
  LCDML_addAdvanced (16 , LCDML_0         , 5  , NULL,          "Parameter"      , NULL,                0,            _LCDML_TYPE_default);                    // NULL = no menu function
  LCDML_addAdvanced (17 , LCDML_0_5       , 1  , NULL,          "Parameter 1"      , mFunc_para,       10,            _LCDML_TYPE_default);                    // NULL = no menu function
  LCDML_addAdvanced (18 , LCDML_0_5       , 2  , NULL,          "Parameter 2"      , mFunc_para,       20,            _LCDML_TYPE_default);                    // NULL = no menu function
  LCDML_addAdvanced (19 , LCDML_0_5       , 3  , NULL,          "Parameter 3"      , mFunc_para,       30,            _LCDML_TYPE_default);                    // NULL = no menu function
  LCDML_add         (20 , LCDML_0_5       , 4  , "Back"             , mFunc_back);              // this menu function can be found on "LCDML_display_menuFunction" tab


  // Example for dynamic content
  // 1. set the string to ""
  // 2. use type  _LCDML_TYPE_dynParam   instead of    _LCDML_TYPE_default
  // this function type can not be used in combination with different parameters
  // LCDMenuLib_addAdvanced(id, prev_layer,     new_num, condition,   lang_char_array, callback_function, parameter (0-255), menu function type  )
  LCDML_addAdvanced (21 , LCDML_0         , 6  , NULL,          ""                  , mDyn_para,                0,   _LCDML_TYPE_dynParam);                     // NULL = no menu function

  // Example for conditions (for example for a screensaver)
  // 1. define a condition as a function of a boolean type -> return false = not displayed, return true = displayed
  // 2. set the function name as callback (remove the braces '()' it gives bad errors)
  // LCDMenuLib_addAdvanced(id, prev_layer,     new_num, condition,   lang_char_array, callback_function, parameter (0-255), menu function type  )
  LCDML_addAdvanced (22 , LCDML_0         , 7  , COND_hide,  "screensaver"        , mFunc_screensaver,        0,   _LCDML_TYPE_default);       // this menu function can be found on "LCDML_display_menuFunction" tab

  // ***TIP*** Try to update _LCDML_DISP_cnt when you add a menu element.

  // menu element count - last element id
  // this value must be the same as the last menu element
  #define _LCDML_DISP_cnt    22

  // create menu
  LCDML_createMenu(_LCDML_DISP_cnt);

// *********************************************************************
// SETUP
// *********************************************************************
  void setup()
  {
    // serial init; only be needed if serial control is used
    Serial.begin(9600);                // start serial
    Serial.println(F(_LCDML_VERSION)); // only for examples

    // LCD Begin
    lcd.begin(_LCDML_DISP_cols,_LCDML_DISP_rows);
    // set special chars for scrollbar
    lcd.createChar(0, (uint8_t*)scroll_bar[0]);
    lcd.createChar(1, (uint8_t*)scroll_bar[1]);
    lcd.createChar(2, (uint8_t*)scroll_bar[2]);
    lcd.createChar(3, (uint8_t*)scroll_bar[3]);
    lcd.createChar(4, (uint8_t*)scroll_bar[4]);

    // LCDMenuLib Setup
    LCDML_setup(_LCDML_DISP_cnt);

    // Some settings which can be used

    // Enable Menu Rollover
    LCDML.MENU_enRollover();

    // Enable Screensaver (screensaver menu function, time to activate in ms)
    LCDML.SCREEN_enable(mFunc_screensaver, 10000); // set to 10 seconds
    //LCDML.SCREEN_disable();

    // Some needful methods

    // You can jump to a menu function from anywhere with
    //LCDML.OTHER_jumpToFunc(mFunc_p2); // the parameter is the function name
  }

// *********************************************************************
// LOOP
// *********************************************************************
  void loop()
  {
    LCDML.loop();
  }