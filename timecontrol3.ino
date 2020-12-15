#include <EEPROM.h>

#include "microLiquidCrystal_I2C.h"
LiquidCrystal_I2C lcd(0x27, 20, 4);
// адрес дисплея 0x3f или 0x27
// ширина дисплея (тут 20 символов)
// высота дисплея (тут 4 строки)

#define CLK 2
#define DT 3
#define SW 4
#include "GyverEncoder.h"
Encoder enc1(CLK, DT, SW);  // для работы c кнопкой


#define LASER_PWR 7
#define LASER_GND 6
#define LASER_S 5

#define LASER_SENS_GND 9
#define LASER_SENS_PWR 8
#define LASER_SENS_S A7
#define LASER_SENS_SD 18

unsigned long t;
boolean laserState = false;

const char *menu[][10]  = {
  {"Race","Sel racer","Cancel","Retry"},
  {"Set","Mode","Laps N","Ignore time","Save res","Sounds"},
  {"Test","All values","Time","Blink"},
};

class State{
  public:
    byte route = 1;
    byte subroute = 3;
    boolean active = false;
    boolean activeEntered = false;
    State(byte _route, byte _subroute, boolean _active) {
      route = _route;
      subroute = _subroute;
      if(!active && _active) setActive();
    };
    void displayState() {
      lcd.setCursor(0, 0);
      lcd.print("                           ");
      lcd.setCursor(0, 0);
      lcd.print(menu[route][0]);
      if(subroute) {
        if(subroute==1) {
          lcd.print("<-");
        } else {
          lcd.print("->");
        }
        lcd.print(menu[route][subroute]);
        if(active) {lcd.print(":");}
      }
    }
    void clearDisplay() {
      lcd.setCursor(0, 1); lcd.print("                    ");     
      lcd.setCursor(0, 2); lcd.print("                    ");
      lcd.setCursor(0, 3); lcd.print("                    ");
    }
    void menuPrev() {
      if(route>0) route--;
    }
    void menuNext() {
      if(route<2) route++;
    }    
    void menuEnter() {
      subroute=1;
    }    
    void submenuPrev() {
      if(subroute>0) {
        subroute--;
      }
    }
    void submenuNext() {
      if(route==0&&subroute<3) subroute++;
      if(route==1&&subroute<5) subroute++;
      if(route==2&&subroute<3) subroute++;
    }    
    void setActive() {
      active = true;
      activeEntered = true;
    }
    void setInactive() {
      active = false;
      activeEntered = false;
    }
};

class Event{
  public:
    boolean fired = false;
    unsigned long payloadLong = 0;
    unsigned int payloadInt = 0;
    String payloadString = "";
    void emit(unsigned long _payloadLong=0,unsigned int _payloadInt=0) {
      payloadLong = _payloadLong;
      payloadInt = _payloadInt;
      fired = true;
    }
    void absorb() {
      fired = false;
    }
};

class Config {
  public:
    unsigned int FS_KEY = 0; byte FS_KEY_addr = 0; // 0 2
    byte MODE = 1;  byte MODE_addr = 2; // 2  1
    unsigned int LAPS_N = 3; byte LAPS_N_addr = 3; // 3 2
    unsigned int SENSOR_IGNORE_TIME = 3000; byte SENSOR_IGNORE_TIME_addr = 5; // 5  2
    boolean MUTE = false; byte MUTE_addr = 7; // 7  1
    boolean SAVE_RESULTS = false; byte SAVE_RESULTS_addr = 8; // 7  1
    Config() {
      EEPROM.get(0, FS_KEY);
      if(FS_KEY!=12345) { // First start
        Serial.println();
        Serial.println("First start, writing defaults...");
        EEPROM.put(FS_KEY_addr, 12345);
        EEPROM.put(MODE_addr, MODE);
        EEPROM.put(LAPS_N_addr, LAPS_N);
        EEPROM.put(SENSOR_IGNORE_TIME_addr, SENSOR_IGNORE_TIME);
        EEPROM.put(MUTE_addr, MUTE);
        EEPROM.put(SAVE_RESULTS_addr, SAVE_RESULTS);
        Serial.println("Done");
        read();
        print();
      }
    }
    void read() {
      EEPROM.get(FS_KEY_addr, FS_KEY);
      EEPROM.get(MODE_addr, MODE);
      EEPROM.get(LAPS_N_addr, LAPS_N);
      EEPROM.get(SENSOR_IGNORE_TIME_addr, SENSOR_IGNORE_TIME);
      EEPROM.get(MUTE_addr, MUTE);
      EEPROM.get(SAVE_RESULTS_addr, SAVE_RESULTS);
    };

    void print() {
      Serial.println("Config: ");
      Serial.print("FS_KEY: "); Serial.println(FS_KEY);
      Serial.print("MODE: "); Serial.println(MODE);
      Serial.print("LAPS_N: "); Serial.println(LAPS_N);
      Serial.print("SENSOR_IGNORE_TIME: "); Serial.println(SENSOR_IGNORE_TIME);
      Serial.print("MUTE: "); Serial.println(MUTE);
      Serial.print("SAVE_RESULTS: "); Serial.println(SAVE_RESULTS);
      Serial.println();
    }

