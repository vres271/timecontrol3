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
#define LASER_SENS_S A7

unsigned long lastLaserOnTime, lastDispRefreshTime, startImpTime, endImpTime, t;
boolean laserState = false;
unsigned int laserSensLevel, laserSens= 0;

//byte eventCode = 0;

struct State{
  unsigned int route = 0;
  unsigned int subroute = 0;
  boolean active = false;
  String routeTitle = "";
  String subRouteTitle = "";
};
State state;

void setup() {


  Serial.begin(9600); 

  lcd.init();
  lcd.backlight();

  state = menu(state,  state);
  displayState();

  enc1.setType(TYPE1);

  pinMode(LASER_PWR, OUTPUT);
  pinMode(LASER_GND, OUTPUT);
  pinMode(LASER_S, OUTPUT);

  pinMode(LASER_SENS_GND, OUTPUT);
  pinMode(LASER_SENS_S, INPUT);

  digitalWrite(LASER_PWR, HIGH);
  digitalWrite(LASER_GND, LOW);
  digitalWrite(LASER_S, laserState);

  digitalWrite(LASER_SENS_GND, LOW);

}

void loop() {

  enc1.tick();

  eventListener(eventEmmiter());

}

byte eventEmmiter() {
  if (enc1.isLeft()) return 10;
  if (enc1.isRight()) return 11;
  if (enc1.isClick()) return 12;
  return 0;
}

boolean eventListener(byte eventCode) {

  if(state.route==1) {
    if(state.subroute==3) {
      if(!state.active) {
        if(eventCode==12) {
          digitalWrite(LASER_S, HIGH);
          lcd.setCursor(0, 2);
          lcd.print("                           ");
          lcd.setCursor(0, 2);
          lcd.print("LASER ON");
          state.active=!state.active;
          eventCode=0;
        }
      } else {
        if(eventCode==12) {
          digitalWrite(LASER_S, LOW);
          lcd.setCursor(0, 2);
          lcd.print("                           ");
          lcd.setCursor(0, 2);
          lcd.print("LASER OFF");
          state.active=!state.active;
          return true;
        }
        if(millis() - lastDispRefreshTime > 1000) {
          lcd.setCursor(12, 2);
          lcd.print("                           ");
          lcd.setCursor(12, 2);
          lcd.print(analogRead(LASER_SENS_S));
          lastDispRefreshTime = millis();
        }

        return true;
      }
    }
  }

  if(eventCode) {
    if(eventCode==10) {menuPrev(); displayState(); return true;}
    if(eventCode==11) {menuNext(); displayState(); return true;}
    if(eventCode==12) {menuEnter(); displayState(); return true;}    
  }
  return true;
}


struct State menu(struct State newState, struct State oldState) {
  if(newState.route==0) {
    newState.routeTitle = "Home";
    if(newState.subroute==0) {newState.subRouteTitle = ""; return newState;}
    if(newState.subroute==1) {newState.subRouteTitle = "back"; return newState;}
    if(newState.subroute==2) {newState.subRouteTitle = "About"; return newState;}
    if(newState.subroute==3) {newState.subRouteTitle = "Help"; return newState;}
    if(newState.subroute==4) {newState.subRouteTitle = "Credit"; return newState;}
  }
  if(newState.route==1) {
    newState.routeTitle = "Set";
    if(newState.subroute==0) {newState.subRouteTitle = ""; return newState;}
    if(newState.subroute==1) {newState.subRouteTitle = "back"; return newState;}
    if(newState.subroute==2) {newState.subRouteTitle = "Sens level"; return newState;}
    if(newState.subroute==3) {newState.subRouteTitle = "Sens test"; return newState;}
  }
  if(newState.route==2) {
    newState.routeTitle = "Race";
    if(newState.subroute==0) {newState.subRouteTitle = ""; return newState;}
    if(newState.subroute==1) {newState.subRouteTitle = "back"; return newState;}
    if(newState.subroute==2) {newState.subRouteTitle = "Racer sel"; return newState;}
  }
  if(newState.route==3) {
    newState.routeTitle = "Res";
    if(newState.subroute==0) {newState.subRouteTitle = ""; return newState;}
    if(newState.subroute==1) {newState.subRouteTitle = "back"; return newState;}
    if(newState.subroute==2) {newState.subRouteTitle = "All res"; return newState;}
    if(newState.subroute==3) {newState.subRouteTitle = "Reset results"; return newState;}
  }
  return oldState;
}

