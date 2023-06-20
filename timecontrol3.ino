#include <EEPROM.h>

#include "microLiquidCrystal_I2C.h"
LiquidCrystal_I2C lcd(0x27, 20, 4);
// адрес дисплея 0x3f или 0x27
// ширина дисплея (тут 20 символов)
// высота дисплея (тут 4 строки)

#define CLK 24
#define DT 25
#define SW 26
#include "GyverEncoder.h"
Encoder enc1(CLK, DT, SW);  // для работы c кнопкой

// Radio Module nRF24
// #include <SPI.h>
// #include "nRF24L01.h"
// #include "RF24.h"
// RF24 radio(22,23);// Для Меги
// #define RADIO_PWR_ON 16

byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; //возможные номера труб

byte counter;


#define LASER_PWR 29
#define LASER_GND 28
#define LASER_S 27

#define LASER_SENS_GND 31
#define LASER_SENS_PWR 30
#define LASER_SENS_S A0
#define LASER_SENS_SD 18

// Beeper
#define BEEP_S 17
#define EXTERNAL_AUDIO 19
//#define BEEP_GND 16

// Battery control
#define BATTERY_V A1

#define RESULTS_EEPROM_SHIFT 100

unsigned long t;
boolean laserState = false;


char divider = ' ';
char ending = ';';
const char *headers[]  = {"getresults","getinputs","getconfig","race","save","mode","laps","level","cancel","scal","batr","mute","stimeout","<", "^", ">", "help",};
enum names {GET_RESULTS, GET_INPUTS, GET_CONFIG, RACE, SAVE, _MODE, LAPS, LEVEL, CANCEL, SCAL, BATR, _MUTE, _STIMEOUT, LEFT, CLICK, RIGHT, HELP, };
names thisName;
byte headers_am = sizeof(headers) / 2;
uint32_t prsTimer;
String prsValue = "";
String prsHeader = "";
enum stages {WAIT, HEADER, GOT_HEADER, VALUE, SUCCESS};
stages parseStage = WAIT;
boolean recievedFlag;


const char *menu[][10]  = {
  {"Race","Get ready","Results","Records","Summary"},
  {"Set","Mode","Laps N","Ignore time","Save results","Sounds","External audio"},
  {"Test","All values","Aim","Blink","Radio"},
};


#include "classes/state.ino";
#include "classes/event.ino";
#include "classes/config.ino";
#include "classes/results.ino";
// #include "classes/radio.ino";



State state(0,1,false);

Event event0; 
Event event1; // Sensor
Event event2; // Encoder left
Event event3; // Encoder right
Event event4; // Encoder click
Event event5; // BT command recieved
Event events[] = {event0,event1,event2,event3,event4,event5};

Config config;
Results results;
// Radio radioModule;