    void setMODE() {
      EEPROM.put(MODE_addr, MODE);
    }
    void setLAPS_N() {
      EEPROM.put(LAPS_N_addr, LAPS_N);
    }
    void setSENSOR_IGNORE_TIME() {
      EEPROM.put(SENSOR_IGNORE_TIME_addr, SENSOR_IGNORE_TIME);
    }
    void setMUTE() {
      EEPROM.put(MUTE_addr, MUTE);
    }
    void setSAVE_RESULTS() {
      EEPROM.put(SAVE_RESULTS_addr, SAVE_RESULTS);
    }

    void setMODE(byte value) {
      if(value == MODE) return;
      MODE = value;
      EEPROM.put(MODE_addr, MODE);
    }
    void setLAPS_N(unsigned int value) {
      if(value == LAPS_N) return;
      LAPS_N = value;
      EEPROM.put(LAPS_N_addr, LAPS_N);
    }
    void setSENSOR_IGNORE_TIME(unsigned int value) {
      if(value == SENSOR_IGNORE_TIME) return;
      SENSOR_IGNORE_TIME = value;
      EEPROM.put(SENSOR_IGNORE_TIME_addr, SENSOR_IGNORE_TIME);
    }
    void setMUTE(boolean value) {
      if(value == MUTE) return;
      MUTE = value;
      EEPROM.put(MUTE_addr, MUTE);
    }
    void setSAVE_RESULTS(boolean value) {
      if(value == SAVE_RESULTS) return;
      SAVE_RESULTS = value;
      EEPROM.put(SAVE_RESULTS_addr, SAVE_RESULTS);
    }

};

State state(2,1,true);

Event event0; 
Event event1; // Sensor
Event event2; // Encoder left
Event event3; // Encoder right
Event event4; // Encoder click
Event event5; // BT command recieved
Event events[] = {event0,event1,event2,event3,event4,event5};

Config config;

void setup() {

  Serial.begin(9600); 

  Config config;

  lcd.init();
  lcd.backlight();
  State state(2,1,true);

  enc1.setType(TYPE1);

  pinMode(LASER_PWR, OUTPUT);
  pinMode(LASER_GND, OUTPUT);
  pinMode(LASER_S, OUTPUT);

  pinMode(LASER_SENS_GND, OUTPUT);
  pinMode(LASER_SENS_PWR, OUTPUT);
  pinMode(LASER_SENS_S, INPUT);
  pinMode(LASER_SENS_SD, INPUT);

  digitalWrite(LASER_PWR, HIGH);
  digitalWrite(LASER_GND, LOW);
  digitalWrite(LASER_S, laserState);
  digitalWrite(LASER_S, HIGH);

  digitalWrite(LASER_SENS_GND, LOW);
  digitalWrite(LASER_SENS_PWR, HIGH);

  attachInterrupt(5, onSensor, RISING); 

  state.displayState();

  config.read();
  config.print();

}

void loop() {
  t = millis();

  eventEmmiter();
  handler();
  enc1.tick();
}

volatile unsigned long _onSensor_t = 0;
volatile unsigned int _onSensor_value = 0;
void onSensor() {
  _onSensor_t = t;
  _onSensor_value = analogRead(LASER_SENS_S);
}

void eventEmmiter() {

  // Sensor
  if(_onSensor_t) {
    events[1].emit(_onSensor_t,_onSensor_value);
    _onSensor_t = 0;
    _onSensor_value = 0;
  }

  // Encoder
  if (enc1.isLeft()) events[2].emit();
  if (enc1.isRight()) events[3].emit();
  if (enc1.isClick()) events[4].emit();

}

void handler() {
  if(events[1].fired) {
  }

  if(state.subroute==0) { // в меню первого уровня
    if(events[2].fired) {
      state.menuPrev();
      state.displayState();
    }
    if(events[3].fired) {
      state.menuNext();
      state.displayState();
    }
    if(events[4].fired) {
      state.menuEnter();
      state.displayState();
    }
  } else { // в меню второго уровня
    if(!state.active) { // пункт не выбран
      if(events[2].fired) {
        state.submenuPrev();
        state.displayState();
      }
      if(events[3].fired) {
        state.submenuNext();
        state.displayState();
      }
      if(events[4].fired) {
        state.setActive();
        state.clearDisplay();
        state.displayState();
      }
    } else { // пункт выбран

      if(events[4].fired) {
        state.setInactive();
        state.clearDisplay();
        state.displayState();
        setLaser(false);
        config.print();
      }

      if(state.route==2 && state.subroute==1) {
        allValues();
      }

      if(state.route==1 && state.subroute) {
        settings();
      }

      state.activeEntered = false;
    }
  }

  events[0].absorb();
  events[1].absorb();
  events[2].absorb();
  events[3].absorb();
  events[4].absorb();
  events[5].absorb();

}



