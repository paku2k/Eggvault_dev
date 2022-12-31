
#include "backend.h"
#include "global.h"
#include <math.h>

// globals
int brightness = -1;
char *brString;

doorDayAlarm_t openingAlarms[7];
doorDayAlarm_t closingAlarms[7];
const char *keyOpenAlarms = "open_alarm";
const char *keyCloseAlarms = "close_alarm";
const char *keyLDRFlag = "LDR_flag";
const char *keyAlarmFlag = "alarm_flag";
const char *keyLastMove = "last_move";
const char *keyLastMovePosition = "last_move_Position";

// uint8_t luxMap[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
uint32_t voltMap[10] = {300, 600, 900, 1200, 1500, 1800, 2100, 2400, 2700, 3000};

int16_t Year = 2020;
int8_t Month;
int8_t Date;
int8_t DoW; // Tag 1 ist Sonntag
int8_t Hour;
int8_t Minute;
int8_t Second;

KLAPPENPOSITION doorPosition, doorGoal, lastDoorPosition;
KLAPPENRICHTUNG doorDirection;

// Alarm Variablen
bool century = false;
bool h12Flag;
bool pmFlag;
byte alarmDay, alarmHour, alarmMinute, alarmSecond, alarmBits;
bool alarmDy, alarmH12Flag, alarmPmFlag;
ERROR_CODE errorFlag;

KLAPPENPOSITION LDRFlag = POS_DOWN; // bei nächstem aufwachen den Sensor nutzen
// byte errorFlag = 0; //Notalarm, wenn Klappe nicht durch Sensor geöffnet
// byte timeFlag = 0; //Alarm für LichtZeit Modus schon gekommen
KLAPPENPOSITION alarmFlag = POS_DOWN; // Alarm kommt für Öffnen oder Schließen
// byte LDRTimer = NO_TIMER;
uint8_t nextLux;
uint8_t ramCounter;
byte blinkFactor = 8;

int dammerungsverzogerung = 600; // TODO: Dämmerungsverzögerung implementieren
byte lux_debounce_number = 3;    // < 16 !!!! wie oft muss die Lichtschwelle erreicht sein, bis die Klappe sich bewegt DARF NICHT GRÖßER ALS 16 SEIN
int lux_debounce_time = 20;      // Zeit in sekunden zwischen zwei positiven Lichtprüfungen
int t_delta_min = 600;           // Zeit in Sekunden die zwischen zwei betätigungen per Licht vergangen sein müssen
int t_sens = 120;                // Zeit in Sekunden, die zwischen zwei Sensorprüfungen vergeht

int t_err_open = 180;  // Maximale Zeit zum Öffnen
int t_err_close = 150; // Maximale Zeit zum Schließen
int t_err_block = 30;  // Maximale Zeit, welche die Klappe blockieren darf

openingMode nextMode = NICHT;
KLAPPENPOSITION nextMove = POS_DOWN;

const byte weekdays[7] = {
    SUNDAY,
    MONDAY,
    TUESDAY,
    WEDNESDAY,
    THURSDAY,
    FRIDAY,
    SATURDAY,

};

time_t lastMove, t_now;

volatile int count; // timer variable
hw_timer_t *timerBlocked = NULL;
hw_timer_t *timerMoving = NULL;

// devices
ESP32AnalogRead adc_ldr;
ESP32AnalogRead adc_vbatt;
RV8803 rtc;
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
Preferences memory;

String MenuItems[8][3] = {
    // MenuItems[itemID][Language]
    {"\xEF"
     "ffnungsmodus",
     "Opening mode", "blub"},
    {"Schlie"
     "\xE2"
     "modus",
     "Closing mode", "blub"},
    {"Uhrzeit", "Time", "blub"},
    {"Datum", "Date", "blub"},
    {"Erweitert", "advanced", "blub"},
    {"Sprache", "language", "blub"},
    {"Reset", "reset", "blub"},
    {"EXIT", "EXIT", "EXIT"},
};

String InitItems[3][3] = {
    // MenuItems[itemID][Language]
    {"   Willkommen   ",
     "    Welcome     ", "NOX"},
    {"Taste dr"
     "\xF5"
     "cken  "
     "\x7F",
     "Press any key  "
     "\x7F",
     "blub"},
    {"Ersteinstellung"
     "\x7F",
     "Initial setup  "
     "\x7F",
     "blub"},
};

String MenuItemsSpecial[3][3] = {
    // MenuItems[itemID][Language]
    {"Besch"
     "\xE1"
     "tigt",
     "Busy", "blub"},
    {"N"
     "\xE1"
     "chste Alarme:",
     "Upcoming Alarms:", "blub"},
    {"NOX H"
     "\xF5"
     "hnerklappe",
     "NOX Chickendoor", "blub"},

};

String MenuItemsDays[7][3] = {
    // MenuItems[itemID][Language]
    {"So", "Su", "Di"},
    {"Mo", "Mo", "Lu"},
    {"Di", "Tu", "Ma"},
    {"Mi", "We", "Me"},
    {"Do", "Th", "Je"},
    {"Fr", "Fr", "Ve"},
    {"Sa", "Sa", "Sa"},
};

/*DE
#define M_INV 0
#define o_INV 1
#define D_INV 2
#define i_INV 3
#define F_INV 4
#define r_INV 5
#define S_INV 6
#define a_INV 7

EN


FR

*/

int MenuItemsDaysInv[7][3][2]{
    // MenuItems[itemID][Language]
    {{6, 1}, {6, 1}, {6, 1}},
    {{0, 1}, {6, 1}, {6, 1}},
    {{2, 3}, {6, 1}, {6, 1}},
    {{0, 3}, {6, 1}, {6, 1}},
    {{2, 1}, {6, 1}, {6, 1}},
    {{4, 5}, {6, 1}, {6, 1}},
    {{6, 7}, {6, 1}, {6, 1}},
};

// TODO: Multilanguage inverse characters

String MenuItemsMode[9][3] = {
    // MenuItems[itemID][Language]
    {"LUX", "LUX", "blub"},
    {"LUX+ZEIT", "LUX+TIME", "blub"},
    {"ZEIT", "TIME", "blub"},
    {"MANUELL", "MANUAL", "blub"},
    {"Lichtschwelle", "LUX threshold", "blub"},
    {"Aktuell", "Current", "blub"},
    {"\xEF"
     "ffnungszeit",
     "latest opening", "blub"},
    {"Schlie"
     "\xE2"
     "zeit",
     "earliest closing", "blub"},
    {"GESPEICHERT!", "SAVED!", "blub"},
};

template <typename T, size_t NumberOfSizeX, size_t NumberOfSizeY>
size_t MenuItemsSize(T (&)[NumberOfSizeX][NumberOfSizeY]) { return NumberOfSizeX; }
int numberOfMenuItems = MenuItemsSize(MenuItems) - 1;
int currentMenuItem = 0;
int previousMenuItem = 1;
byte button_flag = 0;

unsigned long previousMillis = millis();
unsigned long lastPress = millis();
unsigned long blinkZero = millis();
unsigned long longPressStart = millis();
unsigned long moveZero = millis();
unsigned long blockZero = millis();

byte blinkCount;

int F_init_alarm = 0;
int F_init_NOX = 0;
int F_init_flag = 0;
int F_init_last_move = 0;

byte blink = 0;
byte manualFlag = 0; // TODO: implement manual Flag

// ================================================
// ============== Statusvariablen: ================
// ================================================

uint8_t maxDayThisMonth = 31;

Language lang = DEUTSCH;
byte days_o = WEEK;
byte days_c = WEEK;
byte *days;

int cursor_tag = MON;
bool showNextOpen = true;

byte mode_o = LICHT_ZEIT;
byte mode_c = LICHT_ZEIT;
byte *mode;

byte lux_o = 0;
byte lux_c = 0;
byte *lux;

int8_t hour_o = 0;
int8_t hour_c = 0;
int8_t *hour_set;

int8_t minute_o = 0;
int8_t minute_c = 0;
int8_t *minute_set;

doorDayAlarm_t *nextAlarm;

Modeset modeset = MODESET_OPENING;
Setmode timeSetMode = SETNOTHING;

byte day_bitmask = 0b0000000; // SO_MO_DI_MI_DO_FR_SA_

int menu_id = 100;
int menuActive = 0; // TODO: implement menu active
bool menuRefreshFlag = true;
bool longPressFlag = false;

void saveAlarmValues();
void moveMotor(KLAPPENPOSITION pos);
void statusabfrage();
void saveTimeSetting();

// ================================================================================================
// ================================================================================================
// ================================================================================================

// ================================================================================================
// ========================================== FRONTEND ============================================
// ================================================================================================

// ================================================================================================
// ================================================================================================
// ================================================================================================

void timeUpdate()
{
  // Serial.println("Getting current time");
  rtc.updateTime();

  Hour = rtc.getHours();
  Minute = rtc.getMinutes();
  Second = rtc.getSeconds();

  makeLocalTime();
  // TODO: check DoW conversion / automatic setting --> tmels makes its own weekday. That one is correct. Sunday = 1 Saturday = 7
  // TODO: implement init time setting (wenn init = 0)
}

void dateUpdate()
{
  Date = rtc.getDate();
  Month = rtc.getMonth();
  Year = rtc.getYear();
  DoW = rtc.getWeekday() + 1;

  makeLocalTime();
}

void makeLocalTime()
{
  tmElements_t tmels;

  tmels.Day = Date;
  tmels.Month = Month;
  tmels.Year = Year - 1970;
  tmels.Second = Second;
  tmels.Hour = Hour;
  tmels.Minute = Minute;

  t_now = makeTime(tmels);
  DoW = weekday(t_now); // Update weekday to real weekday
}

