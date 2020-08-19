const char *menuTitles[] = {
	"Home",
	"Race",
	"Settings",
	"Results",
};

struct State{
  unsigned int route = 0;
  unsigned int subroute = 0;
};

State state;

void applyState() {
	//lcd.clear(); 
	lcd.setCursor(0, 0);
	lcd.print("                           ");
	lcd.setCursor(0, 0);
	lcd.print(state.route);
	lcd.print(menuTitles[state.route]);
	lcd.print("-");
	lcd.print(state.subroute);
	if(state.subroute) {
		lcd.print("->");
		lcd.print(state.subroute);
	}
}

void incState() {
	if(state.subroute) {
		state.subroute++;
		applyState();
	} else {
		if(state.route < 3) {
			state.route++;
			applyState();
		}
	}
}

void decState() {
	if(state.subroute) {
		state.subroute--;
		applyState();
	} else {
		if(state.route > 0) {
			lcd.print(">>>>>");
			state.route--;
			applyState();
		}
	}
}

void switchState() {
	state.subroute = 1;
	applyState();
}

// void route() {
// 	switch () {
// 	    case :
// 	      break;
// 	    case :
// 	      // do something
// 	      break;
// 	    default:
// 	      // do something
// 	}
// }
