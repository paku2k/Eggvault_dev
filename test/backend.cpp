
#include "backend.h"
#include "global.h"
#include <math.h>

//globals
int brightness = -1;
char *brString;

doorDayAlarm_t openingAlarms[7];
doorDayAlarm_t closingAlarms[7];

uint16_t Year;
uint8_t Month;
uint8_t Date;
uint8_t DoW;
uint8_t Hour;
uint8_t Minute;
uint8_t Second;

KLAPPENPOSITION doorPosition, doorGoal;

// Alarm Variablen
bool century = false;
bool h12Flag;
bool pmFlag;
byte alarmDay, alarmHour, alarmMinute, alarmSecond, alarmBits;
bool alarmDy, alarmH12Flag, alarmPmFlag;


KLAPPENPOSITION LDRFlag = POS_DOWN; //bei nächstem aufwachen den Sensor nutzen
//byte errorFlag = 0; //Notalarm, wenn Klappe nicht durch Sensor geöffnet
//byte timeFlag = 0; //Alarm für LichtZeit Modus schon gekommen
KLAPPENPOSITION alarmFlag = POS_DOWN;  //Alarm kommt für Öffnen oder Schließen
//byte LDRTimer = NO_TIMER;
//uint16_t lux_open = 0;


byte lux_debounce_number = 3; // wie oft muss die Lichtschwelle erreicht sein, bis die Klappe sich bewegt
int lux_debounce_time = 20; // Zeit in sekunden zwischen zwei positiven Lichtprüfungen
int t_delta_min = 600;   // Zeit in Sekunden die zwischen zwei betätigungen per Licht vergangen sein müssen
int t_sens = 120; // Zeit in Sekunden, die zwischen zwei Sensorprüfungen vergeht

int t_err_open = 60; // Maximale Zeit zum Öffnen
int t_err_close = 60; // Maximale Zeit zum Schließen

openingMode nextMode = NICHT;
KLAPPENPOSITION nextMove = POS_DOWN;

time_t lastMove, now;

//TODO: getValuesfrom EEprom: lastMove, openAlarm, closeAlarm



volatile int count; //timer variable
hw_timer_t *timerBlocked = NULL;
hw_timer_t *timerMoving = NULL;



//devices
ESP32AnalogRead adc_ldr;
ESP32AnalogRead adc_vbatt;
RV8803 rtc;
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

void setAlarm(byte mDoW, byte mHour, byte mMinute)
{
  rtc.disableAllInterrupts();
  rtc.clearAllInterruptFlags();//Clear all flags in case any interrupts have occurred.
  rtc.setItemsToMatchForAlarm(true, true, true, false); //The alarm interrupt compares the alarm interrupt registers with the current time registers. We must choose which registers we want to compare by setting bits to true or false
  rtc.setAlarmMinutes(mMinute);
  rtc.setAlarmHours(mHour);
  rtc.setAlarmWeekday(mDoW);
  rtc.enableHardwareInterrupt(ALARM_INTERRUPT); 
}


void statusabfrage(){
  switch (doorPosition){
    case POS_UP:
      setNextClosingAlarm();
    break;
    

    case POS_DOWN:
      setNextOpeningAlarm();
    break;


    case POS_BLOCKED:
    //TODO: implement error handling here or skip position
    break;


    case POS_DRIVING:
    //TODO: do nothing or wait?
    break;
  }
}




void setNextOpeningAlarm(){
  //TODO: Implement next opening logic
}