void menuFunctions(BTNAction action) // Ihre Menüfunktionen
{

  // nach aktueller iteration noch ein update um menu zu refreshen
  if (action != NOTHING)
  {
    menuRefreshFlag = true;
  }

  lcd.clear();
  // // Serial.print("Days Open: ");
  // // Serial.println(days_o);
  // // Serial.print("Days Close: ");
  // // Serial.println(days_c);

  // ================================================
  // ================ Hauptmenü: ====================
  // ================================================

  if (menu_id == 000)
  {
    // // Serial.println("Hauptmenü");
    lcd.createChar(TIME_SYM, time_sym);
    lcd.createChar(LUX_SYM, lux_sym);
    lcd.createChar(ARROW_DOWN, arrowDown);
    lcd.createChar(ARROW_UP, arrowUp);

    timeUpdate();
    dateUpdate();

    switch (action)
    {
    case LEFT:
      menu_id = 700;
      break;

    case RIGHT:
      menu_id = 100;
      break;

    case SELECT:
      menu_id = 100;
      break;

    case EXIT:
      menu_id = 000; // Startscreen
      break;

    default:
      menu_id = 000;
      break;
    }

    lcd.setCursor(0, 0);
    lcd.printf("%02d", Hour);
    lcd.setCursor(2, 0);
    if (blink)
    {
      lcd.print(":");
    }
    else
    {
      lcd.print(" ");
    }

    lcd.setCursor(3, 0);
    lcd.printf("%02d", Minute);
    lcd.setCursor(5, 0);
    lcd.print("   ");
    lcd.setCursor(8, 0);
    lcd.print(MenuItemsDays[DoW - 1][lang]);
    lcd.setCursor(11, 0);
    lcd.printf("%02d", Date);
    lcd.setCursor(13, 0);
    lcd.print(".");
    lcd.setCursor(14, 0);
    lcd.printf("%02d", Month);

    // Serial.println("HERE");
    /*
        lcd.setCursor(0, 1);
        lcd.print(MenuItemsSpecial[1][lang]);
        lcd.setCursor(4, 1);
        lcd.print(": ");
        lcd.setCursor(6, 1);
    */
    lcd.setCursor(0, 1);
    if (blinkCount / (blinkFactor / 2) == 0)
    {
      lcd.print(MenuItemsSpecial[2][lang]);
    }
    else if (blinkCount / (blinkFactor / 2) == 1)
    {
      lcd.print(MenuItemsSpecial[1][lang]);
    }
    else
    {
      lcd.setCursor(1, 1);
      lcd.print(MenuItemsDays[blinkCount / blinkFactor - 1][lang]);
      lcd.setCursor(4, 1);

      if (blinkCount % blinkFactor < blinkFactor / 2)
      {
        if (blink)
        {
          lcd.write(byte(ARROW_UP));
        }
        else
        {
          lcd.write(" ");
        }

        nextAlarm = &openingAlarms[blinkCount / blinkFactor - 1];
      }
      else
      {
        if (blink)
        {
          lcd.write(byte(ARROW_DOWN));
        }
        else
        {
          lcd.write(" ");
        }
        nextAlarm = &closingAlarms[blinkCount / blinkFactor - 1];
      }

      lcd.setCursor(7, 1);
      switch ((*nextAlarm).mode)
      {
      case LICHT:
        lcd.write(byte(LUX_SYM));
        lcd.setCursor(8, 1);
        lcd.printf("%01d", (*nextAlarm).lux);
        break;

      case LICHT_ZEIT:
        lcd.write(byte(TIME_SYM));
        lcd.setCursor(8, 1);
        lcd.printf("%02d", (*nextAlarm).hour);
        lcd.setCursor(10, 1);
        lcd.print(":");
        lcd.setCursor(11, 1);
        lcd.printf("%02d", (*nextAlarm).minute);
        lcd.setCursor(14, 1);
        lcd.write(byte(LUX_SYM));
        lcd.setCursor(15, 1);
        lcd.print((*nextAlarm).lux);
        break;

      case ZEIT:
        lcd.write(byte(TIME_SYM));
        lcd.setCursor(8, 1);
        lcd.printf("%02d", (*nextAlarm).hour);
        lcd.setCursor(10, 1);
        lcd.print(":");
        lcd.setCursor(11, 1);
        lcd.printf("%02d", (*nextAlarm).minute);
        break;

      case NICHT:
        lcd.print("\x2F"
                  "MANUAL");
        break;

      default:
        break;
      }
    }
    lcd.noCursor();
    lcd.noBlink();
    action = NOTHING;
  }

  // TODO: Statusmeldung implementieren (Montag - Sonntag)

  // ================================================
  // ================ INIT-MENU: ====================
  // ================================================

  else if (menu_id == 001)
  {
    // // Serial.println("INIT-MENU");

    switch (action)
    {
    case LEFT:
      menu_id = 300;
      break;

    case RIGHT:
      menu_id = 300;
      break;

    case SELECT:
      menu_id = 300;
      break;

    case EXIT:
      menu_id = 300;
      break;

    default:
      menu_id = 001;
      break;
    }

    lcd.setCursor(0, 0);
    lcd.print(InitItems[0][lang]);
    lcd.setCursor(0, 1);
    if (blinkCount % 8 < 4)
    {
      lcd.print(InitItems[1][lang]);
    }
    else
    {
      lcd.print(InitItems[2][lang]);
    }
    lcd.setCursor(15,1);
    lcd.blink();
    lcd.noCursor();
  }

  // ================================================
  // ============== Öffnungsmodus: ==================
  // ================================================

  else if (menu_id == 100)
  {
    // Serial.println("Öffnungsmodus");

    switch (action)
    {
    case LEFT:
      menu_id = 700;
      break;

    case RIGHT:
      menu_id = 200;
      break;

    case SELECT:
      modeset = MODESET_OPENING;
      menu_id = 110; // Tageseinstellung
      break;

    case EXIT:
      menu_id = 000; // Startscreen
      break;

    default:
      menu_id = 100;
      break;
    }

    lcd.setCursor(0, 0);
    lcd.print("1.");
    lcd.setCursor(2, 0);
    lcd.print(MenuItems[0][lang]);

    // lcd.setCursor(0, 1);
    // switch (days_o){
    //     case WEEK:
    //     lcd.printf("  %s", MenuItemsDays[0][lang]);
    //     break;

    //     case WEEKEND:
    //     lcd.printf("  %s", MenuItemsDays[1][lang]);
    //     break;

    //     case DAILY:
    //     lcd.printf("  %s", MenuItemsDays[2][lang]);
    //     break;

    //     default:
    //     break;
    // }

    lcd.noCursor();
    lcd.noBlink();
    action = NOTHING;
  }

  //====================== Tage auswählen ====================
  else if (menu_id == 110)
  {
    switch (lang)
    {
    case DEUTSCH:
      lcd.createChar(M_INV, M_inv);
      lcd.createChar(o_INV, o_inv);
      lcd.createChar(D_INV, D_inv);
      lcd.createChar(i_INV, i_inv);
      lcd.createChar(F_INV, F_inv);
      lcd.createChar(r_INV, r_inv);
      lcd.createChar(S_INV, S_inv);
      lcd.createChar(a_INV, a_inv);
      break;

    default:
      break;
    }
    // TODO: Multilanguage characters

    if (modeset == MODESET_CLOSING)
    {
      // Serial.println("Schließtage set");
    }
    else
    {
      // Serial.println("Öffnungstage set");
    }

    switch (action)
    {
    case LEFT:

      if ((cursor_tag) == 0)
      {
        (cursor_tag) = 7;
      }
      else
      {
        (cursor_tag)--;
      }
      break;

    case RIGHT:
      (cursor_tag)++;
      if ((cursor_tag) > 7)
      {
        (cursor_tag) = 0;
      }
      break;

    case SELECT:
      if (cursor_tag < 7)
      {
        day_bitmask = (day_bitmask ^ weekdays[cursor_tag]);
      }
      else
      {
        menu_id = 800;
      }
      break;

    case EXIT:
      if (modeset == MODESET_CLOSING)
      {
        menu_id = 200; // Schließmodus
      }
      else
      {
        menu_id = 100; // Öffnungsmodus
      }
      break;

    default:
      menu_id = 110;
      break;
    }

    lcd.setCursor(0, 0);
    lcd.print(">  ");
    lcd.setCursor(2, 0);
    if (modeset == MODESET_CLOSING)
    {
      lcd.print(MenuItems[1][lang]);
    }
    else
    {
      lcd.print(MenuItems[0][lang]);
    }

    if ((day_bitmask & MONDAY) > 0)
    {
      lcd.setCursor(2, 1);
      lcd.write(MenuItemsDaysInv[1][lang][0]);
      lcd.setCursor(3, 1);
      lcd.write(MenuItemsDaysInv[1][lang][1]);
    }
    else
    {
      lcd.setCursor(2, 1);
      lcd.print(MenuItemsDays[1][lang]);
    }

    if ((day_bitmask & TUESDAY) > 0)
    {
      lcd.setCursor(4, 1);
      lcd.write(MenuItemsDaysInv[2][lang][0]);
      lcd.setCursor(5, 1);
      lcd.write(MenuItemsDaysInv[2][lang][1]);
    }
    else
    {
      lcd.setCursor(4, 1);
      lcd.print(MenuItemsDays[2][lang]);
    }

    if ((day_bitmask & WEDNESDAY) > 0)
    {
      lcd.setCursor(6, 1);
      lcd.write(MenuItemsDaysInv[3][lang][0]);
      lcd.setCursor(7, 1);
      lcd.write(MenuItemsDaysInv[3][lang][1]);
    }
    else
    {
      lcd.setCursor(6, 1);
      lcd.print(MenuItemsDays[3][lang]);
    }

    if ((day_bitmask & THURSDAY) > 0)
    {
      lcd.setCursor(8, 1);
      lcd.write(MenuItemsDaysInv[4][lang][0]);
      lcd.setCursor(9, 1);
      lcd.write(MenuItemsDaysInv[4][lang][1]);
    }
    else
    {
      lcd.setCursor(8, 1);
      lcd.print(MenuItemsDays[4][lang]);
    }

    if ((day_bitmask & FRIDAY) > 0)
    {
      lcd.setCursor(10, 1);
      lcd.write(MenuItemsDaysInv[5][lang][0]);
      lcd.setCursor(11, 1);
      lcd.write(MenuItemsDaysInv[5][lang][1]);
    }
    else
    {
      lcd.setCursor(10, 1);
      lcd.print(MenuItemsDays[5][lang]);
    }

    if ((day_bitmask & SATURDAY) > 0)
    {
      lcd.setCursor(12, 1);
      lcd.write(MenuItemsDaysInv[6][lang][0]);
      lcd.setCursor(13, 1);
      lcd.write(MenuItemsDaysInv[6][lang][1]);
    }
    else
    {
      lcd.setCursor(12, 1);
      lcd.print(MenuItemsDays[6][lang]);
      ;
    }

    if ((day_bitmask & SUNDAY) > 0)
    {
      lcd.setCursor(0, 1);
      lcd.write(MenuItemsDaysInv[0][lang][0]);
      lcd.setCursor(1, 1);
      lcd.write(MenuItemsDaysInv[0][lang][1]);
    }
    else
    {
      lcd.setCursor(0, 1);
      lcd.print(MenuItemsDays[0][lang]);
    }

    lcd.setCursor(15, 1);
    lcd.write(byte(0x7E));

    if (cursor_tag < 7)
    {
      lcd.setCursor(cursor_tag * 2, 1);
    }
    else
    {
      lcd.setCursor(15, 1);
    }

    lcd.noCursor();
    lcd.blink();
  }

  // ================================================
  // ============== Schließmodus: ===================
  // ================================================

  else if (menu_id == 200)
  {
    // Serial.println("Schließmodus");

    switch (action)
    {
    case LEFT:
      menu_id = 100;
      break;

    case RIGHT:
      timeSetMode = SETNOTHING;
      menu_id = 300;
      break;

    case SELECT:
      modeset = MODESET_CLOSING;
      menu_id = 110; // Tageseinstellung
      break;

    case EXIT:
      menu_id = 000; // Startscreen
      break;

    default:
      menu_id = 200;
      break;
    }

    lcd.setCursor(0, 0);
    lcd.print("2.");
    lcd.setCursor(2, 0);
    lcd.print(MenuItems[1][lang]);

    // lcd.setCursor(0, 1);
    // switch (days_c){
    //     case WEEK:
    //     lcd.printf("  %s", MenuItemsDays[0][lang]);
    //     break;

    //     case WEEKEND:
    //     lcd.printf("  %s", MenuItemsDays[1][lang]);
    //     break;

    //     case DAILY:
    //     lcd.printf("  %s", MenuItemsDays[2][lang]);
    //     break;

    //     default:
    //     break;
    // }
    lcd.noCursor();
    lcd.noBlink();
    action = NOTHING;
  }

  // ================================================
  // ================= Uhrzeit: =====================
  // ================================================

  else if (menu_id == 300)
  {
    // Serial.println("Uhrzeit set");

    if (timeSetMode == SETNOTHING)
    {
      Serial.println("Func: Menu_MenuID = 300, Timesetmode == Nothing");
      timeUpdate();
      dateUpdate();
      // TODO: get current time
    }

    switch (action)
    {
    case LEFT:
      if (timeSetMode == SETMINUTE)
      {

        if (Minute == 0)
        {
          Minute = 59;
        }
        else
        {
          Minute--;
        }
        menu_id = 300;
      }
      else if (timeSetMode == SETHOUR)
      {

        if (Hour == 0)
        {
          Hour = 23;
        }
        else
        {
          Hour--;
        }
        menu_id = 300;
      }
      else
      {
        timeSetMode = SETNOTHING;
        menu_id = 200;
      }
      break;

    case RIGHT:
      if (timeSetMode == SETMINUTE)
      {
        Minute++;
        if (Minute > 59)
        {
          Minute = 0;
        }
        menu_id = 300;
      }
      else if (timeSetMode == SETHOUR)
      {
        Hour++;
        if (Hour > 23)
        {
          Hour = 0;
        }
        menu_id = 300;
      }
      else
      {
        timeSetMode = SETNOTHING;
        menu_id = 400;
      }
      break;

    case SELECT:
      if (timeSetMode == SETMINUTE)
      {
        dateUpdate();
        saveTimeSetting();
        timeSetMode = SETNOTHING;
      }
      else if (timeSetMode == SETNOTHING)
      {
        timeSetMode = SETHOUR;
      }
      else if (timeSetMode == SETHOUR)
      {
        timeSetMode = SETMINUTE;
      }
      menu_id = 300; //
      break;

    case EXIT:
      if (timeSetMode == SETNOTHING)
      {
        menu_id = 000; // Hauptmenu
      }
      else
      {
        dateUpdate();
        saveTimeSetting();
        timeSetMode = SETNOTHING;
      }
      break;

    default:
      menu_id = 300;
      break;
    }

    if (timeSetMode == SETNOTHING)
    {
      lcd.setCursor(0, 0);
      lcd.print("3.");
    }
    else
    {
      lcd.setCursor(0, 0);
      lcd.print(">  ");
    }
    lcd.setCursor(2, 0);
    lcd.print(MenuItems[2][lang]);

    lcd.setCursor(2, 1);
    lcd.printf("%02d", Hour);
    lcd.setCursor(4, 1);

    if (blink && timeSetMode == SETNOTHING)
    {
      lcd.print(" ");
    }
    else
    {
      lcd.print(":");
    }

    lcd.setCursor(5, 1);
    lcd.printf("%02d", Minute);

    if (timeSetMode == SETHOUR)
    {
      lcd.setCursor(2, 1);
      lcd.noCursor();
      lcd.blink();
    }
    else if (timeSetMode == SETMINUTE)
    {
      lcd.setCursor(5, 1);
      lcd.noCursor();
      lcd.blink();
    }
    else if (timeSetMode == SETNOTHING)
    {
      lcd.noCursor();
      lcd.noBlink();
    }

    action = NOTHING;
  }

  // ================================================
  // ================= Datum: =======================
  // ================================================
  else if (menu_id == 400)
  {
    // Serial.println("Datum set");

    if (timeSetMode == SETNOTHING)
    {
      // TODO: get current date
      timeUpdate();
      dateUpdate();
    }

    if (timeSetMode != SETDAY)
    { // Set monthly days to max if month or year changed
      if (Date > maxDayThisMonth)
      {
        Date = maxDayThisMonth;
      }
      menu_id = 400;
    }

    switch (action)
    {
    case LEFT:

      if (timeSetMode == SETDAY)
      {
        Date--;
        if (Date < 1)
        {
          Date = maxDayThisMonth;
        }
        menu_id = 400;
      }
      else if (timeSetMode == SETMONTH)
      {
        Month--;
        if (Month < 1)
        {
          Month = 12;
        }
        menu_id = 400;
      }
      else if (timeSetMode == SETYEAR)
      {
        Year--;
        if (Year < 2000)
        {
          Year = 2099;
        }
        menu_id = 400;
      }
      else
      {
        timeSetMode = SETNOTHING;
        menu_id = 300;
      }
      break;

    case RIGHT:

      if (timeSetMode == SETDAY)
      {
        Date++;
        if (Date > maxDayThisMonth)
        {
          Date = 1;
        }
        menu_id = 400;
      }
      else if (timeSetMode == SETMONTH)
      {
        Month++;
        if (Month > 12)
        {
          Month = 1;
        }
        menu_id = 400;
      }
      else if (timeSetMode == SETYEAR)
      {
        Year++;
        if (Year > 2099)
        {
          Year = 2000;
        }
        menu_id = 400;
      }
      else
      {
        timeSetMode = SETNOTHING;
        menu_id = 500;
      }

      break;

    case SELECT:
      if (timeSetMode == SETDAY)
      {
        timeSetMode = SETMONTH;
      }
      else if (timeSetMode == SETMONTH)
      {
        timeSetMode = SETYEAR;
      }
      else if (timeSetMode == SETYEAR)
      {
        // TODO: Implement saveTimeSetting();
        timeUpdate();
        saveTimeSetting();
        timeSetMode = SETNOTHING;
      }
      else if (timeSetMode == SETNOTHING)
      {
        timeSetMode = SETDAY;
      }
      menu_id = 400; //
      break;

    case EXIT:
      if (timeSetMode == SETNOTHING)
      {
        menu_id = 000; // Hauptmenu
      }
      else
      {
        // TODO: Implement saveTimeSetting();
        timeUpdate();
        saveTimeSetting();
        timeSetMode = SETNOTHING;
      }
      break;

    default:
      menu_id = 400;
      break;
    }

    // Calculate max day:
    if (Month == 1 || Month == 3 || Month == 5 || Month == 7 || Month == 8 || Month == 10 || Month == 12)
    {
      maxDayThisMonth = 31;
    }
    else if (Month == 2)
    {
      if (Year % 4 == 0)
      { // leap year correction
        maxDayThisMonth = 29;
      }
      else
      {
        maxDayThisMonth = 28;
      }
    }
    else
    {
      maxDayThisMonth = 30;
    }

    if (timeSetMode == SETNOTHING)
    {
      lcd.setCursor(0, 0);
      lcd.print("4.");
    }
    else
    {
      lcd.setCursor(0, 0);
      lcd.print(">  ");
    }
    lcd.setCursor(2, 0);
    lcd.print(MenuItems[3][lang]);

    lcd.setCursor(2, 1);
    lcd.printf("%02d.", Date);
    lcd.setCursor(5, 1);
    lcd.printf("%02d.", Month);
    lcd.setCursor(8, 1);
    lcd.printf("%04d", Year);

    if (timeSetMode == SETDAY)
    {
      lcd.setCursor(2, 1);
      lcd.noCursor();
      lcd.blink();
    }
    else if (timeSetMode == SETMONTH)
    {
      lcd.setCursor(5, 1);
      lcd.noCursor();
      lcd.blink();
    }
    else if (timeSetMode == SETYEAR)
    {
      lcd.setCursor(8, 1);
      lcd.noCursor();
      lcd.blink();
    }
    else if (timeSetMode == SETNOTHING)
    {
      lcd.noCursor();
      lcd.noBlink();
    }

    action = NOTHING;
  }

  // ================================================
  // =============== Erweitert: =====================
  // ================================================

  else if (menu_id == 500)
  {
    // Serial.println("Erweitert");
    // TODO: Erweiterte einstellungen implementieren

    switch (action)
    {
    case LEFT:
      menu_id = 400;
      break;

    case RIGHT:
      menu_id = 600;
      break;

    case SELECT:
      menu_id = 510; // Erweitert untermenu
      break;

    case EXIT:
      menu_id = 000; // Startscreen
      break;

    default:
      menu_id = 500;
      break;
    }

    lcd.setCursor(0, 0);
    lcd.print("5. ");
    lcd.setCursor(2, 0);
    lcd.print(MenuItems[4][lang]);

    lcd.setCursor(0, 1);
    lcd.printf("SOMMERZEIT TBD");

    lcd.noCursor();

    action = NOTHING;
  }

  // ================================================
  // ================= Sprache: =====================
  // ================================================
  else if (menu_id == 600)
  {
    // Serial.println("Sprache");
    // TODO: Sprache implementieren

    switch (action)
    {
    case LEFT:
      menu_id = 500;
      break;

    case RIGHT:
      menu_id = 700;
      break;

    case SELECT:
      menu_id = 610; // Sprache untermenu
      break;

    case EXIT:
      menu_id = 000; // Startscreen
      break;

    default:
      menu_id = 600;
      break;
    }

    lcd.setCursor(0, 0);
    lcd.print("6. ");
    lcd.setCursor(2, 0);
    lcd.print(MenuItems[5][lang]);

    lcd.setCursor(0, 1);
    lcd.printf("DEUTSCH TBD");

    lcd.noCursor();
  }

  // ================================================
  // =================== Reset: =====================
  // ================================================
  else if (menu_id == 700)
  {
    // Serial.println("Reset");
    // TODO: Reset implementieren

    switch (action)
    {
    case LEFT:
      menu_id = 600;
      break;

    case RIGHT:
      menu_id = 100;
      break;

    case SELECT:
      menu_id = 710; // Reset untermenu
      esp_sleep_enable_ext1_wakeup(WAKEUP_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
      PREP_FOR_DEEP_SLEEP // TODO: reset ersetzen

          break;

    case EXIT:
      menu_id = 000; // Startscreen
      break;

    default:
      menu_id = 700;
      break;
    }

    lcd.setCursor(0, 0);
    lcd.print("7. ");
    lcd.setCursor(2, 0);
    lcd.print(MenuItems[6][lang]);

    lcd.setCursor(0, 1);
    lcd.printf("RESET TBD");

    lcd.noCursor();
  }

  // ================================================
  //=============== Modus auswählen =================
  // ================================================
  else if (menu_id == 800)
  {
    lcd.createChar(TIME_SYM, time_sym);
    lcd.createChar(LUX_SYM, lux_sym);

    if (modeset == MODESET_CLOSING)
    {
      mode = &mode_c;
      // Serial.println("Schließmodus set");
    }
    else
    {
      mode = &mode_o;
      // Serial.println("Öffnungsmodus set");
    }

    switch (action)
    {
    case LEFT:

      if ((*mode) == 0)
      {
        (*mode) = 3;
      }
      else
      {
        (*mode)--;
      }
      break;

    case RIGHT:

      if ((*mode) == 3)
      {
        (*mode) = 0;
      }
      else
      {
        (*mode)++;
      }
      break;

    case SELECT:
      switch ((*mode))
      {
      case LICHT:
        menu_id = 810;
        break;

      case LICHT_ZEIT:
        menu_id = 810;
        break;

      case ZEIT:
        timeSetMode = SETHOUR;
        menu_id = 820;
        break;

      case NICHT:
        // TODO: Speichern!
        menu_id = 900;
        break;
      }
      break;

    case EXIT:
      menu_id = 100; // Hauptmenu
      break;

    default:
      menu_id = 800;
      break;
    }

    lcd.setCursor(0, 0);
    lcd.print(">  ");
    lcd.setCursor(2, 0);
    if (modeset == MODESET_CLOSING)
    {
      lcd.print(MenuItems[1][lang]);
    }
    else
    {
      lcd.print(MenuItems[0][lang]);
    }

    lcd.setCursor(0, 1);
    lcd.print("< ");
    lcd.setCursor(15, 1);
    lcd.print(">");
    lcd.setCursor(2, 1);
    switch ((*mode))
    {
    case LICHT:
      lcd.write(byte(LUX_SYM));
      lcd.setCursor(7, 1);
      lcd.print(MenuItemsMode[0][lang]);
      break;

    case LICHT_ZEIT:
      lcd.write(byte(LUX_SYM));

      lcd.setCursor(4, 1);

      lcd.print(MenuItemsMode[1][lang]);
      lcd.setCursor(13, 1);
      lcd.write(byte(TIME_SYM));
      break;

    case ZEIT:
      lcd.write(byte(TIME_SYM));
      lcd.setCursor(6, 1);
      lcd.print(MenuItemsMode[2][lang]);
      break;

    case NICHT:
      lcd.print("\x2F");
      lcd.setCursor(5, 1);
      lcd.print(MenuItemsMode[3][lang]);
      break;
    }
    lcd.setCursor(2, 1);
    lcd.noBlink();
    lcd.noCursor();
  }

  //============ Lichtschwelle einstellen ===========
  else if (menu_id == 810)
  {
    if (modeset == MODESET_CLOSING)
    {
      lux = &lux_c;
      mode = &mode_c;
      // Serial.println("Lichtschließ set");
    }
    else
    {
      lux = &lux_o;
      mode = &mode_o;
      // Serial.println("Lichtschließ set");
    }

    switch (action)
    {
    case LEFT:
      (*lux)--;
      if ((*lux) < MINLUX)
      {
        (*lux) = MAXLUX;
      }
      break;

    case RIGHT:
      (*lux)++;
      if ((*lux) > MAXLUX)
      {
        (*lux) = MINLUX;
      }
      break;

    case SELECT:
      // TODO: Speichern!
      switch ((*mode))
      {
      case LICHT:
        menu_id = 900;
        break;

      case LICHT_ZEIT:
        timeSetMode = SETHOUR;
        menu_id = 820;
        break;

      case ZEIT:
        timeSetMode = SETHOUR;
        menu_id = 820;
        break;

      case NICHT:

        menu_id = 900;
        break;
      }
      break;

    case EXIT:
      menu_id = 800; // Modus einstellen
      break;

    default:
      menu_id = 810;
      break;
    }

    lcd.setCursor(0, 0);
    lcd.print(">  ");
    lcd.setCursor(2, 0);
    lcd.print(MenuItemsMode[4][lang]);

    lcd.setCursor(6, 1);
    lcd.printf("%s:", MenuItemsMode[5][lang]);
    lcd.setCursor(15, 1);
    lcd.print(analogRead(LDR_VAL)); // TODO: analog read

    lcd.setCursor(0, 1);
    lcd.print("LUX:");
    lcd.setCursor(4, 1);
    lcd.print((*lux));
    lcd.setCursor(4, 1);
    lcd.blink();
    lcd.noCursor();
  }

  //============ Zeitschwelle einstellen ===========
  else if (menu_id == 820)
  {
    if (modeset == MODESET_CLOSING)
    {
      hour_set = &hour_c;
      minute_set = &minute_c;
      mode = &mode_c;
      // Serial.println("Zeitschließ set");
    }
    else
    {
      hour_set = &hour_o;
      minute_set = &minute_o;
      mode = &mode_o;
      // Serial.println("Zeitöffnung set");
    }

    switch (action)
    {
    case LEFT:
      if (timeSetMode == SETMINUTE)
      {

        if ((*minute_set) == 0)
        {
          (*minute_set) = 59;
        }
        else
        {
          (*minute_set)--;
        }
        menu_id = 820;
      }
      else if (timeSetMode == SETHOUR)
      {

        if ((*hour_set) == 0)
        {
          (*hour_set) = 23;
        }
        else
        {
          (*hour_set)--;
        }
        menu_id = 820;
      }
      else
      {
        timeSetMode = SETMINUTE;
        menu_id = 820;
      }
      break;

    case RIGHT:
      if (timeSetMode == SETMINUTE)
      {
        (*minute_set)++;
        if ((*minute_set) > 59)
        {
          (*minute_set) = 0;
        }
        menu_id = 820;
      }
      else if (timeSetMode == SETHOUR)
      {
        (*hour_set)++;
        if ((*hour_set) > 23)
        {
          (*hour_set) = 0;
        }
        menu_id = 820;
      }
      else if (timeSetMode == SETNOTHING)
      {
        timeSetMode = SETHOUR;
        menu_id = 820;
      }
      break;

    case SELECT:
      if (timeSetMode == SETMINUTE)
      {
        timeSetMode = SETNOTHING;
        menu_id = 820; //
      }
      else if (timeSetMode == SETNOTHING)
      {
        menu_id = 900;
        // TODO: Implement saveTimeCloseOpenSetting();
      }
      else if (timeSetMode == SETHOUR)
      {
        timeSetMode = SETMINUTE;
        menu_id = 820; //
      }

      break;

    case EXIT:
      if (timeSetMode == SETNOTHING)
      {
        timeSetMode = SETMINUTE;
      }
      else if (timeSetMode == SETMINUTE)
      {
        timeSetMode = SETHOUR;
      }
      else
      {
        if ((*mode) == LICHT_ZEIT || (*mode) == LICHT)
        {
          menu_id = 810;
        }
        else
        {
          menu_id = 800;
        }
      }
      break;

    default:
      menu_id = 820;
      break;
    }

    lcd.setCursor(0, 0);
    if (modeset == MODESET_CLOSING)
    {
      lcd.print(MenuItemsMode[7][lang]);
    }
    else
    {
      lcd.print(MenuItemsMode[6][lang]);
    }
    lcd.setCursor(2, 1);
    lcd.printf("%02d", (*hour_set));
    lcd.setCursor(4, 1);

    lcd.print(":");

    lcd.setCursor(5, 1);
    lcd.printf("%02d", (*minute_set));

    lcd.setCursor(15, 1);
    lcd.write(byte(0x7E));

    if (timeSetMode == SETHOUR)
    {
      lcd.setCursor(2, 1);
      lcd.noCursor();
      lcd.blink();
    }
    else if (timeSetMode == SETMINUTE)
    {
      lcd.setCursor(5, 1);
      lcd.noCursor();
      lcd.blink();
    }
    else if (timeSetMode == SETNOTHING)
    {
      lcd.setCursor(15, 1);
      lcd.noCursor();
      lcd.blink();
    }
  }

  // ================================================
  //=================== Speichern ===================
  // ================================================

  else if (menu_id == 900)
  {
    switch (action)
    {
    case LEFT:
      menu_id = 100;
      break;

    case RIGHT:
      menu_id = 100;
      break;

    case SELECT:
      menu_id = 100;
      break;

    case EXIT:
      menu_id = 100;
      break;

    default:
      menu_id = 900;
      break;
    }

    saveAlarmValues();
    // TODO: einmaliges speichern
    lcd.setCursor(0, 0);
    lcd.print(MenuItemsMode[9][lang]);
    lcd.noBlink();
    lcd.noCursor();

    statusabfrage();
  }
}

void menuUpdate()
{

  // Serial.print("Func: menuUpdate(), menu_id: ");
  // Serial.println(menu_id);

  if (digitalRead(SW_SELECT) == HIGH && button_flag == 0)
  {
    // Serial.println("Select pressed");
    menuFunctions(SELECT);
    button_flag = 1;
    previousMillis = millis();
    lastPress = millis();
  }
  if (digitalRead(SW_EXIT) == HIGH && button_flag == 0)
  {
    // Serial.println("Exit pressed");
    menuFunctions(EXIT);
    button_flag = 1;
    previousMillis = millis();
    lastPress = millis();
  }
  if (digitalRead(SW_BACK) == HIGH && button_flag == 0)
  {
    // Serial.println("Left pressed");
    menuFunctions(LEFT);
    button_flag = 1;
    previousMillis = millis();
    lastPress = millis();
  }
  else if (digitalRead(SW_FWD) == HIGH && button_flag == 0)
  {
    // Serial.println("Right pressed");
    menuFunctions(RIGHT);
    button_flag = 1;
    previousMillis = millis();
    lastPress = millis();
  }
  else
  {
    if (menuRefreshFlag)
    {
      menuRefreshFlag = false;
      menuFunctions(NOTHING);
    }
  }

  // Check if long press flag should be set or reset
  if (digitalRead(SW_FWD) == HIGH || digitalRead(SW_BACK) == HIGH || digitalRead(SW_EXIT) == HIGH || digitalRead(SW_SELECT) == HIGH)
  {
    if (!longPressFlag)
    {
      longPressStart = millis();
      longPressFlag = true;
    }
  }
  else
  {
    longPressFlag = false;
  }

  // Check if long press is valdi and short debounce time should be taken
  if (longPressFlag && (millis() - longPressStart > BUTTON_LONG_PRESS_TIME))
  {
    if (millis() - previousMillis >= BUTTON_DEBOUNCE_TIME_FAST)
    {
      previousMillis = millis();
      button_flag = 0;
    }
  }
  else if (millis() - previousMillis >= BUTTON_DEBOUNCE_TIME_NORMAL)
  {
    previousMillis = millis();
    button_flag = 0;
  }
}

// ================================================================================================
// ================================================================================================
// ================================================================================================

// ================================================================================================
// ========================================== BACKEND ============================================
// ================================================================================================

// ================================================================================================
// ================================================================================================
// ================================================================================================

/*
22.12 als nächstes zu tun:
- Wakeup pfade untersuchen
  - timer wakeup alle pfade ok
  - statusabfrage immer ok

  - alarm während betrieb
  - timer während betrieb
- testing beginnen
  - aufwachen direkt nach einschlafen untersuchen
  - sleep timer testen



*/

void saveTimeSetting()
{
  rtc.setTime(Second, Minute, Hour, DoW - 1, Date, Month, Year);
}

uint8_t VoltToLux(uint32_t mV)
{ // TODO: LDR lux umsetzung implementieren
  uint32_t m_mV = mV;
  int i;
  for (i = 0; i < 9; i++)
  {
    if (mV > voltMap[i])
    {
      if (mV < voltMap[i + 1])
      {
        break;
      }
    }
  }
  return i; // Lux Schwelle
}

uint8_t getLux()
{
  uint8_t lux;
  adc_ldr.attach(LDR_VAL);
  digitalWrite(LDR_EN, HIGH);
  lux = VoltToLux(adc_ldr.readMiliVolts());
  digitalWrite(LDR_EN, LOW); // TODO: Prüfen ob das reicht
  return lux;
}

float getVolt()
{

  float mv;
  adc_vbatt.attach(V_BATT);
  digitalWrite(LDR_EN, HIGH);
  mv = (adc_vbatt.readVoltage() * (1.0 / 6.0)) - 300.0; // TODO: Volt umsetzung implementieren

  return mv;
}

void activateLDR()
{

  if (LDRFlag == POS_DOWN)
  {
    // als nächstes Schließen
    if (getLux() <= nextLux)
    {
      ramCounter++;
      if (ramCounter >= lux_debounce_number)
      { // Counter voll
        ramCounter = 0;
        moveMotor(POS_DOWN);
      }
      else
      { // Counter noch nicht voll
        esp_sleep_enable_timer_wakeup(S_TO_uS * lux_debounce_time);
      }
    }
    else
    {
      esp_sleep_enable_timer_wakeup(S_TO_uS * t_sens);
      ramCounter = 0;
    }
  }
  else if (LDRFlag == POS_UP)
  {
    // als nächstes Öffnen
    if (getLux() >= nextLux)
    {
      ramCounter++;
      if (ramCounter >= lux_debounce_number)
      { // Counter voll
        ramCounter = 0;
        moveMotor(POS_UP);
      }
      else
      { // Counter noch nicht voll
        esp_sleep_enable_timer_wakeup(S_TO_uS * lux_debounce_time);
      }
    }
    else
    {
      esp_sleep_enable_timer_wakeup(S_TO_uS * t_sens);
      ramCounter = 0;
    }
  }
}

void setAlarm(byte mDoW, byte mHour, byte mMinute)
{
  rtc.disableAllInterrupts();
  rtc.clearAllInterruptFlags();                         // Clear all flags in case any interrupts have occurred.
  rtc.setItemsToMatchForAlarm(true, true, true, false); // The alarm interrupt compares the alarm interrupt registers with the current time registers. We must choose which registers we want to compare by setting bits to true or false
  rtc.setAlarmMinutes(mMinute);
  rtc.setAlarmHours(mHour);
  rtc.setAlarmWeekday(weekdays[DoW - 1]);
  rtc.enableHardwareInterrupt(ALARM_INTERRUPT);
}

void setAlarmTomorrow0()
{
  if (weekday(t_now) >= 7)
  {
    setAlarm(1, 0, 1);
  }
  else
  {
    setAlarm(weekday(t_now) + 1, 0, 1);
  }
}

void setNextOpeningAlarm()
{
  Serial.println("Setze öffnungsalarm und gehe schlafen");

  nextMove = POS_UP;
  LDRFlag = POS_BLOCKED;
  alarmFlag = POS_BLOCKED;
  nextLux = openingAlarms[weekday(t_now) - 1].lux;
  if (manualFlag)
  {
    alarmFlag = POS_BLOCKED;
    setAlarmTomorrow0();
    esp_sleep_enable_ext1_wakeup(WAKEUP_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
  }

  if (F_init_flag == 0)
  {
    memory.putInt("init_flag", 1); // flags wurden gesetzt
  }

  switch (openingAlarms[weekday(t_now) - 1].mode)
  {
  case ZEIT:
    if (!manualFlag)
    {
      if ((openingAlarms[weekday(t_now) - 1].hour * 60 + openingAlarms[weekday(t_now) - 1].minute) > (hour(t_now) * 60 + minute(t_now) + 1)) // Alarm noch nicht vergangen
      {
        Serial.print("Öffnungsalarm noch nicht vergangen, setze alarm auf Stunde: ");
        Serial.print(openingAlarms[weekday(t_now) - 1].hour);
        Serial.print(" und Minute: ");
        Serial.println(openingAlarms[weekday(t_now) - 1].minute);
        alarmFlag = nextMove;
        setAlarm(weekday(t_now), openingAlarms[weekday(t_now) - 1].hour, openingAlarms[weekday(t_now) - 1].minute);
        esp_sleep_enable_ext1_wakeup(WAKEUP_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
      }

      else // Alarm schon vergangen, setze Alarm auf morgen 00:01 Uhr
      {
        Serial.println("Öffnungsalarm bereits vergangen, setze alarm Morgen 0 Uhr");
        alarmFlag = POS_BLOCKED;
        setAlarmTomorrow0();
        esp_sleep_enable_ext1_wakeup(WAKEUP_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
      }
    }

    if (!menuActive)
    {
      PREP_FOR_DEEP_SLEEP // TODO: Deep Sleep with alarm interrupt
    }

    break;

  case LICHT:
    alarmFlag = POS_BLOCKED;
    setAlarmTomorrow0();
    esp_sleep_enable_ext1_wakeup(WAKEUP_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
    if (!manualFlag)
    {
      // LDR wird nur aktiviert wenn nicht manuell geschlossen wurde

      if (lastMove + t_delta_min > t_now + 1) // Letzte Bewegung noch nicht lange genug her
      {
        LDRFlag = nextMove;
        esp_sleep_enable_timer_wakeup(S_TO_uS * (lastMove + t_delta_min - t_now));
      }
      else // Letzte Bewegung schon lange genug her um LDR zu aktivieren
      {
        LDRFlag = nextMove;
        activateLDR();
      }
    }
    if (!menuActive)
    {
      PREP_FOR_DEEP_SLEEP // TODO: Deep Sleep with alarm interrupt
    }

    break;

  case LICHT_ZEIT:
    if (!manualFlag)
    {
      // LDR wird nur aktiviert wenn nicht manuell geschlossen wurde

      if ((openingAlarms[weekday(t_now) - 1].hour * 60 + openingAlarms[weekday(t_now) - 1].minute) > (hour(t_now) * 60 + minute(t_now) + 1)) // Alarm noch nicht vergangen
      {
        alarmFlag = POS_BLOCKED;
        setAlarm(weekday(t_now), openingAlarms[weekday(t_now) - 1].hour, openingAlarms[weekday(t_now) - 1].minute);
        esp_sleep_enable_ext1_wakeup(WAKEUP_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
      }

      else // Alarm schon vergangen, setze Alarm auf morgen 00:01 Uhr
      {
        alarmFlag = POS_BLOCKED;
        setAlarmTomorrow0();
        esp_sleep_enable_ext1_wakeup(WAKEUP_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
        if (lastMove + t_delta_min > t_now + 1) // Letzte Bewegung noch nicht lange genug her
        {
          LDRFlag = nextMove;
          esp_sleep_enable_timer_wakeup(S_TO_uS * (lastMove + t_delta_min - t_now));
        }
        else // Letzte Bewegung schon lange genug her um LDR zu aktivieren
        {
          LDRFlag = nextMove;
          activateLDR();
        }
      }
    }
    if (!menuActive)
    {
      PREP_FOR_DEEP_SLEEP // TODO: Deep Sleep with alarm interrupt
    }
    break;

  case NICHT:
    alarmFlag = POS_BLOCKED;
    setAlarmTomorrow0();
    esp_sleep_enable_ext1_wakeup(WAKEUP_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);

    if (!menuActive)
    {
      PREP_FOR_DEEP_SLEEP // TODO: Deep Sleep with alarm interrupt
    }
    break;

  default:
    if (!menuActive)
    {
      PREP_FOR_DEEP_SLEEP // TODO: Deep Sleep with alarm interrupt
    }
    break;
  }
}

void setNextClosingAlarm()
{
  Serial.println("Setze schließalarm und gehe schlafen");
  nextMove = POS_DOWN;
  LDRFlag = POS_BLOCKED;
  alarmFlag = POS_BLOCKED;
  if (manualFlag)
  {
    alarmFlag = POS_BLOCKED;
    setAlarmTomorrow0();
    esp_sleep_enable_ext1_wakeup(WAKEUP_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
  }

  if (F_init_flag == 0)
  {
    memory.putInt("init_flag", 1); // flags wurden gesetzt
  }

  nextLux = closingAlarms[weekday(t_now) - 1].lux;
  switch (closingAlarms[weekday(t_now) - 1].mode)
  {
  case ZEIT:
    if ((closingAlarms[weekday(t_now) - 1].hour * 60 + closingAlarms[weekday(t_now) - 1].minute) > (hour(t_now) * 60 + minute(t_now) + 1)) // Alarm noch nicht vergangen
    {
      alarmFlag = nextMove;
      setAlarm(weekday(t_now), closingAlarms[weekday(t_now) - 1].hour, closingAlarms[weekday(t_now) - 1].minute);
      esp_sleep_enable_ext1_wakeup(WAKEUP_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
    }

    else // Alarm schon vergangen, setze Alarm auf morgen 00:01 Uhr
    {
      alarmFlag = POS_BLOCKED;
      setAlarmTomorrow0();
      esp_sleep_enable_ext1_wakeup(WAKEUP_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
    }

    if (!menuActive)
    {
      PREP_FOR_DEEP_SLEEP // TODO: Deep Sleep with alarm interrupt
    }
    break;

  case LICHT:
    alarmFlag = POS_BLOCKED;
    setAlarmTomorrow0();
    esp_sleep_enable_ext1_wakeup(WAKEUP_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
    if (!manualFlag)
    {
      // wird bei Licht nur geschlossen, wenn nicht manuell geöffnet wurde

      if (lastMove + t_delta_min > t_now + 1) // Letzte Bewegung noch nicht lange genug her
      {
        LDRFlag = nextMove;
        esp_sleep_enable_timer_wakeup(S_TO_uS * (lastMove + t_delta_min - t_now));
      }
      else // Letzte Bewegung schon lange genug her um LDR zu aktivieren
      {
        LDRFlag = nextMove;
        activateLDR();
      }
    }

    if (!menuActive)
    {
      PREP_FOR_DEEP_SLEEP // TODO: Deep Sleep with alarm interrupt
    }
    break;

  case LICHT_ZEIT:

    if ((closingAlarms[weekday(t_now) - 1].hour * 60 + closingAlarms[weekday(t_now) - 1].minute) > (hour(t_now) * 60 + minute(t_now) + 1)) // Alarm noch nicht vergangen
    {
      alarmFlag = nextMove;
      setAlarm(weekday(t_now), closingAlarms[weekday(t_now) - 1].hour, closingAlarms[weekday(t_now) - 1].minute); // Späteste Schließung
      esp_sleep_enable_ext1_wakeup(WAKEUP_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);

      if (!manualFlag)
      {
        // Sensor wird bei LichtZeit nur aktiviert, wenn nicht manuell geöffnet wurde
        if (lastMove + t_delta_min > t_now + 1) // Letzte Bewegung noch nicht lange genug her
        {
          LDRFlag = nextMove;
          esp_sleep_enable_timer_wakeup(S_TO_uS * (lastMove + t_delta_min - t_now));
        }
        else // Letzte Bewegung schon lange genug her um LDR zu aktivieren
        {
          LDRFlag = nextMove;
          activateLDR();
        }
      }
    }

    else // Alarm schon vergangen, setze Alarm auf morgen 00:01 Uhr
    {
      alarmFlag = POS_BLOCKED;
      setAlarmTomorrow0();
      esp_sleep_enable_ext1_wakeup(WAKEUP_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
    }

    if (!menuActive)
    {
      PREP_FOR_DEEP_SLEEP // TODO: Deep Sleep with alarm interrupt
    }
    break;

  case NICHT:
    alarmFlag = POS_BLOCKED;
    setAlarmTomorrow0();
    esp_sleep_enable_ext1_wakeup(WAKEUP_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);

    if (!menuActive)
    {
      PREP_FOR_DEEP_SLEEP // TODO: Deep Sleep with alarm interrupt
    }
    break;

  default:
    if (!menuActive)
    {
      PREP_FOR_DEEP_SLEEP // TODO: Deep Sleep with alarm interrupt
    }
    break;
  }
}

void statusabfrage()
{
  Serial.println("Func: statusabfrage()");
  pinMode(END_LOW, INPUT_PULLUP);
  pinMode(END_UP, INPUT_PULLUP);

  delay(5);

  byte up = digitalRead(END_UP);
  byte low = digitalRead(END_LOW);
  Serial.print("Func: statusabfrage(), Status upper: ");
  Serial.println(up);
  Serial.print("Func: statusabfrage(), Status lower: ");
  Serial.println(low);

  if (!up && !low)
  {
    doorPosition = POS_DOWN;
    // Klappe ist unten oder
    // Klappe ist zu leicht
    Serial.println("Klappe ist unten");
  }
  else if (!up && low)
  { // Klappe hängt fest
    doorPosition = POS_BLOCKED;
    Serial.println("Klappe ist stuck");
  }
  else if (up && low)
  {
    doorPosition = POS_UP;
    Serial.println("Klappe ist oben");
    // Klappe ist oben
  }
  else if (up && !low)
  { // Klappe fährt grade  oder
    doorPosition = POS_DRIVING;
    Serial.println("Klappe fährt");
  }

  switch (doorPosition)
  {
  case POS_UP:
    setNextClosingAlarm();
    break;

  case POS_DOWN:
    setNextOpeningAlarm();
    break;

  case POS_BLOCKED:
    errorFlag = TIMER_BLOCKED_ELAPSED; // TODO: ErrorHandling
    break;

  case POS_DRIVING:
    // Klappe ist irgendwo zwischen oben und unten --> irgendetwas ist passiert seit der letzten Bewegung.
    // In diesem Fall wird die letzte bekannte position abgefragt und das Gegenteil gemacht.
    if (lastDoorPosition == POS_DOWN)
    {
      setNextOpeningAlarm();
    }
    else if (lastDoorPosition == POS_UP)
    {
      setNextClosingAlarm(); // TODO: Diskutieren, ob standardmäßiges Schließen genug ist
    }
    else
    {
      errorFlag = TIMER_BLOCKED_ELAPSED; // TODO: ErrorHandling
    }
    break;
  }
}

void moveMotor(KLAPPENPOSITION pos)
{
  Serial.print("Func: moveMotor() Ziel: ");
  if (pos == POS_DOWN)
  {
    Serial.println("Klappe runter");
  }
  else
  {
    Serial.println("Klappe hoch");
  }

  pinMode(END_LOW, INPUT_PULLUP);
  pinMode(END_UP, INPUT_PULLUP);

  delay(5); // Input Pins müssen gepullupt werden

  byte up = digitalRead(END_UP);
  byte low = digitalRead(END_LOW);
  Serial.print("Func: moveMotor(), Status upper: ");
  Serial.println(up);
  Serial.print("Func: moveMotor(), Status lower: ");
  Serial.println(low);

  doorDirection = STANDING;

  if (!up && !low)
  {
    doorPosition = POS_DOWN;
    Serial.println("Func: moveMotor(), Klappe unten ");
  }
  else if (!up && low)
  { // Klappe hängt fest
    doorPosition = POS_BLOCKED;
    Serial.println("Func: moveMotor(), Klappe hängt ");
  }
  else if (up && low)
  {
    doorPosition = POS_UP;
    Serial.println("Func: moveMotor(), Klappe oben ");
  }
  else if (up && !low)
  { // Klappe fährt grade  oder
    doorPosition = POS_DRIVING;
    Serial.println("Func: moveMotor(), Klappe zwischen oben und unten ");
  }

  while (doorPosition != pos)
  {
    pinMode(END_LOW, INPUT_PULLUP);
    pinMode(END_UP, INPUT_PULLUP);

    delay(5);

    byte up = digitalRead(END_UP);
    byte low = digitalRead(END_LOW);
    // Serial.print("Func: moveMotor(), Status upper: ");
    // Serial.println(up);
    // Serial.print("Func: moveMotor(), Status lower: ");
    // Serial.println(low);

    if (!up && !low)
    {
      Serial.println("Func: moveMotor(), Klappe ist unten, fährt hoch ");
      doorPosition = POS_DOWN;
      if (doorDirection != MOVING_UP)
      {
        doorDirection = MOVING_UP;
        moveZero = millis();
      }
      digitalWrite(M_FWD, HIGH);
      digitalWrite(M_BACK, LOW);
      // Klappe ist unten oder
      // Klappe ist zu leicht
    }

    else if (!up && low)
    { // Klappe hängt fest
      Serial.println("Klappe steckt fest, Motor fährt weiter...");
      if (doorDirection == STANDING)
      {
        if (pos == POS_DOWN)
        {
          moveZero = millis();
          doorDirection = MOVING_DOWN;
          digitalWrite(M_FWD, LOW);
          digitalWrite(M_BACK, HIGH);
        }
        else
        {
          moveZero = millis();
          doorDirection = MOVING_UP;
          digitalWrite(M_FWD, HIGH);
          digitalWrite(M_BACK, LOW);
        }
      }

      if (doorPosition != POS_BLOCKED)
      {
        doorPosition = POS_BLOCKED;
        blockZero = millis();
      }
      // timerBlocked = timerBegin(BLOCKED_TIMER, t_err_open, true); // TODO: Implement Blocked Timer
    }

    else if (up && low)
    {
      Serial.println("Klappe oben, Motor fährt runter...");
      doorPosition = POS_UP;
      if (doorDirection != MOVING_DOWN)
      {
        doorDirection = MOVING_DOWN;
        moveZero = millis();
      }
      digitalWrite(M_FWD, LOW);
      digitalWrite(M_BACK, HIGH);
      // Klappe ist oben
    }

    else if (up && !low)
    { // Klappe fährt grade
      doorPosition = POS_DRIVING;

      if (doorDirection == MOVING_DOWN)
      {
        Serial.println("Klappe fährt grade herunter, Motor fährt weiter...");
      }
      else if (doorDirection == MOVING_UP)
      {
        Serial.println("Klappe fährt grade hoch, Motor fährt weiter...");
      }

      if (doorDirection == STANDING)
      {
        if (pos == POS_DOWN)
        {
          moveZero = millis();
          doorDirection = MOVING_DOWN;
          digitalWrite(M_FWD, LOW);
          digitalWrite(M_BACK, HIGH);
        }
        else
        {
          moveZero = millis();
          doorDirection = MOVING_UP;
          digitalWrite(M_FWD, HIGH);
          digitalWrite(M_BACK, LOW);
        }
      }
      // timerMoving = timerBegin(MOVING_TIMER, t_err_open, true); // TODO: Implement Moving Timer
    }

    //======================TIMER CHECKS========================

    if (doorPosition == POS_BLOCKED)
    {
      // Klappe ist blockiert
      if (millis() - blockZero > (t_err_block * 1000))
      {
        digitalWrite(LED, HIGH); // TODO: ERROR HANDLING
        errorFlag = TIMER_BLOCKED_ELAPSED;
        break;
      }
    }

    if (doorDirection == MOVING_DOWN && (millis() - moveZero > (t_err_close * 1000)))
    {
      // Klappe fährt zu lange runter
      digitalWrite(LED, HIGH);
      doorPosition = POS_BLOCKED;
      errorFlag = TIMER_CLOSE_ELAPSED;
      break;
    }

    if (doorDirection == MOVING_UP && (millis() - moveZero > (t_err_close * 1000)))
    {
      digitalWrite(LED, HIGH);
      doorPosition = POS_BLOCKED;
      // Klappe fährt zu lange hoch
      errorFlag = TIMER_OPEN_ELAPSED;
      break;
    }

    //=====================END WHILE================
  }
  digitalWrite(M_FWD, LOW);
  digitalWrite(M_BACK, LOW);
  doorDirection = STANDING;

  lastMove = t_now;
  lastDoorPosition = doorPosition;
  memory.putBytes(keyLastMove, &lastMove, sizeof(time_t));
  memory.putBytes(keyLastMovePosition, &doorPosition, sizeof(KLAPPENPOSITION));

  if (F_init_last_move == 0)
  {
    memory.putInt("init_last_move", 1); // erste bewegung hat stattgefunden
  }

  delay(50); // Motor darf sich nicht mehr drehen
  statusabfrage();
}

uint64_t GPIO_wake_up_reason()
{
  uint64_t GPIO_reason = esp_sleep_get_ext1_wakeup_status();
  Serial.print("GPIO that triggered the wake up: GPIO ");
  GPIO_reason = (log(GPIO_reason)) / log(2);
  Serial.println(GPIO_reason);
  return GPIO_reason;
}

void saveAlarmValues()
{
  int i;
  byte wd;
  for (i = 0; i < 7; i++)
  {
    wd = weekdays[i];
    if (day_bitmask & wd)
    {
      if (modeset == MODESET_OPENING)
      {
        openingAlarms[i].delay = 0;
        openingAlarms[i].hour = hour_o;
        openingAlarms[i].minute = minute_o;
        openingAlarms[i].mode = (openingMode)mode_o;
        openingAlarms[i].lux = lux_o;

        Serial.println("Saving open alarms");
      }

      else
      {
        closingAlarms[i].delay = 0;
        closingAlarms[i].hour = hour_c;
        closingAlarms[i].minute = minute_c;
        closingAlarms[i].mode = (openingMode)mode_c;
        closingAlarms[i].lux = lux_c;

        Serial.println("Saving close alarms");
      }
    }
  }

  memory.putBytes(keyOpenAlarms, openingAlarms, sizeof(doorDayAlarm_t[7]));
  memory.putBytes(keyCloseAlarms, closingAlarms, sizeof(doorDayAlarm_t[7]));
  memory.putInt("init_alarm", 1);
  Serial.println("SAVED");
}

void writeValuesToFlash()
{
  uint8_t ram = 0b00000000;
  if (ramCounter > 15)
  {
    ramCounter = 15;
  }
  ram = ram | ramCounter;
  ram = ram | ((LDRFlag << 4) & 0b00110000);
  ram = ram | ((alarmFlag << 6) & 0b11000000);

  rtc.writeRegister(RV8803_RAM, ram);
}

void setAlarmsDefault()
{
  doorDayAlarm_t buf_o;
  doorDayAlarm_t buf_c;

  buf_o.delay = 0;
  buf_o.done = 0;
  buf_o.hour = 9;
  buf_o.minute = 30;
  buf_o.mode = NICHT;
  buf_o.lux = 0;

  buf_c.delay = 0;
  buf_c.done = 0;
  buf_c.hour = 20;
  buf_c.minute = 30;
  buf_c.mode = NICHT;
  buf_c.lux = 0;

  int i;
  for (i = 0; i < 7; i++)
  {
    openingAlarms[i] = buf_o;
    closingAlarms[i] = buf_c;
  }
}

void readValuesFromFlash()
{
  // char buf_o[sizeof(doorDayAlarm_t[7])];

  F_init_flag = memory.getInt("init_flag", 0);
  if (F_init_flag == 0)
  { // keine flags gesetzt bis jeztz
    LDRFlag = POS_BLOCKED;
    alarmFlag = POS_BLOCKED;
    ramCounter = 0;
  }
  else
  { // Flags aus Speicher lesen
    uint8_t ram = rtc.readRegister(RV8803_RAM);
    if (ram == false)
    {
      ram = 0b00000000;
    }
    ramCounter = ram & 0b00001111;
    LDRFlag = (KLAPPENPOSITION)((ram & 0b00110000) >> 4);
    alarmFlag = (KLAPPENPOSITION)((ram & 0b11000000) >> 6);
  }

  F_init_last_move = memory.getInt("init_last_move", 0);
  if (F_init_last_move == 0)
  {
    // keine Bewegung bisher
    lastMove = 0;
    lastDoorPosition = POS_BLOCKED;
  }
  else
  { // Letzte Bewegung aus Speicher lesen
    memory.getBytes(keyLastMove, &lastMove, sizeof(time_t));
    memory.getBytes(keyLastMovePosition, &lastDoorPosition, sizeof(KLAPPENPOSITION));
  }

  F_init_NOX = memory.getInt("init_NOX", 0);

  F_init_alarm = memory.getInt("init_alarm", 0);
  if (F_init_alarm == 0)
  {
    // set Alarms to default
    setAlarmsDefault();
    Serial.println("No values to read --> default");
    memory.putBytes(keyOpenAlarms, openingAlarms, sizeof(doorDayAlarm_t[7]));
    memory.putBytes(keyCloseAlarms, closingAlarms, sizeof(doorDayAlarm_t[7]));
  }
  else
  {
    // char buffer[sizeof(doorDayAlarm_t[7])];

    memory.getBytes(keyOpenAlarms, openingAlarms, 7 * sizeof(doorDayAlarm_t));
    memory.getBytes(keyCloseAlarms, closingAlarms, 7 * sizeof(doorDayAlarm_t));
  }

  for (int i = 0; i < 7; i++)
  {
    Serial.println("Func: readValuesFromFlash(), Printing Alarms:\n\n");
    Serial.println("Alarms read --> printing opening Alarms");
    Serial.print("Minute on day");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(openingAlarms[i].minute);
    Serial.print("Mode on day");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(openingAlarms[i].mode);
  }

  Serial.println("Alarms read --> printing closing Alarms");
  for (int i = 0; i < 7; i++)
  {
    Serial.print("Minute on day");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(closingAlarms[i].minute);
    Serial.print("Mode on day");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(closingAlarms[i].mode);
  }
}

void alarmWakeupHandling()
{
  switch (alarmFlag)
  {
  case POS_DOWN:
    moveMotor(POS_DOWN);
    break;

  case POS_UP:
    moveMotor(POS_UP);
    break;

  case POS_BLOCKED:
    statusabfrage();
    break;

  default:
    Serial.println("Func: alarmWakeupHandling(): ERROR: wrong alarmFlag");
    digitalWrite(LED, HIGH);
    statusabfrage();
    break;
  }
}

bool checkLongPressStartup(int sw) // when the up or down button is pressed long at startup ---> klappe wird manuell bewegt
{
  int i = 0; // safety escape
  while (millis() - previousMillis <= LONG_PRESS_STARTUP_DURATION)
  { // TODO: test timing
    if (sw == SW_BACK && digitalRead(SW_BACK) == HIGH)
    {
      // DOWN switch is being pressed
    }
    else if (sw == SW_FWD && digitalRead(SW_FWD) == HIGH)
    {
      // UP switch is being pressed
    }
    else
    {
      return false; // Button was released to early
    }
    delay(1);
    i = i + 1;
    if (i > 5000)
    {
      return false;
    }
  }
  // while loop complete --> button was pressed longer than LONG_PRESS_STARTUP_DURATION
  return true;
}

void timerWakeupHandling()
{
  // TODO: timer wakeup handling implementieren
  activateLDR();
  PREP_FOR_DEEP_SLEEP
}

// TODO: implement alarm recognition while menu is active (CPU awake)

void wakeupHandling()
{
  esp_sleep_wakeup_cause_t wakeup_reason;
  uint64_t w_pin;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println("Wakeup caused by external buttons");
    w_pin = GPIO_wake_up_reason();
    switch (w_pin)
    {
    case CLK_INT:
      manualFlag = 0;
      menuActive = 0;
      Serial.println("wakeup by alarm");
      alarmWakeupHandling();
      break;

    case SW_BACK:
      // TODO: implement wakeup by btn down
      delay(50);
      previousMillis = millis();
      if (checkLongPressStartup(SW_BACK))
      {
        Serial.println("Klappe wurde manuell geschlossen. Bewege Motor aufwärts...");
        manualFlag = 1;
        menuActive = 0;
        moveMotor(POS_DOWN);
      }
      else
      {
        menuActive = 1;
      }
      break;

    case SW_FWD:
      // TODO: implement wakeup by btn up
      delay(50);
      previousMillis = millis();
      if (checkLongPressStartup(SW_FWD))
      {
        Serial.println("Klappe wurde manuell geöffnet. Bewege Motor aufwärts...");
        manualFlag = 1;
        menuActive = 0;
        moveMotor(POS_UP);
      }
      else
      {
        menuActive = 1;
      }
      break;

    case SW_EXIT:
      menuActive = 1;
      manualFlag = 0;
      // TODO: implement wakeup by btn left
      break;

    case SW_SELECT:
      menuActive = 1;
      manualFlag = 0;
      // TODO: implement wakeup by btn right
      break;

    default:
      // TODO: error handling andere knöpfe
      break;
    }
    break;

  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("Wakeup caused by timer");
    manualFlag = 0;
    menuActive = 0;
    timerWakeupHandling();
    break;
  default:
    Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
    menuActive = 1;
    manualFlag = 0;
    // TODO: default wakeup handling
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
  pinMode(LCD_BL_EN, INPUT);
  pinMode(LED, OUTPUT);

  delay(5);

  Serial.begin(115200);
  lcd.begin(16, 2);
  Wire.setPins(I2C_SDA, I2C_SCL);
  Wire.begin();
  memory.begin("storage", false);

  if (rtc.begin() == false)
  {
    Serial.println("Device not found. Please check wiring. Freezing.");

    // TODO: Error handling
  }
  rtc.set24Hour(); // TODO: 12hour mode
  readValuesFromFlash();
  timeUpdate();
  dateUpdate();

  wakeupHandling();

  // ========= Der folgende Teil wird nur erreicht, wenn die Wakeup-Quelle kein Knopf war =========
  LCD_ON;
  digitalWrite(LDR_EN, HIGH);
  delay(50);
  lcd.clear();
  lcd.setCursor(6, 0);
  lcd.print("NOX");
  delay(100);
  if (F_init_NOX == 0)
  {
    menu_id = 001; // TODO: richtige weiterleitung ins init menu gewährleisten
  }
  else
  {  
    statusabfrage(); // TODO: testen ob alles korrekt geladen wird und klappe nicht ausgeht.
    menu_id = 000;
  }


  blinkZero = millis();
}

void loop()
{
  if (digitalRead(CLK_INT) == 1)
  { // Alarm ist während der Menülaufzeit gekommen //TODO: Was wenn alarm auf 00:01 gestellt wurde? --> verhindern oder Fehlerreaktion (immer auf 00:02 aufrunden zb)
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(MenuItemsSpecial[1][lang]); // Klappe ist beschäftigt, menu wird unterbrochen
    alarmWakeupHandling();
  }
  else
  {
    // Check ob menu zu lange inaktiv ist --> schlafen
    if (millis() - lastPress >= LCD_OFF_TIME)
    {
      pinMode(LCD_BL_EN, INPUT);
    }
    else
    {
      LCD_ON
    }

    if (millis() - lastPress >= NOX_SLEEP_TIME)
    {
      menuActive = 0;
      statusabfrage();
    }

    // Check ob menü refresh nötig ist
    if (((millis() - blinkZero) / 500) % 2 == 0)
    {
      if (blink == 1 && ((menu_id == 300 && timeSetMode == SETNOTHING) || (menu_id == 000) || (menu_id == 810) || (menu_id == 001)))
      {
        menuRefreshFlag = true;
        blinkCount += 1;
        if (blinkCount > blinkFactor * 8 - 1)
        {
          blinkCount = 0;
        }
      }
      blink = 0;
    }
    else
    {
      if (blink == 0 && ((menu_id == 300 && timeSetMode == SETNOTHING) || (menu_id == 000) || (menu_id == 810) || (menu_id == 001)))
      {
        menuRefreshFlag = true;
      }
      blink = 1;
    }

    menuUpdate();
    // delay(50);
  }

  //// Serial.print("menuId ");
  //// Serial.println(menu_id);
  // delay(50); // simulate a delay as if other tasks are running
}
