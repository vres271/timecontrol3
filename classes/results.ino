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
            Serial3.print(stored_row.r);Serial3.print(" ");Serial3.print(millisToTime(stored_row.t));Serial3.print("\n");
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
