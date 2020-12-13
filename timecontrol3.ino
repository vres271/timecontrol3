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

#include <EEPROM.h>

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
  {"Set","Laps n","Mode","Ignore t"},
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
      if(subroute<3) {
        subroute++;
      }
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
State state(2,1,true);

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

Event event0; 
Event event1; // Sensor
Event event2; // Encoder left
Event event3; // Encoder right
Event event4; // Encoder click
Event event5; // BT command recieved
Event events[] = {event0,event1,event2,event3,event4,event5};

unsigned int EEMEM fs_key_addr;
byte EEMEM mode_addr;
unsigned int EEMEM laps_n_addr;
unsigned int EEMEM sensor_ignore_time_addr;
unsigned int EEMEM mute_addr;

void setup() {

  Serial.begin(9600); 

  lcd.init();
  lcd.backlight();

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

  // объявляем переменные, куда будем читать
  unsigned int FS_KEY;
  byte MODE;
  unsigned int LAPS_N;
  unsigned int SENSOR_IGNORE_TIME;
  unsigned int MUTE;

  // читаем точно так же, как писали
  EEPROM.get((int)&fs_key_addr, FS_KEY);
  EEPROM.get((int)&mode_addr, MODE);
  EEPROM.get((int)&laps_n_addr, LAPS_N);
  EEPROM.get((int)&sensor_ignore_time_addr, SENSOR_IGNORE_TIME);
  EEPROM.get((int)&mute_addr, MUTE);

  Serial.println(FS_KEY);
  Serial.println(MODE);
  Serial.println(LAPS_N);
  Serial.println(SENSOR_IGNORE_TIME);
  Serial.println(MUTE);
  Serial.println();

  

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

void setLaser(boolean value) {
  laserState = value;
  digitalWrite(LASER_S, value);
}






















// byte eventEmmiter() {
//   if (enc1.isLeft()) return 10;
//   if (enc1.isRight()) return 11;
//   if (enc1.isClick()) return 12;
//   return 0;
// }


// void enableSensorTest() {
//   digitalWrite(LASER_S, HIGH);
//   lcd.setCursor(0, 1);
//   lcd.print("                                      ");
//   lcd.setCursor(0, 1);
//   lcd.print("Started");
//   lcd.setCursor(10, 1);
//   lcd.print("V0:");
//   lcd.setCursor(10, 2);
//   lcd.print("V1:");
//   lcd.setCursor(10, 3);
//   lcd.print("VA:");
//   lcd.setCursor(0, 3);
//   lcd.print("dV:");
//   lcd.setCursor(0, 2);
//   lcd.print("V:");
// }

// void disableSensorTest() {
//   digitalWrite(LASER_S, LOW);
//   lcd.setCursor(0, 1);
//   lcd.print("                                      ");
//   lcd.setCursor(0, 1);
//   lcd.print("Stopped");
// }

// unsigned long lastLaserSwitchTime, LaserSwitchPeriod=500;
// boolean LaserState=true;
// unsigned int laser0=1, laser1=2, laserA=3;
// void SensorTest() {

//   if(millis() - lastLaserSwitchTime > LaserSwitchPeriod) {
//     if(LaserState) {
//       laser1 = analogRead(LASER_SENS_S);
//       lcd.setCursor(14, 2);
//       lcd.print("     ");
//       lcd.setCursor(14, 2);
//       lcd.print(laser1);
//     } else {
//       laser0 = analogRead(LASER_SENS_S);
//       lcd.setCursor(14, 1);
//       lcd.print("     ");
//       lcd.setCursor(14, 1);
//       lcd.print(laser0);
//     }
//     laserA = (laser0+laser1)/2;
//     lcd.setCursor(14, 3);
//     lcd.print("     ");
//     lcd.setCursor(14, 3);
//     lcd.print(laserA);

//     lcd.setCursor(4, 3);
//     lcd.print("   ");
//     lcd.setCursor(4, 3);
//     lcd.print((laser1-laser0));

//     lcd.setCursor(4, 2);
//     lcd.print("     ");
//     lcd.setCursor(4, 2);
//     lcd.print(analogRead(LASER_SENS_S));
//   }

//   if(millis() - lastLaserSwitchTime > LaserSwitchPeriod) {
//     LaserState=!LaserState;
//     digitalWrite(LASER_S, LaserState);
//     lastLaserSwitchTime = millis();
//   }
// }

// void LaserSwitchPeriodDec() {
//   if(LaserSwitchPeriod<=2) return;
//   LaserSwitchPeriod = LaserSwitchPeriod/2;
//   lcd.setCursor(0, 1);
//   lcd.print("          ");
//   lcd.setCursor(0, 1);
//   lcd.print("T:");
//   lcd.print(LaserSwitchPeriod);
// }

// void LaserSwitchPeriodInc() {
//   if(LaserSwitchPeriod>=10000) return;
//   LaserSwitchPeriod = LaserSwitchPeriod*2;
//   lcd.setCursor(0, 1);
//   lcd.print("          ");
//   lcd.setCursor(0, 1);
//   lcd.print("T:");
//   lcd.print(LaserSwitchPeriod);
// }

// boolean eventListener(byte eventCode) {

//   if(state.route==1) {
//     if(state.subroute==3) {
//       if(state.active) {
//         if(eventCode==12) {
//           disableSensorTest();
//           state.active=!state.active;
//           return true;
//         }
//         if(eventCode==10) {LaserSwitchPeriodDec(); eventCode=0;}
//         if(eventCode==11) {LaserSwitchPeriodInc(); eventCode=0;}
//         SensorTest();
//         return true;
//       } else {
//         if(eventCode==12) {
//           enableSensorTest();
//           state.active=!state.active;
//           eventCode=0;
//         }

//       }
//     }
//   }

//   if(eventCode) {
//     if(eventCode==10) {menuPrev(); displayState(); return true;}
//     if(eventCode==11) {menuNext(); displayState(); return true;}
//     if(eventCode==12) {menuEnter(); displayState(); return true;}    
//   }
//   return true;
// }


// struct State menu(struct State newState, struct State oldState) {
//   if(newState.route==0) {
//     newState.routeTitle = "Home";
//     if(newState.subroute==0) {newState.subRouteTitle = ""; return newState;}
//     if(newState.subroute==1) {newState.subRouteTitle = "back"; return newState;}
//     if(newState.subroute==2) {newState.subRouteTitle = "About"; return newState;}
//     if(newState.subroute==3) {newState.subRouteTitle = "Help"; return newState;}
//     if(newState.subroute==4) {newState.subRouteTitle = "Credit"; return newState;}
//   }
//   if(newState.route==1) {
//     newState.routeTitle = "Set";
//     if(newState.subroute==0) {newState.subRouteTitle = ""; return newState;}
//     if(newState.subroute==1) {newState.subRouteTitle = "back"; return newState;}
//     if(newState.subroute==2) {newState.subRouteTitle = "Sens level"; return newState;}
//     if(newState.subroute==3) {newState.subRouteTitle = "Sens test"; return newState;}
//   }
//   if(newState.route==2) {
//     newState.routeTitle = "Race";
//     if(newState.subroute==0) {newState.subRouteTitle = ""; return newState;}
//     if(newState.subroute==1) {newState.subRouteTitle = "back"; return newState;}
//     if(newState.subroute==2) {newState.subRouteTitle = "Racer sel"; return newState;}
//   }
//   if(newState.route==3) {
//     newState.routeTitle = "Res";
//     if(newState.subroute==0) {newState.subRouteTitle = ""; return newState;}
//     if(newState.subroute==1) {newState.subRouteTitle = "back"; return newState;}
//     if(newState.subroute==2) {newState.subRouteTitle = "All res"; return newState;}
//     if(newState.subroute==3) {newState.subRouteTitle = "Reset results"; return newState;}
//   }
//   return oldState;
// }

// void displayState() {

//   lcd.setCursor(0, 0);
//   lcd.print("                           ");
//   lcd.setCursor(0, 0);
//   lcd.print(state.route);
//   lcd.print(state.routeTitle);
//   if(state.subroute) {
//     if(state.subroute==1) {
//       lcd.print("<-");
//       //state.subRouteTitle = "back"; 
//     } else {
//       lcd.print("->");
//     }
//     lcd.print(state.subroute);
//     lcd.print(state.subRouteTitle);
//   }
// }

// void menuNext() {
//   State newState = state;
//   if(newState.subroute) {
//     newState.subroute++;
//     newState.active = false;
//   } else {
//     newState.route++;
//     newState.active = false;
//   }
//   state = menu(newState,  state);
//   lcd.setCursor(0, 1);
//   lcd.print("                          ");
//   lcd.setCursor(0, 2);
//   lcd.print("                          ");
//   lcd.setCursor(0, 3);
//   lcd.print("                          ");
//   lcd.setCursor(0, 4);
//   lcd.print("                          ");
// }

// void menuPrev() {
//   State newState = state;
//   if(newState.subroute) {
//     if(newState.subroute>1) {
//       newState.subroute--;
//       newState.active = false;
//     }
//   } else {
//     newState.route--;
//     newState.active = false;
//   }
//   state = menu(newState,  state);
//   lcd.setCursor(0, 2);
//   lcd.print("                          ");
//   lcd.setCursor(0, 3);
//   lcd.print("                          ");
//   lcd.setCursor(0, 4);
//   lcd.print("                          ");
// }

// void menuEnter() {
//   State newState = state;
//   if(newState.subroute==1) {
//     newState.subroute = 0;
//   } else if(newState.subroute==0) {
//     newState.subroute = 2;
//   } else {
//     newState.active = !state.active;
//   }
//   state = menu(newState,  state);
// }