void setNextClosingAlarm(){
  nextMove = POS_DOWN;
  LDRFlag = POS_BLOCKED;
  alarmFlag = POS_BLOCKED;
  switch (closingAlarms[weekday(now)].mode)
  {
  case ZEIT:
    if( (closingAlarms[weekday(now)].hour*60 + closingAlarms[weekday(now)].minute) > (hour(now)*60 + minute(now)) ) //Alarm noch nicht vergangen
    {
      alarmFlag = nextMove;
      setAlarm(weekday(now), closingAlarms[weekday(now)].hour, closingAlarms[weekday(now)].minute);
      esp_sleep_enable_ext1_wakeup(WAKEUP_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
    }

    else //Alarm schon vergangen, setze Alarm auf morgen 00:01 Uhr
    {
      alarmFlag = POS_BLOCKED;
      setAlarmTomorrow0();
      esp_sleep_enable_ext1_wakeup(WAKEUP_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
    }
    
    PREP_FOR_DEEP_SLEEP //TODO: Deep Sleep with alarm interrupt
    
    break;


  case LICHT:
    alarmFlag = POS_BLOCKED;
    setAlarmTomorrow0();
    esp_sleep_enable_ext1_wakeup(WAKEUP_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
    
    if(lastMove + t_delta_min > now+1) //Letzte Bewegung noch nicht lange genug her
    {
      LDRFlag = nextMove;
      esp_sleep_enable_timer_wakeup(S_TO_uS * (lastMove + t_delta_min - now) );
    } 
    else // Letzte Bewegung schon lange genug her um LDR zu aktivieren
    {
      LDRFlag = nextMove;
      activateLDR();
    }

    PREP_FOR_DEEP_SLEEP //TODO: Deep Sleep with alarm interrupt

    
    break;


  case LICHT_ZEIT:

    if( (closingAlarms[weekday(now)].hour*60 + closingAlarms[weekday(now)].minute) > (hour(now)*60 + minute(now)) ) //Alarm noch nicht vergangen
    {
      alarmFlag = nextMove;
      setAlarm(weekday(now), closingAlarms[weekday(now)].hour, closingAlarms[weekday(now)].minute);
      
      if(lastMove + t_delta_min > now+1) //Letzte Bewegung noch nicht lange genug her
      {
        LDRFlag = nextMove;
        esp_sleep_enable_timer_wakeup(S_TO_uS * (lastMove + t_delta_min - now) );
      } 
      else // Letzte Bewegung schon lange genug her um LDR zu aktivieren
      {
        LDRFlag = nextMove;
        activateLDR();
      }

    }

    else //Alarm schon vergangen, setze Alarm auf morgen 00:01 Uhr
    {
      alarmFlag = POS_BLOCKED;
      setAlarmTomorrow0();
      esp_sleep_enable_ext1_wakeup(WAKEUP_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
    }

      PREP_FOR_DEEP_SLEEP //TODO: Deep Sleep

    

    break;


  case NICHT:
    alarmFlag = POS_BLOCKED;
    setAlarmTomorrow0();
    break;
  }
}

void activateLDR(){
  //TODO: Implement activate Sensor
  esp_sleep_enable_timer_wakeup(S_TO_uS * t_sens);
}

void setAlarmTomorrow0()
{
  if(weekday(now) >= 6){
        setAlarm(0, 0, 1);
      }
      else
      {
        setAlarm(weekday(now)+1, 0, 1);
      }
}



void timeUpdate(){
  tmElements_t tmels;

  tmels.Minute = rtc.getDate();
  tmels.Month = rtc.getMonth();
  tmels.Year = rtc.getYear();
  tmels.Wday = rtc.getWeekday();

  tmels.Hour = rtc.getHours();
  tmels.Minute = rtc.getMinutes();
  tmels.Second = rtc.getSeconds();

  now = makeTime(tmels);
}


void IRAM_ATTR moveMotor(KLAPPENPOSITION pos)
{
  digitalWrite(LED, HIGH);
  while (doorPosition != pos)
  {
    pinMode(END_LOW, INPUT_PULLUP);
    pinMode(END_UP, INPUT_PULLUP);

    byte up = digitalRead(END_UP);
    byte low = digitalRead(END_LOW);
    if (up && !low)
    {
      doorPosition = POS_DOWN;
      digitalWrite(M_FWD, HIGH);
      digitalWrite(M_BACK, LOW);
      //Klappe ist unten oder
      //Klappe ist zu leicht
    }
    else if (up && low)
    { //Klappe hängt fest

      doorPosition = POS_BLOCKED;
      timerBlocked = timerBegin(BLOCKED_TIMER, t_err_open, true); //TODO: Implement Blocked Timer
    }
    else if (!up && low)
    {
      doorPosition = POS_UP;
      digitalWrite(M_FWD, LOW);
      digitalWrite(M_BACK, HIGH);
      //Klappe ist oben
    }
    else if (!up && !low)
    { //Klappe fährt grade  oder

      doorPosition = POS_DRIVING;
      timerMoving = timerBegin(MOVING_TIMER, t_err_open, true); //TODO: Implement Moving Timer
    }
  }
}

uint64_t GPIO_wake_up_reason()
{
  uint64_t GPIO_reason = esp_sleep_get_ext1_wakeup_status();
  //Serial.print("GPIO that triggered the wake up: GPIO ");
  GPIO_reason = (log(GPIO_reason)) / log(2);
  return GPIO_reason;
  //Serial.println(GPIO_reason);
}

uint16_t getLux()
{
  uint16_t lux;
  adc_ldr.attach(LDR_VAL);
  digitalWrite(LDR_EN, HIGH);
  lux = adc_ldr.readMiliVolts(); //TODO: LDR lux umsetzung implementieren

  return lux;
}

float getVolt()
{

  float mv;
  adc_vbatt.attach(V_BATT);
  digitalWrite(LDR_EN, HIGH);
  mv = (adc_vbatt.readVoltage() * (1.0 / 6.0)) - 300.0; //TODO: Volt umsetzung implementieren

  return mv;
}

void getValuesFromFlash()
{
  //TODO: Implement getValuesFromFlash
}

// TimerReturnVal checkLDR()
// {
//   switch (LDRTimer)
//   {
//   case NEXT_CLOSE:
//     int lux = getLux();
//     if (lux < nextLux)
//     {
//       thresholdCount++;
//       if (thresholdCount >= THRESHOLD_COUNT_MAX)
//       {
//         thresholdCount = 0;
//         LDRTimer = NO_TIMER;
//         return GO_DOWN;
//       }
//       else
//       {
//         thresholdCount++;
//         esp_sleep_enable_timer_wakeup(S_TO_uS * LDR_MEAS_SLEEP);
//         return SLEEP_SHORT;
//       }
//     }
//     else
//     {
//       if (thresholdCount <= 1)
//       {
//         thresholdCount = 0;
//         esp_sleep_enable_timer_wakeup(S_TO_uS * LDR_SLEEP);
//         LDRTimer = NEXT_CLOSE;
//         return SLEEP_LONG;
//       }
//       else
//       {
//         thresholdCount--;
//         esp_sleep_enable_timer_wakeup(S_TO_uS * LDR_MEAS_SLEEP);
//         return SLEEP_SHORT;
//       }
//     }
//     break;

//   case NEXT_OPEN:
//     int lux = getLux();
//     if (lux > nextLux)
//     {
//       thresholdCount++;
//       if (thresholdCount >= THRESHOLD_COUNT_MAX)
//       {
//         thresholdCount = 0;
//         LDRTimer = NO_TIMER;
//         return GO_UP;
//       }
//       else
//       {
//         thresholdCount++;
//         esp_sleep_enable_timer_wakeup(S_TO_uS * LDR_MEAS_SLEEP);
//         return SLEEP_SHORT;
//       }
//     }
//     else
//     {
//       if (thresholdCount <= 1)
//       {
//         thresholdCount = 0;
//         esp_sleep_enable_timer_wakeup(S_TO_uS * LDR_SLEEP);
//         LDRTimer = NEXT_OPEN;
//         return SLEEP_LONG;
//       }
//       else
//       {
//         thresholdCount--;
//         esp_sleep_enable_timer_wakeup(S_TO_uS * LDR_MEAS_SLEEP);
//         return SLEEP_SHORT;
//       }
//     }
//     break;

//   case NO_TIMER:
//     //TODO: No Timer Error Handling
//   }
// }




// KLAPPENPOSITION getAndDisableAlarm(){
//   //TODO: Alarm deaktivieren

//   return POS_UP;
// }

// void activateSensor(KLAPPENPOSITION nextAction){
//   //TODO: Sensor aktivieren
// }

// void eval_wakeup_reason()
// {
//   esp_sleep_wakeup_cause_t wakeup_reason;

//   wakeup_reason = esp_sleep_get_wakeup_cause();

//   switch (wakeup_reason){
//   case ESP_SLEEP_WAKEUP_EXT0:
//     Serial.println("Wakeup caused by external signal using RTC_IO");
//     break;
//   case ESP_SLEEP_WAKEUP_EXT1:
//     Serial.println("Wakeup caused by external signal using RTC_CNTL");
//     getValuesFromFlash();

//     TimerReturnVal nextAction = SLEEP_LONG;

//     switch (GPIO_wake_up_reason())
//     {
//     case CLK_INT:

//       nextAction = getAndDisableAlarm();

//       if(errorFlag){
//         //Klappe wurde nicht durch Sensor betätigt --> nächsten Alarm setzen
//         errorFlag = 0;
//         lightFlag = 0;
//       }
//       else if(lightFlag){
//         activateSensor(nextAction);
//         return;
//       }
//       else{
//         moveMotor(nextMove);
//       }
//       activateNextAlarm();
    
//     case SW_FWD:
//       //TODO: Implement Menu Timer, or Move Door UP
//       break;

//     case SW_BACK:
//       //TODO: Implement Menu Timer, or Move Door DOWN
//       break;
    
//     default:
//       LCD_ON
//       break;
//     }

//     break;
//   case ESP_SLEEP_WAKEUP_TIMER:
//     Serial.println("Wakeup caused by timer");
//     getValuesFromFlash();
    
    

//     TimerReturnVal nextAction = SLEEP_LONG;

//     switch(nextMode){
//       case LICHT_ZEIT:
//         if(nextMove = CLOSE){
//           if(timeFlag){
//             timeFlag = 0;
//             //TODO: Error handling, Timer kommt nach Zeit Flag
//             moveMotor(POS_DOWN);
//             setNextOpeningAlarm(DoW);
//           }
//           else
//           {
//             nextAction = checkLDR();
//           }
//         }
//         else
//         {
//           if(timeFlag){
//             nextAction = checkLDR();
//           }
//           else
//           {
//             //TODO: Error Handling, alarm ncoh nicht gekommen, trotzdem Timer
//           }
//         }
//         break;

//       case ZEIT:
//         //TODO: Error handling, Timer kommt obwohl Zeit Modus
//         break;

//       case LICHT:
//         nextAction = checkLDR();
//         break;

//       case NICHT:
//         //TODO: Error handling, Timer obwohl keine Aktion
//         break;

//     }
//     switch(nextAction){
//       case GO_UP:
//         moveMotor(POS_UP);
//         thresholdCount = 0;
//         timeFlag = 0;
//         setNextClosingAlarm(DoW);
//         break;

//       case GO_DOWN:
//         moveMotor(POS_DOWN);
//         thresholdCount = 0;
//         timeFlag = 0;
//         setNextOpeningAlarm(DoW);
//         break;

//       case SLEEP_LONG:
//         esp_deep_sleep_start();
//         break;

//       case SLEEP_SHORT:
//         esp_deep_sleep_start();
//         break;
//     }
//     break;

//   case ESP_SLEEP_WAKEUP_TOUCHPAD:
//     Serial.println("Wakeup caused by touchpad");
//     break;
//   case ESP_SLEEP_WAKEUP_ULP:
//     Serial.println("Wakeup caused by ULP program");
//     break;
//   default:
//     Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
//     break;
//   }
// }

void setup()
{
  pinMode(M_FWD, OUTPUT);
  pinMode(M_BACK, OUTPUT);
  pinMode(SW_EXIT, INPUT);
  pinMode(SW_FWD, INPUT);
  pinMode(SW_SELECT, INPUT);
  pinMode(SW_BACK, INPUT);
  pinMode(LDR_VAL, INPUT);
  pinMode(CLK_INT, INPUT);
  pinMode(V_BATT, INPUT);
  pinMode(LED, OUTPUT);

  pinMode(LDR_EN, OUTPUT);
  pinMode(LCD_EN, OUTPUT);
  pinMode(LCD_BL_EN, OUTPUT);
  pinMode(LED, OUTPUT);

  Wire.setPins(I2C_SDA, I2C_SCL);
  Wire.begin();

  if (rtc.begin() == false)
  {
    Serial.println("Device not found. Please check wiring. Freezing.");

    //TODO: Error handling
  }
  
  timeUpdate();
  delay(50);

  //print_wakeup_reason();
}


void loop()
{

  if (timeUpdate)
  {
    timeUpdate();
  }
  delay(50);

  // print_wakeup_reason();

  delay(100); //simulate a delay as if other tasks are running
}

//MENU STUFF:
char bufTime[] = "23:59";
char *const firstHourDigit PROGMEM = "012";
char *const allTimeDigit PROGMEM = "0123456789";
char *const firstMinuteDigit PROGMEM = "0123456";
char *const timeVal[] PROGMEM = {firstHourDigit, allTimeDigit, ":", firstMinuteDigit, allTimeDigit};
//callback funktionen
result doAlert(eventMask e, prompt &item);

result alert(menuOut &o, idleEvent e)
{
  if (e == idling)
  {

    o.setCursor(0, 0);
    o.print("Helligkeit");
    o.setCursor(0, 1);
    o.print(brightness);
  }
  return proceed;
}


result idleDeepSleepFunc(menuOut &o, idleEvent e)
{
  switch (e)
  {
  case idleStart:
    PREP_FOR_DEEP_SLEEP

    esp_deep_sleep_start();
  }
  return proceed;
}


result setOpeningAlarm(eventMask e)
{
  Serial.print("Event Mask Alarm: ");
  Serial.println(e);

  if (e == exitEvent)
  {
    Clock.checkIfAlarm(1);

    Serial.print("Setting Alarm and writing to Clock, monday Alarm hour: ");
    Serial.println(openingAlarms[0].hour);
    Clock.setA1Time(DoW, openingAlarms[0].hour, openingAlarms[0].minute, 0, 0x0, true, false, false);
  }
  else if (e == enterEvent)
  {
  }

  return proceed;
}

result showEvent(eventMask e, navNode &nav, prompt &item)
{
  Serial.print("event: ");
  Serial.println(e);
  return proceed;
}

result readBrightness(eventMask e, navNode &nav, prompt &item)
{
  digitalWrite(LDR_EN, HIGH);
  brightness = analogRead(LDR_VAL);
  digitalWrite(LDR_EN, LOW);
  return proceed;
}

int test = 55;

result setClockTime(eventMask e)
{
  Serial.print("Event Mask Time: ");
  Serial.println(e);

  if (e == exitEvent)
  {
    Serial.println("Setting Time and starting update");
    Clock.setHour(Hour);
    Clock.setMinute(Minute);
    Clock.setSecond(Second);
    timeUpdate = true;
  }
  else if (e == enterEvent)
  {
    Serial.println("Stopping update");
    timeUpdate = false;
  }

  return proceed;
}

result setClockDate(eventMask e)
{
  if (e == exitEvent)
  {
    Clock.setDate(Date);
    Clock.setMonth(Month);
    Clock.setYear(Year);
    timeUpdate = true;
  }
  else if (e == enterEvent)
  {
    timeUpdate = false;
  }

  return proceed;
}

result action1(eventMask e, navNode &nav, prompt &item)
{
  Serial.print("action1 event: ");
  Serial.print(e);
  Serial.println(", proceed menu");
  Serial.flush();
  return proceed;
}

result action2(eventMask e, navNode &nav, prompt &item)
{
  Serial.print("action2 event: ");
  Serial.print(e);
  Serial.print(", quiting menu.");
  Serial.flush();
  return quit;
}

/*extern menu mainMenu;
TOGGLE((mainMenu[1].enabled),togOp,"Op 2:",doNothing,noEvent,noStyle
  ,VALUE("Enabled",enabledStatus,doNothing,noEvent)
  ,VALUE("disabled",disabledStatus,doNothing,noEvent)
);*/

// char* constMEM hexDigit MEMMODE="0123456789ABCDEF";
// char* constMEM hexNr[] MEMMODE={"0","x",hexDigit,hexDigit};
// char buf1[]="0x11";

/*

MENU(mainMenu,"Hauptmenü",doNothing,noEvent,wrapStyle
  ,SUBMENU(resetMenu)
  ,OP("Aktuelle Helligkeit",doNothing,anyEvent)
  ,SUBMENU(openingModeMenu)
  ,SUBMENU(closingModeMenu)
  ,SUBMENU(timeMenu)
  ,SUBMENU(dateMenu)
  ,SUBMENU(languageMenu)

  ,EXIT("<Back")
);

  ,OP("Op1",action1,anyEvent)
  ,OP("Op2",action2,enterEvent)
  //,SUBMENU(togOp)
  ,FIELD(test,"Test","%",0,100,10,1,doNothing,noEvent,wrapStyle)
  ,SUBMENU(subMenu)
  ,SUBMENU(setLed)
  ,OP("LED On",myLedOn,enterEvent)
  ,OP("LED Off",myLedOff,enterEvent)
  ,SUBMENU(selMenu)
  ,SUBMENU(chooseMenu)
  ,OP("Alert test",doAlert,enterEvent)
  // ,EDIT("Hex",buf1,hexNr,doNothing,noEvent,noStyle)
  */
