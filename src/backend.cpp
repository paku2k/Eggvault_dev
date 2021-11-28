

#include "main.h"
#include "global.h"
#include <math.h>

//globals
int brightness = -1;
char *brString;

doorDayAlarm_t openingAlarms[7];
doorDayAlarm_t closingAlarms[7];

byte Year;
byte Month;
byte Date;
byte DoW;
byte Hour;
byte Minute;
byte Second;

KLAPPENPOSITION door_position, door_goal;

bool century = false;
bool h12Flag;
bool pmFlag;
byte alarmDay, alarmHour, alarmMinute, alarmSecond, alarmBits;
bool alarmDy, alarmH12Flag, alarmPmFlag;


byte lightFlag = 0; //bei nächstem aufwachen den Sensor nutzen
byte errorFlag = 0; //Notalarm, wenn Klappe nicht durch Sensor geöffnet
byte timeFlag = 0; //Alarm für LichtZeit Modus schon gekommen
byte openNext = 0;  //als nächstes öffnen
byte LDRTimer = NO_TIMER;
uint16_t nextLux = 0;
byte thresholdCount = 0;
openingMode nextMode = NICHT;
KLAPPENPOSITION nextMove = POS_DOWN;

volatile int count; //timer variable
hw_timer_t *timerBlocked = NULL;
hw_timer_t *timerMoving = NULL;



//devices
ESP32AnalogRead adc_ldr;
ESP32AnalogRead adc_vbatt;
DS3231 Clock;
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);


void setNextOpeningAlarm(byte m_DoW){
nextMove = POS_UP;
nextLux = openingAlarms[m_DoW].lux;
  switch (openingAlarms[m_DoW].mode)
  {
  case ZEIT:
    lightFlag = 0;
    errorFlag = 0;
    Clock.setA1Time(m_DoW, openingAlarms[m_DoW].hour, openingAlarms[m_DoW].minute, 0, 0x0, true, false, false);
    PREP_FOR_DEEP_SLEEP //TODO: Deep Sleep

    break;


  case LICHT:
    lightFlag = 1;
    if(m_DoW == DoW){ //Heute?
      //Kann eigentlich nicht sein, da Öffnungsalarm immer um 0:01 des Tages aktiv sein 
      activateLDR(NEXT_OPEN, LICHT);
    }
    else{
      //ein anderer Tag --> Alarm wird um 0:01 aktiv und macht den Lichtsensor an
      lightFlag = 1;
      errorFlag = 0;
      Clock.setA1Time(m_DoW, openingAlarms[m_DoW].hour, openingAlarms[m_DoW].minute, 0, 0x0, true, false, false); //TODO: Öffnungsalarm immer mit 0:01 eintragen bei Licht
      PREP_FOR_DEEP_SLEEP //TODO: Deep Sleep
    }
    
    break;


  case LICHT_ZEIT:
    lightFlag = 1;
    errorFlag = 0;
    Clock.setA1Time(m_DoW, openingAlarms[m_DoW].hour, openingAlarms[m_DoW].minute, 0, 0x0, true, false, false);
    PREP_FOR_DEEP_SLEEP //TODO: Deep Sleep

    break;


  case NICHT:
    break;
  }
}

