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

// ВT
#define BT_RX 22
#define BT_TX 23
#define BT_PWR 24
#define BT_GND 25
#include <SoftwareSerial.h>
SoftwareSerial BTSerial(BT_RX, BT_TX);



#define LASER_PWR 7
#define LASER_GND 6
#define LASER_S 5

#define LASER_SENS_GND 9
#define LASER_SENS_PWR 8
#define LASER_SENS_S A7
#define LASER_SENS_SD 18

// Beeper
#define BEEP_S 17
#define BEEP_GND 16

#define RESULTS_EEPROM_SHIFT 100

unsigned long t;
boolean laserState = false;

const char *menu[][10]  = {
  {"Race","Get ready","Results","Clear"},
  {"Set","Mode","Laps N","Ignore time","Save results","Sounds"},
  {"Test","All values","Time","Blink"},
};

class State{
  public:
    byte route = 1;
    byte subroute = 3;
    boolean active = false;
    boolean activeEntered = false;
    boolean blockedActive = false;
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
    boolean setActive() {
      active = true;
      activeEntered = true;
      return true;
    }
    boolean setInactive() {
      if(blockedActive) return false;
      active = false;
      activeEntered = false;
      return true;
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
    unsigned int SENSOR_IGNORE_TIME = 3; byte SENSOR_IGNORE_TIME_addr = 5; // 5  2
    boolean MUTE = false; byte MUTE_addr = 7; // 7  1
    boolean SAVE_RESULTS = false; byte SAVE_RESULTS_addr = 8; // 7  1
    Config() {
      EEPROM.get(0, FS_KEY);
      if(FS_KEY!=12346) { // First start
        Serial.println();
        Serial.println("First start, writing defaults...");
        EEPROM.put(FS_KEY_addr, 12346);
        EEPROM.put(MODE_addr, MODE);
        EEPROM.put(LAPS_N_addr, LAPS_N);
        EEPROM.put(SENSOR_IGNORE_TIME_addr, SENSOR_IGNORE_TIME);
        EEPROM.put(MUTE_addr, MUTE);
        EEPROM.put(SAVE_RESULTS_addr, SAVE_RESULTS);
        EEPROM.put(RESULTS_EEPROM_SHIFT, RESULTS_EEPROM_SHIFT+2);
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

struct resultRow{
  unsigned int r;
  unsigned long t;
};
class Results {
  public:
    unsigned int last_addr = 0;
    Results() {
      EEPROM.get(RESULTS_EEPROM_SHIFT, last_addr);
      //Serial.print("last_addr: ");
      //Serial.println(last_addr);
    }
    struct resultRow read(unsigned int i) {
      resultRow row;
      //Serial.print("Read form address: "); Serial.println(RESULTS_EEPROM_SHIFT + 2 + i*sizeof(resultRow));
      EEPROM.get(RESULTS_EEPROM_SHIFT + 2 + i*sizeof(resultRow), row);
      return row;
    }
    int unsigned write(int unsigned racer, long unsigned time) {
      resultRow row;
      int unsigned last_addr;
      row.r = racer;
      row.t = time;
      EEPROM.get(RESULTS_EEPROM_SHIFT, last_addr);
      EEPROM.put(RESULTS_EEPROM_SHIFT + 2 + last_addr, row);
      last_addr += sizeof(resultRow);
      EEPROM.put(RESULTS_EEPROM_SHIFT, last_addr);
      //Serial.print("Write to address: "); Serial.println(RESULTS_EEPROM_SHIFT + 2 + last_addr);
      Serial.println("Result saved");
      return last_addr;
    }
    void printAll(int r=0) {
      unsigned int last_addr = EEPROM.get(RESULTS_EEPROM_SHIFT, last_addr);
      if(last_addr==0) return;
      resultRow stored_row; 
      for(int i=0; i*sizeof(resultRow)<=(last_addr-sizeof(resultRow)); i++) {
          stored_row = read(i);
          if(r==0 || stored_row.r==r) {
            Serial.print(stored_row.r);Serial.print(" ");Serial.print(millisToTime(stored_row.t));Serial.print("\n");
          }
        if(i>99) return;
      }
    }
    void clearAll() {
      Serial.println("Clear all");
      last_addr = 0;
      EEPROM.put(RESULTS_EEPROM_SHIFT, last_addr);
      //Serial.println("last_addr: ");  Serial.println(last_addr);
    }

};

State state(0,2,false);

Event event0; 
Event event1; // Sensor
Event event2; // Encoder left
Event event3; // Encoder right
Event event4; // Encoder click
Event event5; // BT command recieved
Event events[] = {event0,event1,event2,event3,event4,event5};

Config config;
Results results;

void setup() {

  Serial.begin(9600); 
  BTSerial.begin(9600);  //BTSerial.setTimeout(100);
  if(BTSerial.available()) {
    BTSerial.println('BTSerial is available');
  }

  lcd.init();
  lcd.backlight();
  State state(0,2,false);

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

  digitalWrite(LASER_SENS_GND, LOW);
  digitalWrite(LASER_SENS_PWR, HIGH);

  pinMode(BEEP_S, OUTPUT);
  pinMode(BEEP_GND, OUTPUT);
  digitalWrite(BEEP_GND, LOW);

  pinMode(BT_PWR, OUTPUT);
  pinMode(BT_GND, OUTPUT);
  digitalWrite(BT_PWR, HIGH);
  digitalWrite(BT_GND, LOW);

  attachInterrupt(5, onSensor, RISING); 

  state.displayState();

  Serial.print("\n\n");

  config.read();
  config.print();

  beep( 3500,100);
  delay(150);
  beep( 3000,100);
  delay(150);
  beep( 2500,100);
  delay(150);


}

void loop() {
  t = millis();

  eventEmmiter();
  handler();
  enc1.tick();

  //BTSerial.println("test");
  //delay(200);
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

  // if(events[1].fired) beep(800,20);
  // if(events[2].fired  || events[3].fired) beep(140,5);
  // if(events[4].fired) beep(400,20);

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
        if(state.setInactive()) {
          state.clearDisplay();
          state.displayState();
          setLaser(false);
        }
      }

      if(state.route==0 && state.subroute==1) {
        race();
      }

      if(state.route==0 && state.subroute==2) {
        resultsPage();
      }

      if(state.route==0 && state.subroute==3) {
      }

      if(state.route==1 && state.subroute) {
        settings();
      }

      if(state.route==2 && state.subroute==1) {
        allValues();
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
String sN[6] = {
  ""
  ,"RACE MODE"
  ,"LAPS NUMBER"
  ,"SENS IGNORE TMT"
  ,"SAVE TO EEPROM"
  ,"MUTE SOUNDS"  
};
void clearForValue(unsigned int valueLength) {
  lcd.setCursor(sN[state.subroute].length()+2, 1); 
  String spacer = "";
  for(int i=0; i<valueLength; i++){
    spacer = spacer + " ";
  }
  lcd.print(spacer); 
  lcd.setCursor(sN[state.subroute].length()+2, 1);
}
void settings() {
  if(state.activeEntered) {
    lcd.setCursor(0, 1); lcd.print("    "); lcd.setCursor(0, 1); 
    if(state.subroute==1) {lcd.print(sN[state.subroute]+": "); lcd.print(config.MODE);}
    if(state.subroute==2) {lcd.print(sN[state.subroute]+": "); lcd.print(config.LAPS_N);}
    if(state.subroute==3) {lcd.print(sN[state.subroute]+": "); lcd.print(config.SENSOR_IGNORE_TIME);}
    if(state.subroute==4) {lcd.print(sN[state.subroute]+": "); lcd.print(config.SAVE_RESULTS);}
    if(state.subroute==5) {lcd.print(sN[state.subroute]+": "); lcd.print(config.MUTE);}
  }
  if(state.subroute==1) {
    if(events[2].fired) { // enc left
      if(config.MODE<=1) return; config.MODE--;
      clearForValue(5); lcd.print(config.MODE);
    }
    if(events[3].fired) { // enc right
      if(config.MODE>=2) return; config.MODE++;
      clearForValue(5); lcd.print(config.MODE);
    }
  }
  if(state.subroute==2) {
    if(events[2].fired) { // enc left
      if(config.LAPS_N<1) return; config.LAPS_N--;
      clearForValue(5); lcd.print(config.LAPS_N);
    }
    if(events[3].fired) { // enc right
      if(config.LAPS_N>999) return; config.LAPS_N++;
      clearForValue(5); lcd.print(config.LAPS_N);
    }
  }
  if(state.subroute==3) {
    if(events[2].fired) { // enc left
      if(config.SENSOR_IGNORE_TIME<1) return; config.SENSOR_IGNORE_TIME--;
      clearForValue(6); lcd.print(config.SENSOR_IGNORE_TIME);
    }
    if(events[3].fired) { // enc right
      if(config.SENSOR_IGNORE_TIME>300) return; config.SENSOR_IGNORE_TIME++;
      clearForValue(6); lcd.print(config.SENSOR_IGNORE_TIME);
    }
  }
  if(state.subroute==4) {
    if(events[2].fired) { // enc left
      if(!config.SAVE_RESULTS) return; config.SAVE_RESULTS=false;
      clearForValue(3); lcd.print(config.SAVE_RESULTS);
    }
    if(events[3].fired) { // enc right
      if(config.SAVE_RESULTS) return; config.SAVE_RESULTS=true;
      clearForValue(3); lcd.print(config.SAVE_RESULTS);
    }
  }
  if(state.subroute==5) {
    if(events[2].fired) { // enc left
      if(!config.MUTE) return; config.MUTE=false;
      clearForValue(3); lcd.print(config.MUTE);
    }
    if(events[3].fired) { // enc right
      if(config.MUTE) return; config.MUTE=true;
      clearForValue(3); lcd.print(config.MUTE);
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


unsigned int racer = 0;
unsigned long start_t = 0;
unsigned long finish_t = 0;
unsigned long lap_t = 0;
unsigned long lap_duration = 0;
unsigned long min_lap_duration = 0;
unsigned int laps_counter = 0;
unsigned long last_timer_update = 0;
unsigned long last_timer_update_millis = 0;
byte race_state = 0; // wait, started, finished, ignore

void resetRace() {
    start_t = 0;
    finish_t = 0;
    lap_t = 0;
    lap_duration = 0;
    min_lap_duration = 0;
    laps_counter = 0;
    race_state = 0;    
}

void race() {
  if(state.activeEntered) {
    resetRace();
    setLaser(true);
    Serial.print("\n");
    Serial.print("Laps: ");
    Serial.print(config.LAPS_N);
    Serial.println(" : Ready...");
    lcd.setCursor(0, 1); lcd.print("                    "); lcd.setCursor(0, 1); lcd.print("R:"); lcd.print(racer); lcd.setCursor(6, 1); lcd.print(" L:"); lcd.setCursor(9, 1); lcd.print(config.LAPS_N); lcd.setCursor(13, 1); lcd.print(" Ready");
    lcd.setCursor(0, 2); lcd.print("SAVE TO EEPROM: "); lcd.print(config.SAVE_RESULTS?"YES":"NO"); 
  }

  if(race_state==0) { // wait
    if(events[2].fired) { // left
      if(racer>0) racer--;
      lcd.setCursor(2, 1); lcd.print("     "); lcd.setCursor(2, 1); lcd.print(racer);
    }
    if(events[3].fired) { // right
      if(racer<9999) racer++;
      lcd.setCursor(2, 1); lcd.print("     "); lcd.setCursor(2, 1); lcd.print(racer);
    }
    if(events[1].fired) { // on sensor
      start_t = events[1].payloadLong;
      lap_t = events[1].payloadLong;
      race_state=1;
      beep(800,200);
      Serial.println("Started");
      //lcd.setCursor(0, 1); lcd.print("                    "); 
      lcd.setCursor(13, 1); lcd.print("Started");
      lcd.setCursor(0, 2); lcd.print("                    "); 
      lcd.setCursor(0, 3); lcd.print("Timer:              "); 
    }
  } else if(race_state==1) { // race started
    if(t > last_timer_update+1000) {
      lcd.setCursor(7, 3); lcd.print(millisToTime(t - start_t));
      last_timer_update = t;
    }
    if(t > last_timer_update_millis+200) {
      lcd.setCursor(13, 3); lcd.print(millisToMillis(t - start_t));
      last_timer_update_millis = t;
    }
    if(events[1].fired) { // on sensor
      lap_duration = events[1].payloadLong - lap_t;
      if(min_lap_duration==0||lap_duration<min_lap_duration) {
        min_lap_duration = lap_duration;
      }
      lap_t = events[1].payloadLong;
      laps_counter=laps_counter+1;
      beep(600,50);
      Serial.print("Lap "); Serial.print(laps_counter); Serial.print(" : "); Serial.print(lap_t); Serial.print("\n");
      BTSerial.print("Lap "); BTSerial.print(laps_counter); BTSerial.print(" : "); BTSerial.print(lap_t); BTSerial.print("\n");
      lcd.setCursor(0, 2); lcd.print("Lap                 "); lcd.setCursor(4, 2); lcd.print(laps_counter);lcd.print(": "); lcd.print(millisToTime(lap_duration));
      if(laps_counter>=config.LAPS_N) {
        race_state=2;
        finish_t = events[1].payloadLong;
      }
    }
  } else if(race_state==2) { // race finished
    beep(100,200);
    Serial.print("Finished ");
    Serial.print(" : ");
    Serial.print(finish_t - start_t);
    Serial.print("\n");
    if(config.SAVE_RESULTS) {
      results.write(racer,finish_t - start_t);
    }
    //lcd.setCursor(0, 1); lcd.print("                    "); 
    lcd.setCursor(13, 1); lcd.print("Ready  ");
    lcd.setCursor(0, 2); lcd.print("                    "); lcd.setCursor(0, 2); lcd.print("Best:  "); lcd.print(millisToTime(min_lap_duration));
    lcd.setCursor(0, 3); lcd.print("                    "); lcd.setCursor(0, 3); lcd.print("All:   "); lcd.print(millisToTime(finish_t - start_t));
    //setLaser(false);
    resetRace();
  }
}

byte results_action = 0;
boolean results_clear_confirm = false;
void resultsPage() {

  if(state.activeEntered) {
    results_action = 0;
    results_clear_confirm = false;
    lcd.setCursor(0, 1); lcd.print("Show "); lcd.print(" <");
    lcd.setCursor(0, 2); lcd.print("Print"); 
    lcd.setCursor(0, 3); lcd.print("Clear");
    state.blockedActive = true;
  }
  if(events[2].fired) { // left
    if(results_action>0) {
      results_clear_confirm = false;
      lcd.setCursor(6, results_action+1); lcd.print("          ");
      results_action--;
      lcd.setCursor(6, results_action+1); lcd.print("<");
    } else { // exit to submenu
      state.blockedActive = false;
      state.setInactive();
      state.clearDisplay();
      state.displayState();
    }
  }
  if(events[3].fired) { // right
    if(results_action<2) {
      results_clear_confirm = false;
      lcd.setCursor(6, results_action+1); lcd.print("          ");
      results_action++;
      lcd.setCursor(6, results_action+1); lcd.print("<");
    }
  }
  if(events[4].fired) {
    if(results_action==0) {
      lcd.setCursor(6, results_action+1); lcd.print(">"); lcd.print(" OK ");
    } else if (results_action==1) {
      results.printAll();
      lcd.setCursor(6, results_action+1); lcd.print(">"); lcd.print(" OK : "); lcd.print(results.last_addr/sizeof(resultRow));
    } else if (results_action==2) {
      lcd.setCursor(6, results_action+1); lcd.print(">"); 
      if(!results_clear_confirm) {
        lcd.setCursor(8, results_action+1); lcd.print(" Sure?  ");
        results_clear_confirm = true;
      } else {
        results.clearAll();
        lcd.setCursor(8, results_action+1); lcd.print(" Cleared");
        results_clear_confirm = false;
      }
    }
  }

}


void setLaser(boolean value) {
  laserState = value;
  digitalWrite(LASER_S, value);
}

void printLabel(String label, byte row = 1) {
  unsigned int length = label.length();
  lcd.setCursor(length, row);
  String spacer = "";
  for(int i=0; i<length; i++){
    spacer = spacer + "";
  }
  lcd.print(spacer); 
  lcd.setCursor(length, row); 
}

String millisToTime(long unsigned time){
  int unsigned h_,m_,s_,m,s,ms = 0;
  s_ = int(time/1000);
  m_ = int(time/60000);
  h_ = int(time/3600000);
  ms = time-s_*1000;
  s = s_-m_*60;
  m = m_-h_*60;
  return (m<10?"0":"")+String(m)+(":")+(s<10?"0":"")+String(s)+(".")+(ms<10?"0":"")+(ms<100?"0":"")+String(ms);
}
String millisToMillis(long unsigned time){
  int unsigned s_,ms = 0;
  s_ = int(time/1000);
  ms = time-s_*1000;
  return String(ms<10?"0":"")+String(ms<100?"0":"")+String(ms);
}
void beep(unsigned int freq, unsigned int duration) {
  if(config.MUTE) return;
  tone(BEEP_S, freq,duration);
}

