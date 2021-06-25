class State{
  public:
    byte route = 1;
    byte subroute = 2;
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
      if(route==0&&subroute<4) subroute++;
      if(route==1&&subroute<6) subroute++;
      if(route==2&&subroute<4) subroute++;
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