void setNextClosingAlarm(byte m_DoW){
uint16_t now = DoW*24*60+Hour*60+Minute;
uint16_t nextClose = m_DoW*24*60+closingAlarms[(m_DoW)].hour*60+closingAlarms[(m_DoW)].minute;

nextMove = POS_UP;
nextLux = closingAlarms[m_DoW].lux;

  switch (closingAlarms[m_DoW].mode)
  {
  case ZEIT:
    lightFlag = 0;
    errorFlag = 0;
    Clock.setA2Time(m_DoW, closingAlarms[m_DoW].hour, closingAlarms[m_DoW].minute, 0x0, true, false, false);
    PREP_FOR_DEEP_SLEEP //TODO: Deep Sleep

    break;

  case LICHT_ZEIT:
  case LICHT:
    lightFlag = 1;
    errorFlag = 0;
    if(m_DoW == DoW){ //Heute?
      //Klappe wurde grade geöffnet --> aufgewacht durch Öffnung der Klappe --> Sensor wird mit Verzögerung aktiviert

      if(nextClose < now + 60){
        //Späteste Schließung in den nächsten 59 min
        if(closingAlarms[m_DoW].mode == LICHT_ZEIT){
          Clock.setA2Time(m_DoW, closingAlarms[m_DoW].hour, closingAlarms[m_DoW].minute, 0x0, true, false, false);
          activateLDR(NEXT_CLOSE, LICHT_ZEIT);
        }
        else{
          activateLDR(NEXT_CLOSE, LICHT);
        }
      }
      else{
        //Späteste Schließung heute später als in 60min
        if(Hour == 23){
          if(DoW == 6){
            Clock.setA2Time(0, 0, Minute, 0x0, true, false, false);
          }
          else{
            Clock.setA2Time(DoW+1, 0, Minute, 0x0, true, false, false);
          }
        }
        else{
          Clock.setA2Time(DoW, Hour+1, Minute, 0x0, true, false, false);
        }
        
        PREP_FOR_DEEP_SLEEP //TODO: Deep Sleep
      }
    }
    else{
      //ein anderer Tag --> Alarm wird um 23:59 aktiv und schließt die Klappe spätestens
      //TODO: Schließalarm immer bei 23:59 Uhr eintragen bei licht
      lightFlag = 1;
      errorFlag = 0;
      if(closingAlarms[m_DoW].hour<12){
        //Vor 12 Uhr
        if(closingAlarms[m_DoW].hour<2){
          //Vor 2 Uhr
          Clock.setA2Time(m_DoW, 0, 2, 0x0, true, false, false);
        }
        //Zwischen 2 und 12 Uhr
        Clock.setA2Time(m_DoW, closingAlarms[m_DoW].hour-2, closingAlarms[m_DoW].minute, 0x0, true, false, false); //Alarm auf 2 Stunden vor der Schließzeit setzen
      }
      else{
        //nach 12 Uhr --> Alarm auf 12 Uhr setzen
        Clock.setA2Time(m_DoW, 12, 0, 0x0, true, false, false); 
      }
      PREP_FOR_DEEP_SLEEP //TODO: Deep Sleep
    }
    
    break;

  case NICHT:
    break;
  }
}

void activateLDR(TimerLogic nextTimer, openingMode modus){
//TODO: Implement activate Sensor

}