unsigned long lastSensorOnTime = 0;
unsigned long l_lst = 0;
void allValues() {

  if(state.activeEntered) {
    lcd.setCursor(0, 1); lcd.print("    "); lcd.setCursor(0, 1); lcd.print("V: ");    
    lcd.setCursor(0, 2); lcd.print("         "); lcd.setCursor(0, 2); lcd.print("dt: ");
    lcd.setCursor(0, 3); lcd.print("    "); lcd.setCursor(0, 3); lcd.print("iV: ");
    setLaser(true);
  }

  if(t > l_lst+300) {
    lcd.setCursor(3, 1); lcd.print("    "); lcd.setCursor(3, 1); lcd.print(analogRead(LASER_SENS_S));
    l_lst = t;
  }

  if(events[1].fired) {
    lcd.setCursor(4, 2); lcd.print("         "); lcd.setCursor(4, 2); lcd.print(events[1].payloadLong-lastSensorOnTime);
    lcd.setCursor(4, 3); lcd.print("    "); lcd.setCursor(4, 3); lcd.print(events[1].payloadInt);
    lastSensorOnTime = events[1].payloadLong;
  }

}

//MODE, LAPS_N, SENSOR_IGNORE_TIME, MUTE, SAVE_RESULTS
void settings() {
  if(state.activeEntered) {
    lcd.setCursor(0, 1); lcd.print("    "); lcd.setCursor(0, 1); 
    if(state.subroute==1) {lcd.print("RACE MODE: "); lcd.print(config.MODE);}
    if(state.subroute==2) {lcd.print("LAPS NUMBER: "); lcd.print(config.LAPS_N);}
    if(state.subroute==3) {lcd.print("SENSOR IGNORE TIMEOUT: "); lcd.print(config.SENSOR_IGNORE_TIME);}
    if(state.subroute==4) {lcd.print("SAVE RESULTS TO EEPROM: "); lcd.print(config.SAVE_RESULTS);}
    if(state.subroute==5) {lcd.print("MUTE SOUNDS: "); lcd.print(config.MUTE);}
  }
  if(events[2].fired) {
    if(state.subroute==1) {
      if(config.MODE<=1) return; config.MODE--;
      lcd.setCursor(11, 1); lcd.print("    "); lcd.setCursor(11, 1); lcd.print(config.MODE);
    }
    if(state.subroute==2) {
      if(config.LAPS_N<1) return; config.LAPS_N--;
      lcd.setCursor(13, 1); lcd.print("    "); lcd.setCursor(13, 1); lcd.print(config.LAPS_N);
    }
    if(state.subroute==3) {
      if(config.SENSOR_IGNORE_TIME<1) return; config.SENSOR_IGNORE_TIME--;
      lcd.setCursor(23, 1); lcd.print("    "); lcd.setCursor(23, 1); lcd.print(config.SENSOR_IGNORE_TIME);
    }
    if(state.subroute==4) {
      if(!config.SAVE_RESULTS) return; config.SAVE_RESULTS=false;
      lcd.setCursor(24, 1); lcd.print("    "); lcd.setCursor(24, 1); lcd.print(config.SAVE_RESULTS);
    }
    if(state.subroute==5) {
      if(!config.MUTE) return; config.MUTE=false;
      lcd.setCursor(13, 1); lcd.print("    "); lcd.setCursor(13, 1); lcd.print(config.MUTE);
    }
  }
  if(events[3].fired) {
    if(state.subroute==1) {
      if(config.MODE>=2) return; config.MODE++;
      lcd.setCursor(11, 1); lcd.print("    "); lcd.setCursor(11, 1); lcd.print(config.MODE);
    }
    if(state.subroute==2) {
      if(config.LAPS_N>999) return; config.LAPS_N++;
      lcd.setCursor(13, 1); lcd.print("    "); lcd.setCursor(13, 1); lcd.print(config.LAPS_N);
    }
    if(state.subroute==3) {
      if(config.SENSOR_IGNORE_TIME>999) return; config.SENSOR_IGNORE_TIME++;
      lcd.setCursor(23, 1); lcd.print("    "); lcd.setCursor(23, 1); lcd.print(config.SENSOR_IGNORE_TIME);
    }
    if(state.subroute==4) {
      if(config.SAVE_RESULTS) return; config.SAVE_RESULTS=true;
      lcd.setCursor(24, 1); lcd.print("    "); lcd.setCursor(24, 1); lcd.print(config.SAVE_RESULTS);
    }
    if(state.subroute==5) {
      if(config.MUTE) return; config.MUTE=true;
      lcd.setCursor(13, 1); lcd.print("    "); lcd.setCursor(13, 1); lcd.print(config.MUTE);
    }
  }
  if(events[4].fired) {
    if(state.subroute==1) {config.setMODE();}
    if(state.subroute==2) {config.setLAPS_N();}
    if(state.subroute==3) {config.setSENSOR_IGNORE_TIME();}
    if(state.subroute==4) {config.setSAVE_RESULTS();}
    if(state.subroute==5) {config.setMUTE();}
    config.read();
    config.print();
  }
}

void setLaser(boolean value) {
  laserState = value;
  digitalWrite(LASER_S, value);
}