void displayState() {

  lcd.setCursor(0, 0);
  lcd.print("                           ");
  lcd.setCursor(0, 0);
  lcd.print(state.route);
  lcd.print(state.routeTitle);
  if(state.subroute) {
    if(state.subroute==1) {
      lcd.print("<-");
      //state.subRouteTitle = "back"; 
    } else {
      lcd.print("->");
    }
    lcd.print(state.subroute);
    lcd.print(state.subRouteTitle);
  }
}

void menuNext() {
  State newState = state;
  if(newState.subroute) {
    newState.subroute++;
  } else {
    newState.route++;
  }
  state = menu(newState,  state);
}

void menuPrev() {
  State newState = state;
  if(newState.subroute) {
    if(newState.subroute>1) {
      newState.subroute--;
    }
  } else {
    newState.route--;
  }
  state = menu(newState,  state);
}

void menuEnter() {
  State newState = state;
  if(newState.subroute==1) {
    newState.subroute = 0;
  } else if(newState.subroute==0) {
    newState.subroute = 2;
  } else {
    newState.active = !state.active;
  }
  state = menu(newState,  state);
}





  // if (enc1.isTurn()) {     // если был совершён поворот (индикатор поворота в любую сторону)
  //   // ваш код
  // }
  
  // if (enc1.isRight()) {lcd.clear(); lcd.print("Right");  }        // если был поворот
  // if (enc1.isLeft()) {lcd.clear(); lcd.print("Left"); }  
  // if (enc1.isRightH()) {lcd.clear(); lcd.print("Right holded"); } // если было удержание + поворот
  // if (enc1.isLeftH()) {lcd.clear(); lcd.print("Left holded"); }
  
  // //if (enc1.isPress()) {lcd.print("Press");  }        // нажатие на кнопку (+ дебаунс)
  // //if (enc1.isRelease()) {lcd.print("Release");  }    // то же самое, что isClick
  
  // if (enc1.isClick()) {lcd.clear(); lcd.print("Click");  }        // одиночный клик
  // if (enc1.isSingle()) {lcd.clear(); lcd.print("Single");  }      // одиночный клик (с таймаутом для двойного)
  // if (enc1.isDouble()) {lcd.clear(); lcd.print("Double");  }      // двойной клик
  
  
  // if (enc1.isHolded()) {lcd.clear(); lcd.print("Holded");  }      // если была удержана и энк не поворачивался


  // t = millis();

  // if(t - lastLaserOnTime > 1000) {
  //   digitalWrite(LASER_S, LOW);
  //   startImpTime = millis();
  //   delay(3);
  //   digitalWrite(LASER_S, HIGH);
  //   endImpTime = millis();
  //   lastLaserOnTime = t;
  // }


  // laserSens = analogRead(LASER_SENS_S);
  // if(laserSens<laserSensLevel) {
  //   lcd.setCursor(0, 3);
  //   lcd.print("                     ");
  //   lcd.setCursor(0, 3);
  //   lcd.print(t);
  //   lcd.print(" ");
  //   lcd.print(endImpTime-startImpTime);
  //   lcd.print("ms ");
  //   lcd.print(laserSens);
  // }
  // for(int i=0; i<5; i++){
  //     digitalWrite(LASER_S, HIGH);
  //     delay(50);
  //     laserSensLevel += analogRead(LASER_SENS_S);
  //     digitalWrite(LASER_S, LOW);
  //     delay(50);
  //     laserSensLevel += analogRead(LASER_SENS_S);
  //     lcd.setCursor(0, 1);
  //     lcd.print(i);
  // }
  // laserSensLevel = laserSensLevel/10;
  // lcd.setCursor(0, 2);
  // lcd.print(laserSensLevel);

  // digitalWrite(LASER_S, HIGH);
