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


void setup() {
  lcd.init();                      // initialize the lcd
  // Print a message to the LCD.
  lcd.backlight();
	lcd.setCursor(0, 0);
	lcd.print("TY PIDOR");

  // lcd.setCursor(3, 0);
  // lcd.print("Hello, world!");
  // lcd.setCursor(2, 1);
  // lcd.print("Ywrobot Arduino!");
  // lcd.setCursor(0, 2);
  // lcd.print("Arduino LCM IIC 2004");
  // lcd.setCursor(2, 3);
  // lcd.print("Power By Ec-yuan!");

  enc1.setType(TYPE2);

}

void loop() {

  enc1.tick();

  if (enc1.isTurn()) {     // если был совершён поворот (индикатор поворота в любую сторону)
    // ваш код
  }
  
  if (enc1.isRight()) {lcd.clear(); lcd.print("Right");  }        // если был поворот
  if (enc1.isLeft()) {lcd.clear(); lcd.print("Left"); }  
  if (enc1.isRightH()) {lcd.clear(); lcd.print("Right holded"); } // если было удержание + поворот
  if (enc1.isLeftH()) {lcd.clear(); lcd.print("Left holded"); }
  
  //if (enc1.isPress()) {lcd.print("Press");  }        // нажатие на кнопку (+ дебаунс)
  //if (enc1.isRelease()) {lcd.print("Release");  }    // то же самое, что isClick
  
  if (enc1.isClick()) {lcd.clear(); lcd.print("Click");  }        // одиночный клик
  if (enc1.isSingle()) {lcd.clear(); lcd.print("Single");  }      // одиночный клик (с таймаутом для двойного)
  if (enc1.isDouble()) {lcd.clear(); lcd.print("Double");  }      // двойной клик
  
  
  if (enc1.isHolded()) {lcd.clear(); lcd.print("Holded");  }      // если была удержана и энк не поворачивался

}