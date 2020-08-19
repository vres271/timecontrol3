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

void setup() {


  Serial.begin(9600); 

  lcd.init();
  lcd.backlight();

  applyState();

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

  for(int i=0; i<5; i++){
      digitalWrite(LASER_S, HIGH);
      delay(50);
      laserSensLevel += analogRead(LASER_SENS_S);
      digitalWrite(LASER_S, LOW);
      delay(50);
      laserSensLevel += analogRead(LASER_SENS_S);
      lcd.setCursor(0, 1);
      lcd.print(i);
  }
  laserSensLevel = laserSensLevel/10;
  lcd.setCursor(0, 2);
  lcd.print(laserSensLevel);

  digitalWrite(LASER_S, HIGH);
}

void loop() {
  t = millis();

  if(t - lastLaserOnTime > 1000) {
    digitalWrite(LASER_S, LOW);
    startImpTime = millis();
    delay(3);
    digitalWrite(LASER_S, HIGH);
    endImpTime = millis();
    lastLaserOnTime = t;
  }


  laserSens = analogRead(LASER_SENS_S);
  if(laserSens<laserSensLevel) {
    lcd.setCursor(0, 3);
    lcd.print("                     ");
    lcd.setCursor(0, 3);
    lcd.print(t);
    lcd.print(" ");
    lcd.print(endImpTime-startImpTime);
    lcd.print("ms ");
    lcd.print(laserSens);
  }







  enc1.tick();

  if (enc1.isLeft()) decState();
  if (enc1.isRight()) incState();
  if (enc1.isClick()) switchState();


  //


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

}