void setup() {


  Serial.begin(9600); 
  Serial3.begin(9600);  Serial3.setTimeout(100);

  Serial.println("\napi connect_timecontrol3");

  lcd.init();
  lcd.backlight();
  State state(0,1,false);

  enc1.setType(TYPE2);

  //initRadio();

  // pinMode(RADIO_PWR_ON, OUTPUT);

  pinMode(LASER_PWR, OUTPUT);
  pinMode(LASER_GND, OUTPUT);
  pinMode(LASER_S, OUTPUT);

  pinMode(LASER_SENS_GND, OUTPUT);
  pinMode(LASER_SENS_PWR, OUTPUT);
  pinMode(LASER_SENS_S, INPUT);
  pinMode(LASER_SENS_SD, INPUT);

  // digitalWrite(RADIO_PWR_ON, LOW);

  digitalWrite(LASER_PWR, HIGH);
  digitalWrite(LASER_GND, LOW);
  digitalWrite(LASER_S, laserState);

  digitalWrite(LASER_SENS_GND, LOW);
  digitalWrite(LASER_SENS_PWR, HIGH);

  pinMode(BEEP_S, OUTPUT);
  //pinMode(BEEP_GND, OUTPUT);
  //digitalWrite(BEEP_GND, LOW);

  //pinMode(BT_PWR, OUTPUT);
  //pinMode(BT_GND, OUTPUT);
  //digitalWrite(BT_PWR, HIGH);
  //digitalWrite(BT_GND, LOW);

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

  parsingSeparate();
  SerialRouter();


  eventEmmiter();
  handler();
  enc1.tick();

  //scanRadio();
  //sendRadio();

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
        recordsPage();
      }

      if(state.route==0 && state.subroute==4) {
        summaryPage();
      }

      if(state.route==1 && state.subroute) {
        settings();
      }

      if(state.route==2 && state.subroute==1) {
        allValues();
      }
      if(state.route==2 && state.subroute==2) {
        aim();
      }
      if(state.route==2 && state.subroute==4) {
        // testRadio();
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
    lcd.setCursor(0, 2); lcd.print("        "); lcd.setCursor(0, 2); lcd.print("dt: ");
    lcd.setCursor(0, 3); lcd.print("    "); lcd.setCursor(0, 3); lcd.print("iV: ");
    lcd.setCursor(12, 2); lcd.print("    "); lcd.setCursor(12, 2); lcd.print("bL: ");
    lcd.setCursor(12, 3); lcd.print("    "); lcd.setCursor(12, 3); lcd.print("bV: ");
    setLaser(true);
    //initRadioForScan();
  }

  if(t > l_lst+300) {
    lcd.setCursor(3, 1); lcd.print("    "); lcd.setCursor(3, 1); lcd.print(analogRead(LASER_SENS_S));
    lcd.setCursor(16, 2); lcd.print("    "); lcd.setCursor(16, 2); lcd.print(map(map(analogRead(BATTERY_V),0,1023,0,5300),2600,4200,0,100)); lcd.print("%");
    lcd.setCursor(16, 3); lcd.print("    "); lcd.setCursor(16, 3); lcd.print(map(analogRead(BATTERY_V),0,1023,0,5300));
    l_lst = t;
  }

  if(events[1].fired) {
    lcd.setCursor(4, 2); lcd.print("        "); lcd.setCursor(4, 2); lcd.print(events[1].payloadLong-lastSensorOnTime);
    lcd.setCursor(4, 3); lcd.print("    "); lcd.setCursor(4, 3); lcd.print(events[1].payloadInt);
    lastSensorOnTime = events[1].payloadLong;
  }

}

unsigned int aimPer = 0;
unsigned long l_lst_v = 0;
void aim() {

  if(state.activeEntered) {
    setLaser(true);
  }

  if(t > l_lst_v+300) {
    lcd.setCursor(0, 1); lcd.print("     "); 
    lcd.setCursor(0, 1); lcd.print(analogRead(LASER_SENS_S));
    l_lst_v = t;
  }

  if(t > l_lst+100) {
    aimPer = map(analogRead(LASER_SENS_S), 0, 1023, 20, 0);
    lcd.setCursor(0, 2); //lcd.print("                      "); 
    for(int i=0; i<20; i++){
        if(i < aimPer) {
          lcd.print("#");
        } else {
          lcd.print(" ");
        }
    }
    lcd.setCursor(0, 3); //lcd.print("                      "); 
    for(int i=0; i<20; i++){
        if(i < aimPer) {
          lcd.print("#");
        } else {
          lcd.print(" ");
        }
    }
    l_lst = t;
  }

  if(events[4].fired) {
    
  }


}
// void testRadio() {

//   if(state.activeEntered) {
//     radioModule.init();
//   }

//   if(events[4].fired) {
//     radioModule.close();
//   }


// }


