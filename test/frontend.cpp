
#include "backend.h"
#include "global.h"
#include <math.h>

//globals
int brightness = -1;
char *brString;

doorDayAlarm_t openingAlarms[7];
doorDayAlarm_t closingAlarms[7];

// uint8_t luxMap[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
uint32_t voltMap[10] = {300, 600, 900, 1200, 1500, 1800, 2100, 2400, 2700, 3000};

int16_t Year = 2000;
int8_t Month;
int8_t Date;
int8_t DoW;
int8_t Hour;
int8_t Minute;
int8_t Second;

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
uint8_t nextLux;

int dammerungsverzogerung = 600; //TODO: Dämmerungsverzögerung implementieren
byte lux_debounce_number = 3; // wie oft muss die Lichtschwelle erreicht sein, bis die Klappe sich bewegt
int lux_debounce_time = 20; // Zeit in sekunden zwischen zwei positiven Lichtprüfungen
int t_delta_min = 600;   // Zeit in Sekunden die zwischen zwei betätigungen per Licht vergangen sein müssen
int t_sens = 120; // Zeit in Sekunden, die zwischen zwei Sensorprüfungen vergeht

int t_err_open = 60; // Maximale Zeit zum Öffnen
int t_err_close = 60; // Maximale Zeit zum Schließen

openingMode nextMode = NICHT;
KLAPPENPOSITION nextMove = POS_DOWN;


const byte weekdays[7] = {
  MONDAY,
  TUESDAY,
  WEDNESDAY,
  THURSDAY,
  FRIDAY,
  SATURDAY,
  SUNDAY
};

time_t lastMove, t_now;
//TODO: getValuesfrom EEprom: lastMove, openAlarm, closeAlarm



volatile int count; //timer variable
hw_timer_t *timerBlocked = NULL;
hw_timer_t *timerMoving = NULL;



//devices
ESP32AnalogRead adc_ldr;
ESP32AnalogRead adc_vbatt;
RV8803 rtc;
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);



String MenuItems[8][3] = {  // MenuItems[itemID][Language]
  {"\xEF" "ffnungsmodus", "Opening mode", "blub"},
  {"Schlie" "\xE2" "modus", "Closing mode", "blub"},
  {"Uhrzeit", "Time", "blub"},
  {"Datum", "Date", "blub"},
  {"Erweitert", "advanced", "blub"},
  {"Sprache", "language", "blub"},
  {"Reset", "reset", "blub"},
  {"EXIT", "EXIT", "EXIT"},
};

String MenuItemsDays[12][3] = {  // MenuItems[itemID][Language]
  {"Woche", "week", "blub"},
  {"Wochenende", "weekend", "blub"},
  {"Tageweise", "daily", "blub"},
  {"SA - SO", "Sat - Sun", "blub"},
  {"MO - FR", "Mon - Fri", "blub"},
  {"MO", "Mon", "blub"},
  {"DI", "Tue", "blub"},
  {"MI", "Wed", "blub"},
  {"DO", "Thu", "blub"},
  {"FR", "Fri", "blub"},
  {"SA", "Sat", "blub"},
  {"SO", "Sun", "blub"},
};

String MenuItemsMode[9][3] = {  // MenuItems[itemID][Language]
  {"LUX", "LUX", "blub"},
  {"LUX + ZEIT", "LUX + TIME", "blub"},
  {"ZEIT", "TIME", "blub"},
  {"MANUELL", "MANUAL", "blub"},
  {"Lichtschwelle", "LUX threshold", "blub"},
  {"Aktuell", "Current", "blub"},
  {"\xEF""ffnungszeit", "latest opening", "blub"},
  {"Schlie" "\xE2" "zeit", "earliest closing", "blub"},
  {"GESPEICHERT!", "SAVED!", "blub"},
};

template< typename T, size_t NumberOfSizeX, size_t NumberOfSizeY > 
size_t MenuItemsSize(T (&) [NumberOfSizeX][NumberOfSizeY]){ return NumberOfSizeX; }
int numberOfMenuItems = MenuItemsSize(MenuItems) - 1;
int currentMenuItem = 0;
int previousMenuItem = 1;
byte button_flag = 0;
unsigned long previousMillis = millis();
unsigned long blinkZero = millis();
byte blink = 0;


