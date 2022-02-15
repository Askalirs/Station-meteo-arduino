#include <Arduino.h>
#include <EEPROM.h>
//#include <avr/pgmspace.h> //pour libérer de l'espace dans la SRAM

//défini les commandes
#define COMMANDS(F) \
  F(LOG_INTERVALL), \
  F(FILE_MAX_SIZE), \
  F(RESET), \
  F(VERSION), \
  F(TIMEOUT), \
  F(LUMIN), \
  F(LUMIN_LOW), \
  F(LUMIN_HIGH), \
  F(TEMP_AIR), \
  F(MIN_TEMP_AIR), \
  F(MAX_TEMP_AIR), \
  F(HYGR), \
  F(HYGR_MINT), \
  F(HYGR_MAXT), \
  F(PRESSURE), \
  F(PRESSURE_MIN), \
  F(PRESSURE_MAX), \
  F(CLOCK), \
  F(DATE), \
  F(DAY)

//défini les jours de la semaine
#define DAYS(F) \
  F(SUN), \
  F(MON), \
  F(TUE), \
  F(WED), \
  F(THU), \
  F(FRI), \
  F(SAT)

//défini les enum des commandes pour le switch
#undef F
#define F(e) e
enum Commands {COMMANDS(F), ERROR_CMD};
enum Days {DAYS(F)};
#undef F

//crée les strings qui correspondent au commandes
#define F(s) #s
String CommandsNames[] = {COMMANDS(F)};
String DaysNames[] = {DAYS(F)};
#undef F

#define cmd_separator "="
#define clock_separator ":"
#define date_separator ","
#define separator_error -1

#define SN "CVZ18W5Y"
#define VER "1.0.0"

void feedback_print(String);
String cmd_processing(String*);
Commands name_to_command(String*);
Days name_to_day(String*);
void check_command(String *ptr);

/*void setup() {
  Serial.begin(9600);
}

void loop() {
  if (Serial.available()){
    String SerialReponse = Serial.readString();
    check_command(&SerialReponse);
  }
}*/

