//Radio
const uint8_t num_channels = 128;
uint8_t values[num_channels];

const int num_reps = 100;

int serial_putc( char c, FILE * ) {
  Serial.write( c );
  return c;
}

void printf_begin(void) {
  fdevopen( &serial_putc, 0 );
}


class Radio{
  public:
    uint8_t num_channels = 128;
    boolean inited = false;

	void powerOn() {
	  digitalWrite(RADIO_PWR_ON, HIGH);
	}

	void powerOff() {
	  digitalWrite(RADIO_PWR_ON, LOW);
	}

    void init() {
	  powerOn();
	  delay(2000);

	  printf_begin();
	  radio.begin();
	  radio.setAutoAck(false);
	  radio.startListening();

	  radio.printDetails();  // Вот эта строка напечатает нам что-то, если все правильно соединили.
	  delay(1000);              // И посмотрим на это пять секунд.

	  radio.stopListening();
	  inited = true;
      
    }
    
    void initForTX() {
	  powerOn();
	  delay(2000);

	  radio.begin(); //активировать модуль
	  radio.setAutoAck(1);         //режим подтверждения приёма, 1 вкл 0 выкл
	  radio.setRetries(0, 15);    //(время между попыткой достучаться, число попыток)
	  radio.enableAckPayload();    //разрешить отсылку данных в ответ на входящий сигнал
	  radio.setPayloadSize(32);     //размер пакета, в байтах

	  radio.openWritingPipe(address[0]);   //мы - труба 0, открываем канал для передачи данных
	  radio.setChannel(0x60);  //выбираем канал (в котором нет шумов!)

	  radio.setPALevel (RF24_PA_MAX); //уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
	  radio.setDataRate (RF24_250KBPS); //скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
	  //должна быть одинакова на приёмнике и передатчике!
	  //при самой низкой скорости имеем самую высокую чувствительность и дальность!!

	  radio.powerUp(); //начать работу
	  //radio.printDetails();
	  radio.stopListening();  //не слушаем радиоэфир, мы передатчик

	  inited = true;
      
    }

    void close() {
	  inited = false;
	  powerOff();
    }

	void initForScan() {
	  powerOn();
	  delay(2000);

	  printf_begin();
	  radio.begin();
	  radio.setAutoAck(false);
	  radio.startListening();

	  radio.printDetails();  // Вот эта строка напечатает нам что-то, если все правильно соединили.
	  delay(1000);              // И посмотрим на это пять секунд.

	  radio.stopListening();
	  int i = 0;    // А это напечатает нам заголовки всех 127 каналов
	  while ( i < num_channels )  {
	    printf("%x",i>>4);
	    ++i;
	  }
	  printf("\n\r");
	  i = 0;
	  while ( i < num_channels ) {
	    printf("%x",i&0xf);
	    ++i;
	  }
	  printf("\n\r");
	  inited = true;
	}

	void scanRadio() {
	  if(!inited) return;
	  memset(values,0,sizeof(values));
	  int rep_counter = num_reps;
	  while (rep_counter--) {
	    int i = num_channels;
	    while (i--) {
	      radio.setChannel(i);
	      radio.startListening();
	      delayMicroseconds(128);
	      radio.stopListening();
	      if ( radio.testCarrier() )
	        ++values[i];
	    }
	  }
	  int i = 0;
	  while ( i < num_channels ) {
	    printf("%x",min(0xf,values[i]&0xf));
	    ++i;
	  }
	  printf("\n\r");

	}

	void sendRadio() {
	  if(!inited) return;
	  Serial.print("Sent: "); Serial.println(counter);
	  radio.write(&counter, sizeof(counter));
	  counter++;
	  delay(10);
	}

	void ATCommands() {
	  if (Serial3.available()) {
	      char c = Serial3.read();  // читаем из software-порта
	      Serial.print(c);                   // пишем в hardware-порт
	  }
	  if (Serial.available()) {
	      char c = Serial.read();      // читаем из hardware-порта
	      Serial3.write(c);            // пишем в software-порт
	  }
	}

};