// ================================================
// ============== Statusvariablen: ================
// ================================================

uint8_t maxDayThisMonth = 31;

Language lang = DEUTSCH;
byte days_o = WEEK;
byte days_c = WEEK;
byte* days;

int cursor_tag = MON;

byte mode_o = LICHT_ZEIT;
byte mode_c = LICHT_ZEIT; 
byte* mode;


byte lux_o = 0;
byte lux_c = 0;
byte* lux;

int8_t hour_o = 0;
int8_t hour_c = 0;
int8_t* hour_set;

int8_t minute_o = 0;
int8_t minute_c = 0;
int8_t* minute_set;

Modeset modeset = MODESET_OPENING;
Setmode timeSetMode = SETNOTHING;

byte day_bitmask = 0b0000000; // MO_DI_MI_DO_FR_SA_SO

int menu_id = 100;

void saveAlarmValues();

// ================================================================================================
// ================================================================================================
// ================================================================================================

// ================================================================================================
// ========================================== FRONTEND ============================================
// ================================================================================================

// ================================================================================================
// ================================================================================================
// ================================================================================================

void timeUpdate(){
  tmElements_t tmels;

  tmels.Minute = rtc.getDate();
  tmels.Month = rtc.getMonth();
  tmels.Year = rtc.getYear();
  tmels.Wday = rtc.getWeekday();

  tmels.Hour = rtc.getHours();
  tmels.Minute = rtc.getMinutes();
  tmels.Second = rtc.getSeconds();

  t_now = makeTime(tmels);

}


