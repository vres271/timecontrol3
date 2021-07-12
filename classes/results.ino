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
      log("Result saved");
      return last_addr;
    }
    void printAll(int r=0) {
      unsigned int last_addr = EEPROM.get(RESULTS_EEPROM_SHIFT, last_addr);
      if(last_addr==0) {
        log("\nResults is empty\n");
        return;
      }
      log("\nResults\n");

      resultRow stored_row; 
      for(int i=0; i*sizeof(resultRow)<=(last_addr-sizeof(resultRow)); i++) {
          stored_row = read(i);
          if(r==0 || stored_row.r==r) {
            log(String(stored_row.r)+" "+String(millisToTime(stored_row.t))+"\n");
          }
        if(i>99) return;
      }
    }
    void getSummary(boolean print) {
      unsigned int last_addr = EEPROM.get(RESULTS_EEPROM_SHIFT, last_addr);
      if(last_addr==0) return;
      resultRow stored_row; 
      unsigned int size = int((last_addr-sizeof(resultRow))/sizeof(resultRow))+1;
      unsigned long resultsArray[size][2];
      unsigned long grouped[size][2];
      unsigned long summaryArray[size][2];
      for(int i=0; i<size; i++) {
          stored_row = read(i);
          resultsArray[i][0] = stored_row.r;
          resultsArray[i][1] = stored_row.t;
          summaryArray[i][0] = 0;
          summaryArray[i][1] = 0;
        if(i>9999) return;
      }

      for(int j=0; j<size; j++) {
        unsigned long minR[2] = {0,0};
        for(int i=0; i<size; i++) {
          if((resultsArray[i][1]<minR[1] || minR[1]==0) && resultsArray[i][1] != 0) {
            minR[0] = resultsArray[i][0];
            minR[1] = resultsArray[i][1];
          }
        }
        for(int i=0; i<size; i++) {
          if(resultsArray[i][0]==minR[0]) {
            resultsArray[i][0] = 0;
            resultsArray[i][1] = 0;
          }
        }
        summaryArray[j][0] = minR[0];
        summaryArray[j][1] = minR[1];

      }

      log("\nSummary\n");

      for(int i=0; i<size; i++) {
        if(summaryArray[i][1]) {
          log(String(summaryArray[i][0])+" "+String(millisToTime(summaryArray[i][1]))+"\n");
          if(i<=2) {
            lcd.setCursor(0, i+1); lcd.print(i+1); lcd.print(": R"); lcd.print(summaryArray[i][0]); lcd.setCursor(8, i+1); lcd.print(": "); lcd.print(millisToTime(summaryArray[i][1]));
          }
        }
      }


    }
    void clearAll() {
      log("\nClear all\n");
      last_addr = 0;
      EEPROM.put(RESULTS_EEPROM_SHIFT, last_addr);
      //Serial.println("last_addr: ");  Serial.println(last_addr);
    }

};