void check_command(String *cmd){
  //la fonction modifie directement la valeur du string cmd
  String cmdArgs = cmd_processing(cmd);
  int cmdValue;
  name_to_command(cmd) == CLOCK || 
    name_to_command(cmd) == DATE || 
    name_to_command(cmd) == DAY ? 0 : 
      cmdValue = cmdArgs.toInt();

  switch (name_to_command(cmd)){

  case LOG_INTERVALL:
    if(cmdValue == separator_error){
      EEPROM.get(0, cmdValue);
      feedback_print(String(cmdValue));
    } else {
      EEPROM.put(0, cmdValue);
      feedback_print("NONE");
    }
    break;

  case FILE_MAX_SIZE:
    if(cmdValue == separator_error){
      EEPROM.get(2, cmdValue);
      feedback_print(String(cmdValue));
    } else {
      EEPROM.put(2, cmdValue);
      feedback_print("NONE");
    }
    break;

  case RESET:
    EEPROM.put(0, 10); //LOG_INTERVALL
    EEPROM.put(2, 4096); //FILE_MAX_SIZE
    EEPROM.put(4, 30); //TIMEOUT
    EEPROM.put(5, ((boolean)1)); //LUMIN
    EEPROM.put(6, 255); //LUMIN_LOW
    EEPROM.put(8, 768); //LUMIN_HIGH
    EEPROM.put(10, ((boolean)1)); //TEMP_AIR
    EEPROM.put(11, ((int8_t)-10)); //MIN_TEMP_AIR
    EEPROM.put(12, ((int8_t)60)); //MAX_TEMP_AIR
    EEPROM.put(13, ((boolean)1)); //HYGR
    EEPROM.put(14, ((int8_t)0)); //HYGR_MINT
    EEPROM.put(15, ((int8_t)50)); //HYGR_MAXT
    EEPROM.put(16, ((boolean)1)); //PRESSURE
    EEPROM.put(17, 850); //PRESSURE_MIN
    EEPROM.put(19, 1080); //PRESSURE_MAX

    Serial.println("Valeurs réinitialisées par défaut.");
    break;

  case VERSION:
    Serial.println("V" VER);
    Serial.println("S/N: " SN);
    break;
  
  case TIMEOUT:
    if(cmdValue == separator_error){
      EEPROM.get(4, cmdValue);
      feedback_print(String(cmdValue));
    } else {
      EEPROM.put(4, cmdValue);
      feedback_print("NONE");
    }
    break;
  
  case LUMIN:
    if(cmdValue == separator_error){
      boolean temp = ((boolean)cmdValue);
      EEPROM.get(5, temp);
      feedback_print(String(temp));
    } else if(cmdValue == 0 || cmdValue == 1){
      EEPROM.put(5, ((boolean)cmdValue));
      feedback_print("NONE");
    }
    break;

  case LUMIN_LOW:
    if(cmdValue == separator_error){
      EEPROM.get(6, cmdValue);
      feedback_print(String(cmdValue));
    } else if(0 <= cmdValue && cmdValue <= 1023){
      EEPROM.put(6, cmdValue);
      feedback_print("NONE");
    }
    break;
  
  case LUMIN_HIGH:
    if(cmdValue == separator_error){
      EEPROM.get(8, cmdValue);
      feedback_print(String(cmdValue));
    } else if(0 <= cmdValue && cmdValue <= 1023){
      EEPROM.put(8, cmdValue);
      feedback_print("NONE");
    }
    break;
  
  case TEMP_AIR:
    if(cmdValue == separator_error){
      boolean temp = ((boolean)cmdValue);
      EEPROM.get(10, temp);
      feedback_print(String(temp));
    } else if(cmdValue == 0 || cmdValue == 1){
      EEPROM.put(10, ((boolean)cmdValue));
      feedback_print("NONE");
    }
    break;
  
  case MIN_TEMP_AIR:
    if(cmdValue == separator_error){
      int8_t temp = ((int8_t)cmdValue);
      EEPROM.get(11, temp);
      feedback_print(String(temp));
    } else if(-40 <= cmdValue && cmdValue <= 85){
      EEPROM.put(11, ((int8_t)cmdValue));
      feedback_print("NONE");
    }
    break;
  
  case MAX_TEMP_AIR:
    if(cmdValue == separator_error){
      int8_t temp = ((int8_t)cmdValue);
      EEPROM.get(12, temp);
      feedback_print(String(temp));
    } else if(-40 <= cmdValue && cmdValue <= 85){
      EEPROM.put(12, ((int8_t)cmdValue));
      feedback_print("NONE");
    }
    break;
  
  case HYGR:
    if(cmdValue == separator_error){
      boolean temp = ((boolean)cmdValue);
      EEPROM.get(13, temp);
      feedback_print(String(temp));
    } else if(0 == cmdValue || cmdValue == 1){
      EEPROM.put(13, ((boolean)cmdValue));
      feedback_print("NONE");
    }
    break;
  
  case HYGR_MINT:
    if(cmdValue == separator_error){
      int8_t temp = ((int8_t)cmdValue);
      EEPROM.get(14, temp);
      feedback_print(String(temp));
    } else if(-40 <= cmdValue && cmdValue <= 85){
      EEPROM.put(14, ((int8_t)cmdValue));
      feedback_print("NONE");
    }
    break;
  
  case HYGR_MAXT:
    if(cmdValue == separator_error){
      int8_t temp = ((int8_t)cmdValue);
      EEPROM.get(15, temp);
      feedback_print(String(temp));
    } else if(-40 <= cmdValue && cmdValue <= 85){
      EEPROM.put(15, ((int8_t)cmdValue));
      feedback_print("NONE");
    }
    break;
  
  case PRESSURE:
    if(cmdValue == separator_error){
      boolean temp = ((boolean)cmdValue);
      EEPROM.get(16, temp);
      feedback_print(String(temp));
    } else if(0 == cmdValue || cmdValue == 1){
      EEPROM.put(16, ((boolean)cmdValue));
      feedback_print("NONE");
    }
    break;
  
  case PRESSURE_MIN:
    if(cmdValue == separator_error){
      EEPROM.get(17, cmdValue);
      feedback_print(String(cmdValue));
    } else if(300 <= cmdValue && cmdValue <= 1100){
      EEPROM.put(17, cmdValue);
      feedback_print("NONE");
    }
    break;
  
  case PRESSURE_MAX:
    if(cmdValue == separator_error){
      EEPROM.get(19, cmdValue);
      feedback_print(String(cmdValue));
    } else if(300 <= cmdValue && cmdValue <= 1100){
      EEPROM.put(19, cmdValue);
      feedback_print("NONE");
    }
    break;
  
  case CLOCK:
    if(cmdArgs == String(separator_error)){
      Serial.println("Clock " + String(RTC.getHours()) + ":" + String(RTC.getMinutes()) + ":" + String(RTC.getSeconds()));
    } else {
      byte time[3];
      cmdArgs += clock_separator;

      for(byte i = 0; i < sizeof(time); i++){
        if(cmdArgs.indexOf(clock_separator) == -1){
          i == 1 ? time[1] = RTC.getMinutes() : 0;
          i == 2 ? time[2] = RTC.getSeconds() : 0;
        } else {
          time[i] = cmdArgs.substring(0, cmdArgs.indexOf(clock_separator)).toInt();
          cmdArgs = cmdArgs.substring(cmdArgs.indexOf(clock_separator) + 1, cmdArgs.length());
        }
      }

      0 <= time[0] && time[0] <= 23 ? 0 : time[0] = RTC.getHours();
      0 <= time[1] && time[1] <= 59 ? 0 : time[1] = RTC.getMinutes();
      0 <= time[2] && time[2] <= 59 ? 0 : time[2] = RTC.getSeconds();
      RTC.setTime(time[0], time[1], time[2]);
      Serial.println("Heure modifié.");
    }
    break;
  
  case DATE:
    if(cmdArgs == String(separator_error)){
      Serial.println("Date " + String(RTC.getMonth()) + "," + String(RTC.getDay()) + "," + String(RTC.getYear()));
    } else {
      int date[3];

      for(byte i = 0; i < sizeof(date) / sizeof(int); i++){
        date[i] = cmdArgs.substring(0, cmdArgs.indexOf(date_separator)).toInt();
        cmdArgs = cmdArgs.substring(cmdArgs.indexOf(date_separator) + 1, cmdArgs.length());
      }

      1 <= date[0] && date[0] <= 12 ? 0 : date[0] = RTC.getMonth();
      1 <= date[1] && date[1] <= 31 ? 0 : date[1] = RTC.getDay();
      2000 <= date[2] && date[2] <= 2099 ? 0 : date[2] = RTC.getYear();
      RTC.setMonth(date[0]);
      RTC.setDay(date[1]);
      RTC.setYear(date[2]);
      Serial.println("Date modifié.");
    }
    break;
  
  case DAY:
    if(cmdArgs == String(separator_error)){
      Serial.println("Jour : " + DaysNames[RTC.getWeek() - 1]);
    } else {
      String dayName = cmdArgs.substring(0, cmdArgs.length() - 1);

      RTC.setWeek(name_to_day(&dayName) + 1);
    }
    break;

  case ERROR_CMD:
    Serial.println("Cette commande n'est pas reconnu.");
    break;
  }
}