void menuFunctions(BTNAction action)  // Ihre Menüfunktionen
{
  lcd.clear();
  Serial.print("Days Open: ");
  Serial.println(days_o);
  Serial.print("Days Close: ");
  Serial.println(days_c);
  
// ================================================
// ================ Hauptmenü: ====================
// ================================================

  if (menu_id == 000){
  Serial.println("Hauptmenü");

    struct tm *tmp = gmtime(&t_now);

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

//TODO: Statusmeldung implementieren (Montag - Sonntag)

// ================================================
// ============== Öffnungsmodus: ==================
// ================================================

  if (menu_id == 100){
  Serial.println("Öffnungsmodus");

    struct tm *tmp = gmtime(&t_now);

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


//====================== Tagesmodus auswählen (inactive)====
    else if(menu_id == 999) 
    {
      if(modeset == MODESET_CLOSING){
        days = &days_c;
        Serial.println("Schließmodus set");
      }
      else{        
        days = &days_o;
        Serial.println("Öffnungsmodus set");
      }


      switch (action)
      {
      case LEFT:
          (*days)--;
          if((*days) < 0){
              (*days) = 2;
          }
          break;

      case RIGHT:
          (*days)++;
        if((*days) > 2){
            (*days) = 0;
        }
        break;

      
      case SELECT:
          switch((*days)){
            case WEEK:
            menu_id = 800;
            break;

            case WEEKEND:
            menu_id = 111;
            break;

            case DAILY:
            menu_id = 112;
            break;

            default:
            //TODO: Default case
            break;
          }
          break;


      case EXIT:
          menu_id = 100; // Hauptmenu
          break;

      default:
          menu_id = 110;
          break;
    }

      lcd.setCursor(0, 0);
      lcd.print(">  ");
      lcd.setCursor(2, 0);
      if(modeset == MODESET_CLOSING){
        lcd.print(MenuItems[1][lang]);
      }
      else {
        lcd.print(MenuItems[0][lang]);
      }

      lcd.setCursor(0, 1);
      lcd.print("< ");
      lcd. setCursor(15, 1);
      lcd.print(">");
      lcd.setCursor(2, 1);
      switch ((*days)){
          case WEEK:
          lcd.print(MenuItemsDays[0][lang]);
          break;

          case WEEKEND:
          lcd.print(MenuItemsDays[1][lang]);
          break;

          case DAILY:
          lcd.print(MenuItemsDays[2][lang]);
          break;

          default:
          break;
      }
      lcd.setCursor(2, 1);
      lcd.cursor();
      action = NOTHING;
    }



//====================== Tage auswählen ====================
    else if(menu_id == 110) 
    {
      if(modeset == MODESET_CLOSING){
        Serial.println("Schließtage set");
      }
      else{        
        Serial.println("Öffnungstage set");
      }


      switch (action)
      {
      case LEFT:
          (cursor_tag)--;
          if((cursor_tag) < 0){
              (cursor_tag) = 7;
          }
          break;

      case RIGHT:
          (cursor_tag)++;
        if((cursor_tag) > 7){
            (cursor_tag) = 0;
        }
        break;

      
      case SELECT:
          if(cursor_tag < 7){
            day_bitmask = (day_bitmask ^ weekdays[cursor_tag]);
          }
          else{
            menu_id = 800;
          }
          break;


      case EXIT:
          if(modeset == MODESET_CLOSING){
            menu_id = 200;// Schließmodus
          }
          else{
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
      if(modeset == MODESET_CLOSING){
        lcd.print(MenuItems[1][lang]);
      }
      else {
        lcd.print(MenuItems[0][lang]);
      }

      if((day_bitmask & MONDAY) > 0){
        lcd.setCursor(0, 1);
        lcd.write(byte(M_INV));
        lcd.setCursor(1, 1);
        lcd.write(byte(o_INV));
      }
      else{
        lcd.setCursor(0, 1);
        lcd.print("Mo");
      }

      if((day_bitmask & TUESDAY) > 0){
        lcd.setCursor(2, 1);
        lcd.write(byte(D_INV));
        lcd.setCursor(3, 1);
        lcd.write(byte(i_INV));
      }
      else{
        lcd.setCursor(2, 1);
        lcd.print("Di");
      }

      if((day_bitmask & WEDNESDAY) > 0){
        lcd.setCursor(4, 1);
        lcd.write(byte(M_INV));
        lcd.setCursor(5, 1);
        lcd.write(byte(i_INV));
      }
      else{
        lcd.setCursor(4, 1);
        lcd.print("Mi");
      }

      if((day_bitmask & THURSDAY) > 0){
        lcd.setCursor(6, 1);
        lcd.write(byte(D_INV));
        lcd.setCursor(7, 1);
        lcd.write(byte(o_INV));
      }
      else{
        lcd.setCursor(6, 1);
        lcd.print("Do");
      }

      if((day_bitmask & FRIDAY) > 0){
        lcd.setCursor(8, 1);
        lcd.write(byte(F_INV));
        lcd.setCursor(9, 1);
        lcd.write(byte(r_INV));
      }
      else{
        lcd.setCursor(8, 1);
        lcd.print("Fr");
      }

      if((day_bitmask & SATURDAY) > 0){
        lcd.setCursor(10, 1);
        lcd.write(byte(S_INV));
        lcd.setCursor(11, 1);
        lcd.write(byte(a_INV));
      }
      else{
        lcd.setCursor(10, 1);
        lcd.print("Sa");
      }

      if((day_bitmask & SUNDAY) > 0){
        lcd.setCursor(12, 1);
        lcd.write(byte(S_INV));
        lcd.setCursor(13, 1);
        lcd.write(byte(o_INV));
      }
      else{
        lcd.setCursor(12, 1);
        lcd.print("So");
      }

      
      lcd.setCursor(15, 1);
      lcd.write(byte(0x7E));
      
    
      if(cursor_tag < 7){
        lcd.setCursor(cursor_tag*2, 1);
      }
      else{
        lcd.setCursor(15,1);
      }

      lcd.noCursor();
      lcd.blink();
    }


// ================================================
// ============== Schließmodus: ===================
// ================================================

  else if (menu_id == 200){
  Serial.println("Schließmodus");

    struct tm *tmp = gmtime(&t_now);

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

  else if (menu_id == 998){
  Serial.println("Uhrzeit");

    struct tm *tmp = gmtime(&t_now);

    switch (action)
    {
    case LEFT:
        menu_id = 200;
        break;

    case RIGHT:
        menu_id = 400;
      break;

    
    case SELECT:
        menu_id = 310; // Uhrzeit untermenu
        timeSetMode = SETHOUR;
        break;


    case EXIT:
        menu_id = 000; // Startscreen
        break;

    default:
      menu_id = 300;
      break;
    }


    lcd.setCursor(0, 0);
    lcd.print("3. ");
    lcd.setCursor(2, 0);
    lcd.print(MenuItems[2][lang]);

    lcd.setCursor(0, 1);
    lcd.printf("%02d", Hour);
    lcd.setCursor(2,1);
      if(blink){
        lcd.print(":");
      }
      else{
        lcd.print(" ");
      }
      
    lcd.setCursor(3,1);
    lcd.printf("%02d", Minute);

    lcd.noCursor();

    action = NOTHING;
  }

   else if (menu_id == 300){
      Serial.println("Uhrzeit set");

        if(timeSetMode == SETNOTHING){
          struct tm *tmp = gmtime(&t_now);
          //TODO: get current time
        }    

        switch (action)
        {
        case LEFT:
            if(timeSetMode == SETMINUTE){
              Minute--;
              if(Minute<0){
                Minute = 59;
              }
              menu_id = 300;
            }
            else if (timeSetMode == SETHOUR){
              Hour--;
              if(Hour<0){
                Hour = 23;
              }
              menu_id = 300;
            }
            else {
              timeSetMode = SETNOTHING;
              menu_id = 200;
            }
            break;

        case RIGHT:
            if(timeSetMode == SETMINUTE){
              Minute++;
              if(Minute>59){
                Minute = 0;
              }
              menu_id = 300;
            }
            else if (timeSetMode == SETHOUR){
              Hour++;
              if(Hour>23){
                Hour = 0;
              }
              menu_id = 300;
            }
            else {
              timeSetMode = SETNOTHING;
              menu_id = 400;
            }
            break;

        
        case SELECT:
            if(timeSetMode == SETMINUTE){
              //TODO: Implement saveTimeSetting();
              timeSetMode = SETNOTHING;
            }
            else if(timeSetMode == SETNOTHING){
              timeSetMode = SETHOUR;
            }
            else if(timeSetMode == SETHOUR){
              timeSetMode = SETMINUTE;
            }
            menu_id = 300; // 
            break;


        case EXIT:
            if(timeSetMode == SETNOTHING){
              menu_id = 000; // Hauptmenu
            }
            else{
              //TODO: Implement saveTimeSetting();
              timeSetMode = SETNOTHING;
            }
            break;

        default:
          menu_id = 300;
          break;
        }

        if(timeSetMode == SETNOTHING){
          lcd.setCursor(0, 0);
          lcd.print("3.");
        }
        else{
          lcd.setCursor(0, 0);
          lcd.print(">  ");
        }
        lcd.setCursor(2, 0);
        lcd.print(MenuItems[2][lang]);

        lcd.setCursor(2, 1);
        lcd.printf("%02d", Hour);
        lcd.setCursor(4,1);
        
        if(blink && timeSetMode == SETNOTHING){
          lcd.print(" ");
        }
        else{
          lcd.print(":");
        }
        
        lcd.setCursor(5,1);
        lcd.printf("%02d", Minute);

        if(timeSetMode == SETHOUR){
          lcd.setCursor(2,1);
          lcd.noCursor();
          lcd.blink();
        }
        else if (timeSetMode == SETMINUTE){
          lcd.setCursor(5,1);
          lcd.noCursor();
          lcd.blink();
        }
        else if( timeSetMode == SETNOTHING){
          lcd.noCursor();
          lcd.noBlink();
        }

        action = NOTHING;
      }



// ================================================
// ================= Datum: =======================
// ================================================
    else if (menu_id == 400){
      Serial.println("Datum set");

        if(timeSetMode == SETNOTHING){
          struct tm *tmp = gmtime(&t_now);
          //TODO: get current date
        } 

        if(timeSetMode != SETDAY){ // Set monthly days to max if month or year changed
          if(Date>maxDayThisMonth){
            Date = maxDayThisMonth;
          }
          menu_id = 400;
        }   

        switch (action)
        {
        case LEFT:


            if(timeSetMode == SETDAY){
              Date--;
              if(Date<1){
                Date = maxDayThisMonth;
              }
              menu_id = 400;
            }
            else if (timeSetMode == SETMONTH){
              Month--;
              if(Month<1){
                Month = 12;
              }
              menu_id = 400;
            }
            else if (timeSetMode == SETYEAR){
              Year--;
              if(Year<2000){
                Year = 2999;
              }
              menu_id = 400;
            }
            else {
              timeSetMode = SETNOTHING;
              menu_id = 300;
            }
            break;

        case RIGHT:
            
            if(timeSetMode == SETDAY){
              Date++;
              if(Date>maxDayThisMonth){
                Date = 1;
              }
              menu_id = 400;
            }
            else if (timeSetMode == SETMONTH){
              Month++;
              if(Month>12){
                Month = 1;
              }
              menu_id = 400;
            }
            else if (timeSetMode == SETYEAR){
              Year++;
              if(Year>2099){
                Year = 2000;
              }
              menu_id = 400;
            }
            else {
              timeSetMode = SETNOTHING;
              menu_id = 500;
            }
            
            break;

        
        case SELECT:
            if(timeSetMode == SETDAY){
              timeSetMode = SETMONTH;
            }
            else if(timeSetMode == SETMONTH){
              timeSetMode = SETYEAR;
            }
            else if(timeSetMode == SETYEAR){
              //TODO: Implement saveTimeSetting();
              timeSetMode = SETNOTHING;
            }
            else if(timeSetMode == SETNOTHING){
              timeSetMode = SETDAY;
            }
            menu_id = 400; // 
            break;


        case EXIT:
            if(timeSetMode == SETNOTHING){
              menu_id = 000; // Hauptmenu
            }
            else{
              //TODO: Implement saveTimeSetting();
              timeSetMode = SETNOTHING;
            }
            break;

        default:
          menu_id = 400;
          break;
        }

        //Calculate max day:
        if(Month == 1 || Month == 3 || Month == 5 || Month == 7 || Month == 8 || Month == 10 || Month == 12 ){
          maxDayThisMonth = 31;
        }
        else if(Month == 2){
          if(Year % 4 == 0){// leap year correction
            maxDayThisMonth = 29;
          }
          else{
            maxDayThisMonth = 28;
          }
        }
        else{
          maxDayThisMonth = 30;
        }

        if(timeSetMode == SETNOTHING){
          lcd.setCursor(0, 0);
          lcd.print("4.");
        }
        else{
          lcd.setCursor(0, 0);
          lcd.print(">  ");
        }
        lcd.setCursor(2, 0);
        lcd.print(MenuItems[3][lang]);

        lcd.setCursor(2, 1);
        lcd.printf("%02d.", Date);
        lcd.setCursor(5,1);
        lcd.printf("%02d.", Month);    
        lcd.setCursor(8,1);
        lcd.printf("%04d", Year);

        if(timeSetMode == SETDAY){
          lcd.setCursor(2,1);
          lcd.noCursor();
          lcd.blink();
        }
        else if (timeSetMode == SETMONTH){
          lcd.setCursor(5,1);
          lcd.noCursor();
          lcd.blink();
        }
        else if (timeSetMode == SETYEAR){
          lcd.setCursor(8,1);
          lcd.noCursor();
          lcd.blink();
        }
        else if( timeSetMode == SETNOTHING){
          lcd.noCursor();
          lcd.noBlink();
        }

        action = NOTHING;
      }




// ================================================
// =============== Erweitert: =====================
// ================================================

  else if (menu_id == 500){
  Serial.println("Erweitert");
  //TODO: Erweiterte einstellungen implementieren

    struct tm *tmp = gmtime(&t_now);

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
  else if (menu_id == 600){
  Serial.println("Sprache");
  //TODO: Sprache implementieren

    struct tm *tmp = gmtime(&t_now);

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
  else if (menu_id == 700){
  Serial.println("Reset");
  //TODO: Reset implementieren

    struct tm *tmp = gmtime(&t_now);

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
  else if (menu_id == 800){
      if(modeset == MODESET_CLOSING){
        mode = &mode_c;
        Serial.println("Schließmodus set");
      }
      else{        
        mode = &mode_o;
        Serial.println("Öffnungsmodus set");
      }


      switch (action)
      {
      case LEFT:
          (*mode)--;
          if((*mode) < 0){
              (*mode) = 3;
          }
          break;

      case RIGHT:
          (*mode)++;
        if((*mode) > 3){
            (*mode) = 0;
        }
        break;

      
      case SELECT:
          switch((*mode)){
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
            //TODO: Speichern!
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
      if(modeset == MODESET_CLOSING){
        lcd.print(MenuItems[1][lang]);
      }
      else {
        lcd.print(MenuItems[0][lang]);
      }

      lcd.setCursor(0, 1);
      lcd.print("< ");
      lcd. setCursor(15, 1);
      lcd.print(">");
      lcd.setCursor(2, 1);
      switch((*mode)){
            case LICHT:
            lcd.print(MenuItemsMode[0][lang]);
            break;

            case LICHT_ZEIT:
            lcd.print(MenuItemsMode[1][lang]);
            break;

            case ZEIT:
            lcd.print(MenuItemsMode[2][lang]);
            break;

            case NICHT:
            lcd.print(MenuItemsMode[3][lang]);
            break;
          }
      lcd.setCursor(2, 1);
      lcd.blink();
      lcd.noCursor();
    }

//============ Lichtschwelle einstellen ===========
   else if (menu_id == 810){
      if(modeset == MODESET_CLOSING){
        lux = &lux_c;
        mode = & mode_c;
        Serial.println("Lichtschließ set");
      }
      else{        
        lux = &lux_o;
        mode = & mode_o;
        Serial.println("Lichtschließ set");
      }


      switch (action)
      {
      case LEFT:
          (*lux)--;
          if((*lux) < MINLUX){
              (*lux) = MAXLUX;
          }
          break;

      case RIGHT:
          (*lux)++;
        if((*lux) > MAXLUX){
            (*lux) = MINLUX;
        }
        break;

      
      case SELECT:
      //TODO: Speichern!
          switch((*mode)){
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
      

      lcd. setCursor(6, 1);
      lcd.printf("%s:",MenuItemsMode[5][lang]);
      lcd.setCursor(15, 1);
      lcd.print(analogRead(LDR_VAL)); //TODO: analog read

      lcd.setCursor(0, 1);
      lcd.print("LUX:");
      lcd.setCursor(4, 1);
      lcd.print((*lux));
      lcd.setCursor(4, 1);
      lcd.blink();
      lcd.noCursor();
    }


//============ Zeitschwelle einstellen ===========
   else if (menu_id == 820){
      if(modeset == MODESET_CLOSING){
        hour_set = &hour_c;
        minute_set  = &minute_c;
        mode = & mode_c;
        Serial.println("Zeitschließ set");
      }
      else{        
        hour_set = &hour_o;
        minute_set  = &minute_o;
        mode = & mode_o;
        Serial.println("Zeitöffnung set");
      }


      switch (action)
        {
        case LEFT:
            if(timeSetMode == SETMINUTE){
              (*minute_set)--;
              if((*minute_set)<0){
                (*minute_set) = 59;
              }
              menu_id = 820;
            }
            else if (timeSetMode == SETHOUR){
              (*hour_set)--;
              if((*hour_set)<0){
                (*hour_set) = 23;
              }
              menu_id = 820;
            }
            else {
              timeSetMode = SETMINUTE;
              menu_id = 820;
            }
            break;

        case RIGHT:
            if(timeSetMode == SETMINUTE){
              (*minute_set)++;
              if((*minute_set)>59){
                (*minute_set) = 0;
              }
              menu_id = 820;
            }
            else if (timeSetMode == SETHOUR){
              (*hour_set)++;
              if((*hour_set)>23){
                (*hour_set) = 0;
              }
              menu_id = 820;
            }
            else if (timeSetMode == SETNOTHING){
              timeSetMode = SETHOUR;
              menu_id = 820;
            }
            break;

        
        case SELECT:
            if(timeSetMode == SETMINUTE){
              timeSetMode = SETNOTHING;
              menu_id = 820; // 
            }
            else if(timeSetMode == SETNOTHING){
              menu_id = 900;
              //TODO: Implement saveTimeCloseOpenSetting();
            }
            else if(timeSetMode == SETHOUR){
              timeSetMode = SETMINUTE;
              menu_id = 820; // 
            }
            
            break;


        case EXIT:
            if(timeSetMode == SETNOTHING){
              timeSetMode = SETMINUTE;
            }
            else if (timeSetMode == SETMINUTE){
              timeSetMode = SETHOUR;
            }
            else{
              if ((*mode) == LICHT_ZEIT || (*mode) == LICHT){
                menu_id = 810;
              }
              else {
                menu_id = 800;
              }
            }
            break;

        default:
          menu_id = 820;
          break;
        }

        
        lcd.setCursor(0, 0);
        if(modeset == MODESET_CLOSING){
          lcd.print(MenuItemsMode[7][lang]);
        }
        else{
          lcd.print(MenuItemsMode[6][lang]);
        }
        lcd.setCursor(2, 1);
        lcd.printf("%02d", (*hour_set));
        lcd.setCursor(4,1);

        lcd.print(":");
        
        
        lcd.setCursor(5,1);
        lcd.printf("%02d", (*minute_set));

        lcd.setCursor(15, 1);
        lcd.write(byte(0x7E));

        if(timeSetMode == SETHOUR){
          lcd.setCursor(2,1);
          lcd.noCursor();
          lcd.blink();
        }
        else if (timeSetMode == SETMINUTE){
          lcd.setCursor(5,1);
          lcd.noCursor();
          lcd.blink();
        }
        else if( timeSetMode == SETNOTHING){
          lcd.setCursor(15,1);
          lcd.noCursor();
          lcd.blink();
        }
    }




// ================================================
//=================== Speichern ===================
// ================================================

  else if(menu_id == 900){
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

    lcd.setCursor(0, 0);
    lcd.print(MenuItemsMode[9][lang]);
    lcd.noBlink();
    lcd.noCursor();
  }
}


void menuUpdate(){
  if(digitalRead(SW_SELECT) == HIGH && button_flag == 0)
  {
    Serial.println("Select pressed");
    menuFunctions(SELECT);
    button_flag = 1;
    previousMillis = millis();
  }
  if(digitalRead(SW_EXIT) == HIGH && button_flag == 0)
  {
      Serial.println("Exit pressed");
    menuFunctions(EXIT);
    button_flag = 1;
    previousMillis = millis();
  }
  if(digitalRead(SW_BACK) == HIGH && button_flag == 0)
  {
      Serial.println("Left pressed");
    menuFunctions(LEFT);
    button_flag = 1;
    previousMillis = millis();
  }
  else if(digitalRead(SW_FWD) == HIGH && button_flag == 0)
  {
      Serial.println("Right pressed");
    menuFunctions(RIGHT);
    button_flag = 1;
    previousMillis = millis();
  }
  else{
    menuFunctions(NOTHING);
  }
  
  if(millis() - previousMillis >= 400) 
  {
    previousMillis = millis();
    button_flag = 0;
  }

  if(((millis()-blinkZero)/500)%2 == 0){
    blink = 0;
  }
  else{
    blink = 1;
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



void activateLDR(){
  uint8_t ramCounter = rtc.readRegister(RV8803_RAM);
  if(LDRFlag == POS_DOWN){
    // als nächstes Schließen
    if(getLux() <= nextLux){
      ramCounter ++;
      if (ramCounter >= lux_debounce_number){ // Counter voll
        //TODO: Close Klappe
      }
      else{ // Counter noch nicht voll
        esp_sleep_enable_timer_wakeup(S_TO_uS * lux_debounce_time );

      }
    }
    else{
      esp_sleep_enable_timer_wakeup(S_TO_uS * t_sens);
      ramCounter = 0;
    }
  }
  else{
    // als nächstes Schließen
    if(getLux() <= nextLux){
      ramCounter ++;
      if (ramCounter >= lux_debounce_number){ // Counter voll
        //TODO: Close Klappe
      }
      else{ // Counter noch nicht voll
        esp_sleep_enable_timer_wakeup(S_TO_uS * lux_debounce_time );

      }
    }
    else{
      esp_sleep_enable_timer_wakeup(S_TO_uS * t_sens);
      ramCounter = 0;
    }
  }
  rtc.writeRegister(RV8803_RAM, ramCounter);

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

uint8_t VoltToLux(uint32_t mV){
  uint32_t m_mV = mV;
  int i;
  for (i = 0; i<9; i++){
    if (mV > voltMap[i]){
      if(mV < voltMap[i+1]){
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
  lux = VoltToLux(adc_ldr.readMiliVolts()); //TODO: LDR lux umsetzung implementieren
  digitalWrite(LDR_EN, LOW); //TODO: Prüfen ob das reicht
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


void setAlarmTomorrow0()
{
  if(weekday(t_now) >= 6){
        setAlarm(0, 0, 1);
      }
      else
      {
        setAlarm(weekday(t_now)+1, 0, 1);
      }
}





void setNextOpeningAlarm(){
  //TODO: Implement next opening logic
}




void setNextClosingAlarm(){
  nextMove = POS_DOWN;
  LDRFlag = POS_BLOCKED;
  alarmFlag = POS_BLOCKED;
  nextLux = closingAlarms[weekday(t_now)].lux;
  switch (closingAlarms[weekday(t_now)].mode)
  {
  case ZEIT:
    if( (closingAlarms[weekday(t_now)].hour*60 + closingAlarms[weekday(t_now)].minute) > (hour(t_now)*60 + minute(t_now)) ) //Alarm noch nicht vergangen
    {
      alarmFlag = nextMove;
      setAlarm(weekday(t_now), closingAlarms[weekday(t_now)].hour, closingAlarms[weekday(t_now)].minute);
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
    
    if(lastMove + t_delta_min > t_now+1) //Letzte Bewegung noch nicht lange genug her
    {
      LDRFlag = nextMove;
      esp_sleep_enable_timer_wakeup(S_TO_uS * (lastMove + t_delta_min - t_now) );
    } 
    else // Letzte Bewegung schon lange genug her um LDR zu aktivieren
    {
      LDRFlag = nextMove;
      activateLDR();
    }

    PREP_FOR_DEEP_SLEEP //TODO: Deep Sleep with alarm interrupt

    
    break;


  case LICHT_ZEIT:

    if( (closingAlarms[weekday(t_now)].hour*60 + closingAlarms[weekday(t_now)].minute) > (hour(t_now)*60 + minute(t_now)) ) //Alarm noch nicht vergangen
    {
      alarmFlag = nextMove;
      setAlarm(weekday(t_now), closingAlarms[weekday(t_now)].hour, closingAlarms[weekday(t_now)].minute);
      
      if(lastMove + t_delta_min > t_now+1) //Letzte Bewegung noch nicht lange genug her
      {
        LDRFlag = nextMove;
        esp_sleep_enable_timer_wakeup(S_TO_uS * (lastMove + t_delta_min - t_now) );
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

void statusabfrage(){
  switch (doorPosition){
    case POS_UP:
     // setNextClosingAlarm(); //TODO: ImpelementClosing alarm
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

void saveAlarmValues(){
  int i;
  byte wd;
  for (i = 0; i < 7; i++){
    wd = weekdays[i];
    if (day_bitmask & wd){
      if(modeset == MODESET_OPENING){
        openingAlarms[i].delay = 0;
        openingAlarms[i].hour = hour_o;
        openingAlarms[i].minute = minute_o;
        openingAlarms[i].mode = (openingMode)mode_o;
        openingAlarms[i].lux = lux_o;
      }
      else{
        closingAlarms[i].delay = 0;
        closingAlarms[i].hour = hour_c;
        closingAlarms[i].minute = minute_c;
        closingAlarms[i].mode = (openingMode)mode_c;
        closingAlarms[i].lux = lux_c;
      }
    }
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

  Wire.setPins(I2C_SDA, I2C_SCL);
  Wire.begin();

  Serial.begin(115200);
  lcd.begin(16,2);
  lcd.createChar(M_INV, M_inv);
  lcd.createChar(o_INV, o_inv);
  lcd.createChar(D_INV, D_inv);
  lcd.createChar(i_INV, i_inv);
  lcd.createChar(F_INV, F_inv);
  lcd.createChar(r_INV, r_inv);
  lcd.createChar(S_INV, S_inv);
  lcd.createChar(a_INV, a_inv);


  if (rtc.begin() == false)
  {
    Serial.println("Device not found. Please check wiring. Freezing.");

    //TODO: Error handling
  }
  LCD_ON;
  digitalWrite(LDR_EN, HIGH);
  delay(50);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Hallo");
  delay (500);
  blinkZero = millis();
  menu_id = 100;
  //print_wakeup_reason();
}


void loop()
{


  menuUpdate();
  Serial.print("menuId ");
  Serial.println(menu_id);

  delay(50); //simulate a delay as if other tasks are running
}

