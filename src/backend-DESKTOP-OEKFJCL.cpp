

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

bool timeUpdate = true;

byte lightFlag = 0;

char bufTime[] = "23:59";
char *const firstHourDigit PROGMEM = "012";
char *const allTimeDigit PROGMEM = "0123456789";
char *const firstMinuteDigit PROGMEM = "0123456";
char *const timeVal[] PROGMEM = {firstHourDigit, allTimeDigit, ":", firstMinuteDigit, allTimeDigit};

//devices
ESP32AnalogRead adc_ldr;
DS3231 Clock;
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

//callback funktionen
result doAlert(eventMask e, prompt &item);

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

void setNextOpeningAlarm(byte m_DoW)
{
  switch (openingAlarms[m_DoW].mode)
  {
  case ZEIT:
    Clock.setA1Time(DoW, openingAlarms[m_DoW].hour, openingAlarms[m_DoW].minute, 0, 0x0, true, false, false);
    break;

  case LICHT:
    esp_sleep_enable_timer_wakeup(S_TO_uS*LDR_SLEEP);
    break;

  case LICHT_ZEIT:
    Clock.setA1Time(DoW, openingAlarms[m_DoW].hour, openingAlarms[m_DoW].minute, 0, 0x0, true, false, false);
    break;

  case NICHT:
    if(m_DoW > 5){
      m_DoW = 0;
    }
    
    else{
      m_DoW = m_DoW + 1;
    }
    Clock.setA1Time(DoW, 0, 1, 0, 0x0, true, false, false);
    break;
  }
}

void setNextClosingAlarm(byte m_DoW)
{
  switch (closingAlarms[m_DoW].mode)
  {
  case ZEIT:
    Clock.setA2Time(DoW, closingAlarms[m_DoW].hour, closingAlarms[m_DoW].minute, 0x0, true, false, false);
    break;

  case LICHT:
    esp_sleep_enable_timer_wakeup(S_TO_uS*LDR_SLEEP);
    break;

  case LICHT_ZEIT:
    Clock.setA2Time(DoW, closingAlarms[m_DoW].hour, closingAlarms[m_DoW].minute, 0x0, true, false, false);
    break;

  case NICHT:
    if(m_DoW > 5){
      m_DoW = 0;
    }
    else{
      m_DoW = m_DoW + 1;
    }
    Clock.setA2Time(DoW, 0, 1, 0x0, true, false, false);
    break;
  } 
}