//MODE, LAPS_N, SENSOR_IGNORE_TIME, MUTE, SAVE_RESULTS, EXTERNAL_AUDIO_ON
String sN[7] = {
  ""
  ,"RACE MODE"
  ,"LAPS NUMBER"
  ,"SENS IGNORE TMT"
  ,"SAVE TO EEPROM"
  ,"MUTE SOUNDS"  
  ,"EXTERNAL AUDIO ON"  
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
    if(state.subroute==6) {lcd.print(sN[state.subroute]+": "); lcd.print(config.EXTERNAL_AUDIO_ON);}
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
  if(state.subroute==6) {
    if(events[2].fired) { // enc left
      if(!config.EXTERNAL_AUDIO_ON) return; config.EXTERNAL_AUDIO_ON=false;
      clearForValue(3); lcd.print(config.EXTERNAL_AUDIO_ON);
    }
    if(events[3].fired) { // enc right
      if(config.EXTERNAL_AUDIO_ON) return; config.EXTERNAL_AUDIO_ON=true;
      clearForValue(3); lcd.print(config.EXTERNAL_AUDIO_ON);
    }
  }
  if(events[4].fired) {
    if(state.subroute==1) {config.setMODE();}
    if(state.subroute==2) {config.setLAPS_N();}
    if(state.subroute==3) {config.setSENSOR_IGNORE_TIME();}
    if(state.subroute==4) {config.setSAVE_RESULTS();}
    if(state.subroute==5) {config.setMUTE();}
    if(state.subroute==6) {config.setEXTERNAL_AUDIO_ON();}
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
unsigned long result_time = 0;
unsigned long record_lap = 0;
unsigned long record_lap_racer = 0;
unsigned long record_all = 0;
unsigned long record_all_racer = 0;
unsigned long record_lap_bck = 0;
unsigned long record_lap_racer_bck = 0;
unsigned long record_all_bck = 0;
unsigned long record_all_racer_bck = 0;
byte race_state = 0; // wait, started, finished, ignore

void resetRace() {
    start_t = 0;
    finish_t = 0;
    lap_t = 0;
    lap_duration = 0;
    min_lap_duration = 0;
    laps_counter = 0;
    race_state = 0; 
    result_time = 0;   

    // record_lap_bck = record_lap;
    // record_lap_racer_bck = record_lap_racer;
    // record_all_bck = record_all;    
}

void cancelRace() {
    record_lap = record_lap_bck;
    record_lap_racer = record_lap_racer_bck;
    record_all = record_all_bck;    
    record_all_racer = record_all_racer_bck;    
    log("\nRace canceled\n");
    Serial.println("\napi cancel "+String(racer)+" "+String(t));
}

void clearRecords() {
    record_lap = 0;
    record_lap_racer = 0;
    record_all = 0;    
    record_all_racer = 0;    
    log("\nRecords cleared\n");
}

unsigned long lastFixedSensorTime = 0;
void race() {
  if(state.activeEntered) {
    resetRace();
    setLaser(true);
    log("\nLaps: "+String(config.LAPS_N)+" : Ready...");
    Serial.println("\napi ready "+String(racer)+" "+String(start_t)+" "+String(config.LAPS_N));
    lcd.setCursor(0, 1); lcd.print("                    "); lcd.setCursor(0, 1); lcd.print("R:"); lcd.print(racer); lcd.setCursor(6, 1); lcd.print(" L:"); lcd.setCursor(9, 1); lcd.print(config.LAPS_N); lcd.setCursor(13, 1); lcd.print(" Ready");
    lcd.setCursor(0, 2); lcd.print("SAVE TO EEPROM: "); lcd.print(config.SAVE_RESULTS?"YES":"NO"); 
  }

  if(race_state==0) { // wait
    if(events[2].fired) { // left
      if(racer>0) racer--;
      lcd.setCursor(2, 1); lcd.print("     "); lcd.setCursor(2, 1); lcd.print(racer);
      Serial.println("\napi set_racer "+String(racer)+" "+String(t));
    }
    if(events[3].fired) { // right
      if(racer<9999) racer++;
      lcd.setCursor(2, 1); lcd.print("     "); lcd.setCursor(2, 1); lcd.print(racer);
      Serial.println("\napi set_racer "+String(racer)+" "+String(t));
    }
    if(events[1].fired) { // on sensor
      if(events[1].payloadLong > lastFixedSensorTime + config.SENSOR_IGNORE_TIME*1000) {
        start_t = events[1].payloadLong;
        lastFixedSensorTime = events[1].payloadLong;
        lap_t = events[1].payloadLong;
        race_state=1;
        record_lap_bck = record_lap;
        record_lap_racer_bck = record_lap_racer;
        record_all_bck = record_all;    
        record_all_racer_bck = record_all_racer;    
        beep(800,200);
        log("\nR"+String(racer)+" started\n");
        Serial.println("\napi start "+String(racer)+" "+String(start_t));
        lcd.setCursor(13, 1); lcd.print("Started");
        lcd.setCursor(0, 2); lcd.print("                    "); 
        lcd.setCursor(0, 3); lcd.print("Timer:              "); 
      }
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
      if(events[1].payloadLong > lastFixedSensorTime + config.SENSOR_IGNORE_TIME*1000) {
        lap_duration = events[1].payloadLong - lap_t;
        lastFixedSensorTime = events[1].payloadLong;
        if(min_lap_duration==0||lap_duration<min_lap_duration) {
          min_lap_duration = lap_duration;
        }
        lap_t = events[1].payloadLong;
        laps_counter=laps_counter+1;
        beep(600,50);
        Serial.println("\napi lap "+String(racer)+" "+String(lap_t));
        log("Lap "+String(laps_counter)+" : "+String(millisToTime(lap_duration))); 
        if(record_lap==0) record_lap = lap_duration;
        if(lap_duration<record_lap) {
          record_lap = lap_duration;
          record_lap_racer = racer;
          beep(70,1000);
          log(" New Record lap");
        }
        log("\n");
        lcd.setCursor(0, 2); lcd.print("Lap                 "); lcd.setCursor(4, 2); lcd.print(laps_counter);lcd.print(": "); lcd.print(millisToTime(lap_duration));
        if(laps_counter>=config.LAPS_N) {
          race_state=2;
          finish_t = events[1].payloadLong;
        }
      }
    }
    if(events[4].fired) {
      cancelRace();
    }
  } else if(race_state==2) { // race finished
    beep(300,300);
    result_time = finish_t - start_t;
    log("Finished  : "+String(millisToTime(result_time))+"\n");
    Serial.println("\napi finish "+String(racer)+" "+String(finish_t));
    if(record_all==0) record_all = result_time;
    if(result_time<record_all) {
      record_all = result_time;
      record_all_racer = racer;
      beep(45,1500);
      log("\nNew Record result  : R"+String(racer)+" : "+String(millisToTime(record_all))+"\n");
    }
    if(config.SAVE_RESULTS) {
      results.write(racer,result_time);
    }
    //lcd.setCursor(0, 1); lcd.print("                    "); 
    lcd.setCursor(13, 1); lcd.print("Ready  ");
    lcd.setCursor(0, 2); lcd.print("                    "); lcd.setCursor(0, 2); lcd.print("Best:  "); lcd.print(millisToTime(min_lap_duration));
    lcd.setCursor(0, 3); lcd.print("                    "); lcd.setCursor(0, 3); lcd.print("All:   "); lcd.print(millisToTime(result_time));
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
    lcd.setCursor(0, 1); lcd.print("ClrLst"); lcd.print(" <");
    lcd.setCursor(0, 2); lcd.print("Print "); 
    lcd.setCursor(0, 3); lcd.print("Clear ");
    state.blockedActive = true;
  }
  if(events[2].fired) { // left
    if(results_action>0) {
      results_clear_confirm = false;
      lcd.setCursor(7, results_action+1); lcd.print("          ");
      results_action--;
      lcd.setCursor(7, results_action+1); lcd.print("<");
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
      lcd.setCursor(7, results_action+1); lcd.print("          ");
      results_action++;
      lcd.setCursor(7, results_action+1); lcd.print("<");
    }
  }
  if(events[4].fired) {
    if(results_action==0) {
      //lcd.setCursor(7, results_action+1); lcd.print(">"); lcd.print(" OK ");
      if(!results_clear_confirm) {
        lcd.setCursor(9, results_action+1); lcd.print(" Sure?  ");
        results_clear_confirm = true;
      } else {
        results.clearLast();
        lcd.setCursor(9, results_action+1); lcd.print(" Cleared");
        results_clear_confirm = false;
      }
    } else if (results_action==1) {
      results.printAll();
      lcd.setCursor(7, results_action+1); lcd.print(">"); lcd.print(" OK : "); lcd.print(results.last_addr/sizeof(resultRow));
    } else if (results_action==2) {
      lcd.setCursor(7, results_action+1); lcd.print(">"); 
      if(!results_clear_confirm) {
        lcd.setCursor(9, results_action+1); lcd.print(" Sure?  ");
        results_clear_confirm = true;
      } else {
        results.clearAll();
        lcd.setCursor(9, results_action+1); lcd.print(" Cleared");
        results_clear_confirm = false;
      }
    }
  }

}

byte records_action = 0;
boolean records_clear_confirm = false;

void showRecords() {
    lcd.setCursor(0, 1); lcd.print("Lap:"); lcd.setCursor(6, 1); lcd.print(record_lap_racer); lcd.print(":"); lcd.setCursor(10, 1); lcd.print(millisToTime(record_lap));
    lcd.setCursor(0, 2); lcd.print("Race:"); lcd.setCursor(6, 2); lcd.print(record_all_racer); lcd.print(":"); lcd.setCursor(10, 2); lcd.print(millisToTime(record_all));
    lcd.setCursor(0, 3); lcd.print("Clear ");
}

void recordsPage() {

  if(state.activeEntered) {
    records_action = 0;
    records_clear_confirm = false;
    state.blockedActive = true;
    showRecords();
  }
  if(events[2].fired) { // left
    if(records_action>0) {
      records_clear_confirm = false;
      lcd.setCursor(6, 3); lcd.print("          ");
      records_action=0;
    } else { // exit to submenu
      state.blockedActive = false;
      state.setInactive();
      state.clearDisplay();
      state.displayState();
    }
  }
  if(events[3].fired) { // right
    if(records_action==0) {
      records_clear_confirm = false;
      lcd.setCursor(6, 3); lcd.print("          ");
      records_action=1;
      lcd.setCursor(6, 3); lcd.print("<");
    }
  }
  if(events[4].fired) {
    if(records_action==0) {
      //lcd.setCursor(6, 3); lcd.print(">"); lcd.print(" OK ");
    } else if (records_action==1) {
      //results.printAll();
      if(!records_clear_confirm) {
        lcd.setCursor(8, 3); lcd.print(" Sure?  ");
        records_clear_confirm = true;
      } else {
        clearRecords();
        lcd.setCursor(8, 3); lcd.print(" Cleared");
        records_clear_confirm = false;
        records_action = 0;
        lcd.setCursor(6, 1); lcd.print("              ");
        lcd.setCursor(6, 2); lcd.print("              ");       
        showRecords();
      }
    }
  }

}

void summaryPage() {
  if(state.activeEntered) {
    results.getSummary(true);
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
  if(config.EXTERNAL_AUDIO_ON) {
    tone(EXTERNAL_AUDIO, freq,duration);
  } else {
    tone(BEEP_S, freq,duration);
  }
}

void log(String txt) {
  // Serial.print(txt);
  Serial3.print(txt);
}

void parsingSeparate() {
  if (Serial3.available() > 0) {
    if (parseStage == WAIT) {
      parseStage = HEADER;
      prsHeader = "";
      prsValue = "";
    }
    if (parseStage == GOT_HEADER) parseStage = VALUE;
    char incoming = (char)Serial3.read();
    if (incoming == divider) {
      parseStage = GOT_HEADER;
    } else if (incoming == ending) {
      parseStage = SUCCESS;
    }
    if (parseStage == HEADER) {
      prsHeader += incoming;
    }
    else if (parseStage == VALUE) prsValue += incoming;
    prsTimer = millis();
  }
  if (parseStage == SUCCESS) {
    for (byte i = 0; i < headers_am; i++) { if (prsHeader == headers[i]) { thisName = i; } } recievedFlag = true; parseStage = WAIT; } if ((millis() - prsTimer > 10) && (parseStage != WAIT)) {  // таймаут
    parseStage = WAIT;
  }
  // if (parseStage == HEADER) {
  //  if (millis() - prsTimer > 10 ) {
  //    parseStage == SUCCESS;
  //  }
  // }
  
  if (Serial.available() > 0) {
    if (parseStage == WAIT) {
      parseStage = HEADER;
      prsHeader = "";
      prsValue = "";
    }
    if (parseStage == GOT_HEADER) parseStage = VALUE;
    char incoming = (char)Serial.read();
    if (incoming == divider) {
      parseStage = GOT_HEADER;
    } else if (incoming == ending) {
      parseStage = SUCCESS;
    }
    if (parseStage == HEADER) {
      prsHeader += incoming;
    }
    else if (parseStage == VALUE) prsValue += incoming;
    prsTimer = millis();
  }
  if (parseStage == SUCCESS) {
    for (byte i = 0; i < headers_am; i++) { if (prsHeader == headers[i]) { thisName = i; } } recievedFlag = true; parseStage = WAIT; } if ((millis() - prsTimer > 10) && (parseStage != WAIT)) {  // таймаут
    parseStage = WAIT;
  }
  // if (parseStage == HEADER) {
  //  if (millis() - prsTimer > 10 ) {
  //    parseStage == SUCCESS;
  //  }
  // }
}

void SerialRouter() {
  if (recievedFlag) {
    recievedFlag = false;
    if(thisName == GET_RESULTS) {
      if(prsValue.toInt()==0) {
        results.printAll(); 
      } else {
        log("\nResults for racer "); log(prsValue); log("\n\n");
        //printAllResults(prsValue.toInt()); log("\n");
      }
    } else if (thisName == LEFT) {
      events[2].emit();
    } else if (thisName == RIGHT) {
      events[3].emit();
    }  else if (thisName == CLICK) {
      events[4].emit();
    } 
    thisName = '0'; prsValue=""; parseStage = WAIT;
  }
}