//converti la commande de chaine de caractère en enum
Commands name_to_command(String *commandName){
  unsigned int cmd;
  for(cmd = 0; cmd < ERROR_CMD; cmd++){
    if(CommandsNames[cmd] == *commandName) return static_cast<Commands>(cmd);
  }

  return ERROR_CMD;
}

//converti le jour de chaine de caractère en enum
Days name_to_day(String *dayName){
  unsigned int day;
  for(day = 0; day < ERROR_CMD; day++){
    if(DaysNames[day] == *dayName){
      Serial.println("Jour modifié.");
      return static_cast<Days>(day);
    }
  }

  //si le jour spécifié n'existe pas on renvoie le jour par défaut
  return static_cast<Days>(RTC.getWeek());
}

//cette fonction sépare le nom de la commande et l'argument
String cmd_processing(String *cmd){
  if((*cmd).indexOf(cmd_separator) != -1){
    String arg = (*cmd).substring((*cmd).indexOf(cmd_separator) + 1, (*cmd).length());
    *cmd = (*cmd).substring(0, (*cmd).indexOf(cmd_separator));

    return arg;
  }

  //on enlève le dernier caractère mis par le serial
  *cmd = (*cmd).substring(0, (*cmd).length() - 1);
  return String(separator_error);
}

void feedback_print(String val){
  if(val == "NONE"){
    Serial.println("Valeur correctement modifiée.");
  } else
      Serial.println("La valeur actuelle de ce paramètre est : " + val);
}