int16_t setAlarm(doorDayAlarm_t alarm)
{
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

result toggleDaily(eventMask e);

int ledCtrl = LOW;

result myLedOn()
{
  ledCtrl = HIGH;
  return proceed;
}
result myLedOff()
{
  ledCtrl = LOW;
  return proceed;
}

int dailySameVar = 0;

TOGGLE(dailySameVar, dailySame, "Gleich: ", toggleDaily, noEvent, noStyle //,doExit,enterEvent,noStyle
       ,
       VALUE("Ja", 1, toggleDaily, noEvent), VALUE("Nein", 0, toggleDaily, noEvent));

int selTest = 0;
SELECT(selTest, selMenu, "Select", doNothing, noEvent, noStyle, VALUE("Zero", 0, doNothing, noEvent), VALUE("One", 1, doNothing, noEvent), VALUE("Two", 2, doNothing, noEvent));

int chooseClosingModeVar = -1;
CHOOSE(chooseClosingModeVar, chooseClosingMode, "Modus", doNothing, noEvent, noStyle, VALUE("Tages-Modus", 1, doNothing, enterEvent), VALUE("Wochenende-Modus", 2, doNothing, enterEvent), VALUE("Immer gleich", -1, doNothing, noEvent));

//customizing a prompt look!
//by extending the prompt class
class altPrompt : public prompt
{
public:
  altPrompt(constMEM promptShadow &p) : prompt(p) {}
  Used printTo(navRoot &root, bool sel, menuOut &out, idx_t idx, idx_t len, idx_t) override
  {
    return out.printRaw(F("special prompt!"), len);
    ;
  }
};

altMENU(menu, timeMenu, "", setClockTime, anyEvent, noStyle, (systemStyles)(_asPad | _canNav | _menuData | _parentDraw), FIELD(Hour, "", ":", 0, 23, 1, 0, doNothing, noEvent, wrapStyle), FIELD(Minute, "", ":", 0, 59, 10, 1, doNothing, noEvent, wrapStyle), FIELD(Second, "", "", 0, 59, 10, 1, doNothing, noEvent, wrapStyle));

altMENU(menu, dateMenu, "", setClockDate, anyEvent, noStyle, (systemStyles)(_asPad | _canNav | _menuData | _parentDraw), FIELD(Date, "", ".", 0, 31, 1, 0, doNothing, noEvent, noStyle), FIELD(Month, "", ".", 0, 12, 1, 0, doNothing, noEvent, noStyle), FIELD(Year, "", "", 0, 99, 1, 0, doNothing, noEvent, noStyle));

altMENU(menu, openingAlarmMenu, "", setOpeningAlarm, anyEvent, noStyle, (systemStyles)(_asPad | _canNav | _menuData | _parentDraw), FIELD(openingAlarms[0].hour, "", ":", 0, 23, 1, 0, doNothing, noEvent, wrapStyle), FIELD(openingAlarms[0].minute, "", "", 0, 59, 10, 1, doNothing, noEvent, wrapStyle));

MENU(subMenu, "Sub-Menu", showEvent, anyEvent, noStyle, OP("Sub1", showEvent, anyEvent), OP("Sub2", showEvent, anyEvent), OP("Sub3", showEvent, anyEvent), altOP(altPrompt, "", showEvent, anyEvent), EXIT("<Back"));

MENU(closingMode, "Schliess Modus", showEvent, anyEvent, noStyle, SUBMENU(chooseClosingMode), SUBMENU(dailySame), EDIT("Montag: ", bufTime, timeVal, doNothing, noEvent, noStyle), EDIT("Dienstag", bufTime, timeVal, doNothing, noEvent, noStyle), altOP(altPrompt, "", showEvent, anyEvent), EXIT("<Back"));

/*extern menu mainMenu;
TOGGLE((mainMenu[1].enabled),togOp,"Op 2:",doNothing,noEvent,noStyle
  ,VALUE("Enabled",enabledStatus,doNothing,noEvent)
  ,VALUE("disabled",disabledStatus,doNothing,noEvent)
);*/

// char* constMEM hexDigit MEMMODE="0123456789ABCDEF";
// char* constMEM hexNr[] MEMMODE={"0","x",hexDigit,hexDigit};
// char buf1[]="0x11";

MENU(mainMenu, "Main menu", doNothing, noEvent, wrapStyle, SUBMENU(timeMenu), SUBMENU(dateMenu), SUBMENU(openingAlarmMenu)

     //,EDIT("Time", bufTime, timeVal, doNothing, noEvent, noStyle)
     //,OP("\xEF" "ffnungs Modus",action1,anyEvent)
     ,
     SUBMENU(closingMode), FIELD(brightness, "LUX:", "", 0, 59, 10, 1, doNothing, noEvent, wrapStyle)

                               ,
     OP("Datum ", action1, enterEvent), OP("Sprache ", action1, enterEvent), OP("Reset", doNothing, enterEvent), EXIT("<Zurueck")
     //,FIELD(brightness,"Helligkeit","lux",0,4095,100,10,readBrightness,noEvent,wrapStyle)

     //,SUBMENU(togOp)

     //,SUBMENU(subMenu)
     //,SUBMENU(setLed)
     //,OP("LED On",myLedOn,enterEvent)
     //,OP("LED Off",myLedOff,enterEvent)
     //,SUBMENU(selMenu)
     //,SUBMENU(chooseMenu)
     // ,EDIT("Hex",buf1,hexNr,doNothing,noEvent,noStyle)

);

result toggleDaily(eventMask e)
{

  if (dailySameVar)
  {
    closingMode[2].disable();
    Serial.println("disabling option");
  }
  else
  {
    closingMode[2].enable();
  }
  return proceed;
}

//const panel panels[] MEMMODE={{0,0,16,2}};
//navNode* nodes[sizeof(panels)/sizeof(panel)];
//panelsList pList(panels,nodes,1);

//Button inputs
keyMap joystickBtn_map[] = {
    {SW_SELECT, defaultNavCodes[enterCmd].ch},
    {SW_BACK, defaultNavCodes[leftCmd].ch},
    {SW_FWD, defaultNavCodes[rightCmd].ch},
    {SW_EXIT, defaultNavCodes[escCmd].ch},

};

softKeyIn<4> joystickBtns(joystickBtn_map);

serialIn serial(Serial);
menuIn *inputsList[] = {&joystickBtns, &serial};
chainStream<2> in(inputsList); //3 is the number of inputs

//LCD output
MENU_OUTPUTS(out, MAX_DEPTH, LIQUIDCRYSTAL_OUT(lcd, {0, 0, 16, 2}), NONE);

//the navigation root object
NAVROOT(nav, mainMenu, MAX_DEPTH, in, out);

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

result doAlert(eventMask e, prompt &item)
{
  nav.idleOn(alert);
  return proceed;
}

result doResetAlert(eventMask e, prompt &item)
{
  nav.idleOn(alert);
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

void IRAM_ATTR ISR_Motor()
{
  digitalWrite(LED, HIGH);
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
  {
    door_position = POS_BLOCKED;
    //Klappe hängt fest
  }
  else if (!up && low)
  {
    door_position = POS_UP;
    digitalWrite(M_FWD, LOW);
    digitalWrite(M_BACK, HIGH);
    //Klappe ist oben
  }
  else if (!up && !low)
  {
    door_position = POS_DRIVING;
    //Klappe fährt grade  oder
  }
}

void print_GPIO_wake_up()
{
  uint64_t GPIO_reason = esp_sleep_get_ext1_wakeup_status();
  //Serial.print("GPIO that triggered the wake up: GPIO ");
  GPIO_reason = (log(GPIO_reason)) / log(2);
  if (GPIO_reason == SW_FWD)
  {

    ISR_Motor();
  }
  //Serial.println(GPIO_reason);
}

void print_wakeup_reason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
  case ESP_SLEEP_WAKEUP_EXT0:
    Serial.println("Wakeup caused by external signal using RTC_IO");
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println("Wakeup caused by external signal using RTC_CNTL");
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("Wakeup caused by timer");
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
  pinMode(LED, OUTPUT);

  pinMode(LDR_EN, OUTPUT);
  pinMode(LCD_EN, OUTPUT);
  pinMode(LCD_BL_EN, OUTPUT);
  pinMode(LED, OUTPUT);

  digitalWrite(LDR_EN, HIGH);

  Serial.begin(115200);
  Wire.begin();

  print_wakeup_reason();
  print_GPIO_wake_up();

  Clock.enableOscillator(true, false, 0);
  Clock.enable32kHz(false);
  Clock.setClockMode(false);

  LCD_ON;

  delay(200);

  lcd.begin(16, 2);
  nav.timeOut = 2;
  nav.idleTask = idleDeepSleepFunc; //point a function to be used when menu is suspended

  mainMenu[4].enabled = disabledStatus;
  timeMenu[2].enabled = disabledStatus;

  Clock.checkIfAlarm(1);
  Clock.checkIfAlarm(2);
  Clock.turnOffAlarm(2);
  Clock.turnOffAlarm(1);

  nav.showTitle = false;
  lcd.setCursor(4, 0);
  lcd.print("Tettus");
  lcd.setCursor(3, 1);
  lcd.print("Eggvault");
  delay(1000);

  attachInterrupt(SW_FWD, ISR_Motor, RISING);
  esp_sleep_enable_ext1_wakeup(WAKEUP_PINMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
}

void loop()
{

  brightness = analogRead(LDR_VAL);

  nav.poll();
  digitalWrite(LED, ledCtrl);

  if (timeUpdate)
  {
    Date = Clock.getDate();
    Month = Clock.getMonth(century);
    Year = Clock.getYear();
    DoW = Clock.getDoW();

    Hour = Clock.getHour(h12Flag, pmFlag);
    Minute = Clock.getMinute();
    Second = Clock.getSecond();
  }
  delay(50);

  /*
  //Serial.print("Monday opening hour: ");
  //Serial.println(openingAlarms[0].hour);
  Serial.print("control byte: ");
  Serial.println(Clock.readControlByte(true), BIN);
  Serial.print("interrupt: ");
  Serial.println(digitalRead(CLK_INT));

  delay(50);

  
  Serial.print("SW_BACK: ");
  Serial.println(digitalRead(SW_BACK));
 delay(200);
  Serial.print("SW_SEL: ");
  Serial.println(digitalRead(SW_SELECT));
 delay(200);
 Serial.print("SW_EXIT: ");
  Serial.println(digitalRead(SW_EXIT));
 delay(200);
 Serial.print("SW_FWD: ");
  Serial.println(digitalRead(SW_FWD));

  */

  delay(100); //simulate a delay as if other tasks are running
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