void activateNextAlarm()
{
  int i = 0;
  byte m_DoW = DoW;
  uint16_t now = Hour*60+Minute;
  doorDayAlarm_t nextAlarm;

  TimerLogic actAlarm = NO_TIMER;

  while(i<=6){
    uint16_t nextClose = i*24*60+closingAlarms[(m_DoW+i)%7].hour*60+closingAlarms[(m_DoW+i)%7].minute;
    uint16_t nextOpen  = i*24*60+openingAlarms[(m_DoW+i)%7].hour*60+openingAlarms[(m_DoW+i)%7].minute;
    

    if(nextClose > now){
      //Schließen kommt noch
      if(nextOpen > now){
        //Öffnen kommt auch noch
        if(nextOpen > nextClose){
          //Als nächstes Schließen!
          if(closingAlarms[(m_DoW+i)%7].mode == NICHT){
            actAlarm = NO_TIMER;
          }
          else{
            actAlarm = NEXT_CLOSE;
          }
        }
        else{
          //Als nächstes Öffnen!
          if(openingAlarms[(m_DoW+i)%7].mode == NICHT){
            actAlarm = NO_TIMER;
          }
          else{
            actAlarm = NEXT_OPEN;
          }
        }
      }
      else{
        //Als nächstes Schließen!
        if(closingAlarms[(m_DoW+i)%7].mode == NICHT){
            actAlarm = NO_TIMER;
          }
          else{
            actAlarm = NEXT_CLOSE;
          }
      }
    }
    else{
      //Schließen war schon
      if(nextOpen > now){
        //Als nächstes Öffnen!
        if(openingAlarms[(m_DoW+i)%7].mode == NICHT){
            actAlarm = NO_TIMER;
          }
          else{
            actAlarm = NEXT_OPEN;
      }
      else{
        //Beides war schon
        actAlarm = NO_TIMER;
      }
    }
    if(actAlarm==NO_TIMER){
      i++;
    }
    
    if(actAlarm==NEXT_OPEN){
      setNextOpeningAlarm((m_DoW+i)%7);
      break;
    }

    if(actAlarm==NEXT_CLOSE){
      setNextClosingAlarm((m_DoW+i)%7);
      break;
    }
        
  }

  if(actAlarm == NO_TIMER){
    //Kein Alarm mehr gesetzt
  }

  

  //hhh
  switch (closingAlarms[m_DoW].mode)
  {
  case ZEIT:
    Clock.setA2Time(DoW, closingAlarms[m_DoW].hour, closingAlarms[m_DoW].minute, 0x0, true, false, false);
    break;

  case LICHT:
    if (openingAlarms[m_DoW].mode == NICHT)
    {
      Clock.setA2Time(DoW, 12, 1, 0x0, true, false, false);
    }
    //TODO: Fallunterscheidung öffnen

    esp_sleep_enable_timer_wakeup(S_TO_uS * LDR_SLEEP);
    break;

  case LICHT_ZEIT:
    Clock.setA2Time(DoW, closingAlarms[m_DoW].hour, closingAlarms[m_DoW].minute, 0x0, true, false, false);
    break;

  case NICHT:
    if (m_DoW > 5)
    {
      m_DoW = 0;
    }
    else
    {
      m_DoW = m_DoW + 1;
    }
    Clock.setA2Time(DoW, 0, 2, 0x0, true, false, false);
    break;
  }
}

void timeUpdate(){
  Date = Clock.getDate();
  Month = Clock.getMonth(century);
  Year = Clock.getYear();
  DoW = Clock.getDoW();

  Hour = Clock.getHour(h12Flag, pmFlag);
  Minute = Clock.getMinute();
  Second = Clock.getSecond();
}

int16_t setAlarm(doorDayAlarm_t alarm)
{
}

void IRAM_ATTR moveMotor(KLAPPENPOSITION pos)
{
  digitalWrite(LED, HIGH);
  while (door_position != pos)
  {
    pinMode(END_LOW, INPUT_PULLUP);
    pinMode(END_UP, INPUT_PULLUP);

    byte up = digitalRead(END_UP);
    byte low = digitalRead(END_LOW);
    if (up && !low)
    {
      door_position = POS_DOWN;
      digitalWrite(M_FWD, HIGH);
      digitalWrite(M_BACK, LOW);
      //Klappe ist unten oder
      //Klappe ist zu leicht
    }
    else if (up && low)
    { //Klappe hängt fest

      door_position = POS_BLOCKED;
      timerBlocked = timerBegin(BLOCKED_TIMER, 80, true); //TODO: Implement Blocked Timer
    }
    else if (!up && low)
    {
      door_position = POS_UP;
      digitalWrite(M_FWD, LOW);
      digitalWrite(M_BACK, HIGH);
      //Klappe ist oben
    }
    else if (!up && !low)
    { //Klappe fährt grade  oder

      door_position = POS_DRIVING;
      timerMoving = timerBegin(MOVING_TIMER, 80, true); //TODO: Implement Moving Timer
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
  mv = (adc_vbatt.readVoltage() * (1.0 / 6.0)) - 0.3; //TODO: Volt umsetzung implementieren

  return mv;
}

void getValuesFromFlash()
{
  //TODO: Implement getValuesFromFlash
}

TimerReturnVal checkLDR()
{
  switch (LDRTimer)
  {
  case NEXT_CLOSE:
    int lux = getLux();
    if (lux < nextLux)
    {
      thresholdCount++;
      if (thresholdCount >= THRESHOLD_COUNT_MAX)
      {
        thresholdCount = 0;
        LDRTimer = NO_TIMER;
        return GO_DOWN;
      }
      else
      {
        thresholdCount++;
        esp_sleep_enable_timer_wakeup(S_TO_uS * LDR_MEAS_SLEEP);
        return SLEEP_SHORT;
      }
    }
    else
    {
      if (thresholdCount <= 1)
      {
        thresholdCount = 0;
        esp_sleep_enable_timer_wakeup(S_TO_uS * LDR_SLEEP);
        LDRTimer = NEXT_CLOSE;
        return SLEEP_LONG;
      }
      else
      {
        thresholdCount--;
        esp_sleep_enable_timer_wakeup(S_TO_uS * LDR_MEAS_SLEEP);
        return SLEEP_SHORT;
      }
    }
    break;

  case NEXT_OPEN:
    int lux = getLux();
    if (lux > nextLux)
    {
      thresholdCount++;
      if (thresholdCount >= THRESHOLD_COUNT_MAX)
      {
        thresholdCount = 0;
        LDRTimer = NO_TIMER;
        return GO_UP;
      }
      else
      {
        thresholdCount++;
        esp_sleep_enable_timer_wakeup(S_TO_uS * LDR_MEAS_SLEEP);
        return SLEEP_SHORT;
      }
    }
    else
    {
      if (thresholdCount <= 1)
      {
        thresholdCount = 0;
        esp_sleep_enable_timer_wakeup(S_TO_uS * LDR_SLEEP);
        LDRTimer = NEXT_OPEN;
        return SLEEP_LONG;
      }
      else
      {
        thresholdCount--;
        esp_sleep_enable_timer_wakeup(S_TO_uS * LDR_MEAS_SLEEP);
        return SLEEP_SHORT;
      }
    }
    break;

  case NO_TIMER:
    //TODO: No Timer Error Handling
  }
}

KLAPPENPOSITION getAndDisableAlarm(){
  //TODO: Alarm deaktivieren

  return OPEN;
}

void activateSensor(KLAPPENPOSITION nextAction){
  //TODO: Sensor aktivieren
}

void eval_wakeup_reason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason){
  case ESP_SLEEP_WAKEUP_EXT0:
    Serial.println("Wakeup caused by external signal using RTC_IO");
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println("Wakeup caused by external signal using RTC_CNTL");
    getValuesFromFlash();

    TimerReturnVal nextAction = SLEEP_LONG;

    switch (GPIO_wake_up_reason())
    {
    case CLK_INT:

      nextAction = getAndDisableAlarm();

      if(errorFlag){
        //Klappe wurde nicht durch Sensor betätigt --> nächsten Alarm setzen
        errorFlag = 0;
        lightFlag = 0;
      }
      else if(lightFlag){
        activateSensor(nextAction);
        return;
      }
      else{
        moveMotor(nextMove);
      }
      activateNextAlarm();
    
    case SW_FWD:
      //TODO: Implement Menu Timer, or Move Door UP
      break;

    case SW_BACK:
      //TODO: Implement Menu Timer, or Move Door DOWN
      break;
    
    default:
      LCD_ON
      break;
    }

    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("Wakeup caused by timer");
    getValuesFromFlash();
    
    

    TimerReturnVal nextAction = SLEEP_LONG;

    switch(nextMode){
      case LICHT_ZEIT:
        if(nextMove = CLOSE){
          if(timeFlag){
            timeFlag = 0;
            //TODO: Error handling, Timer kommt nach Zeit Flag
            moveMotor(POS_DOWN);
            setNextOpeningAlarm(DoW);
          }
          else
          {
            nextAction = checkLDR();
          }
        }
        else
        {
          if(timeFlag){
            nextAction = checkLDR();
          }
          else
          {
            //TODO: Error Handling, alarm ncoh nicht gekommen, trotzdem Timer
          }
        }
        break;

      case ZEIT:
        //TODO: Error handling, Timer kommt obwohl Zeit Modus
        break;

      case LICHT:
        nextAction = checkLDR();
        break;

      case NICHT:
        //TODO: Error handling, Timer obwohl keine Aktion
        break;

    }
    switch(nextAction){
      case GO_UP:
        moveMotor(POS_UP);
        thresholdCount = 0;
        timeFlag = 0;
        setNextClosingAlarm(DoW);
        break;

      case GO_DOWN:
        moveMotor(POS_DOWN);
        thresholdCount = 0;
        timeFlag = 0;
        setNextOpeningAlarm(DoW);
        break;

      case SLEEP_LONG:
        esp_deep_sleep_start();
        break;

      case SLEEP_SHORT:
        esp_deep_sleep_start();
        break;
    }
    break;

  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Serial.println("Wakeup caused by touchpad");
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    Serial.println("Wakeup caused by ULP program");
    break;
  default:
    Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
    break;
  }
}

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

  Clock.enableOscillator(true, false, 0);
  Clock.enable32kHz(false);
  Clock.setClockMode(false);

  Date = Clock.getDate();
  Month = Clock.getMonth(century);
  Year = Clock.getYear();
  DoW = Clock.getDoW();

  Hour = Clock.getHour(h12Flag, pmFlag);
  Minute = Clock.getMinute();
  Second = Clock.getSecond();

  delay(50);

  print_wakeup_reason();
}


void loop()
{

  if (timeUpdate)
  {
    timeUpdate();
  }
  delay(50);

  print_wakeup_reason();

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
