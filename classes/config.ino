class Config {
  public:
    unsigned int FS_KEY = 0; byte FS_KEY_addr = 0; // 0 2
    byte MODE = 1;  byte MODE_addr = 2; // 2  1
    unsigned int LAPS_N = 3; byte LAPS_N_addr = 3; // 3 2
    unsigned int SENSOR_IGNORE_TIME = 3; byte SENSOR_IGNORE_TIME_addr = 5; // 5  2
    boolean MUTE = false; byte MUTE_addr = 7; // 7  1
    boolean SAVE_RESULTS = false; byte SAVE_RESULTS_addr = 8; // 7  1
    boolean EXTERNAL_AUDIO_ON = false; byte EXTERNAL_AUDIO_ON_addr = 9; // 8  1
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
        EEPROM.put(EXTERNAL_AUDIO_ON_addr, EXTERNAL_AUDIO_ON);
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
      EEPROM.get(EXTERNAL_AUDIO_ON_addr, EXTERNAL_AUDIO_ON);
    };

    void print() {
      Serial.println("Config: ");
      Serial.print("FS_KEY: "); Serial.println(FS_KEY);
      Serial.print("MODE: "); Serial.println(MODE);
      Serial.print("LAPS_N: "); Serial.println(LAPS_N);
      Serial.print("SENSOR_IGNORE_TIME: "); Serial.println(SENSOR_IGNORE_TIME);
      Serial.print("MUTE: "); Serial.println(MUTE);
      Serial.print("SAVE_RESULTS: "); Serial.println(SAVE_RESULTS);
      Serial.print("EXTERNAL_AUDIO_ON: "); Serial.println(EXTERNAL_AUDIO_ON);
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
    void setEXTERNAL_AUDIO_ON() {
      EEPROM.put(EXTERNAL_AUDIO_ON_addr, EXTERNAL_AUDIO_ON);
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
    void setEXTERNAL_AUDIO_ON(boolean value) {
      if(value == EXTERNAL_AUDIO_ON) return;
      EXTERNAL_AUDIO_ON = value;
      EEPROM.put(EXTERNAL_AUDIO_ON_addr, EXTERNAL_AUDIO_ON);
    }

};
