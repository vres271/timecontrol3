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

void applyState(unsigned int route) {
	state.route = route;

	lcd.clear(); 
	lcd.setCursor(0, 0);
	lcd.print(route);
	lcd.setCursor(2, 0);
	lcd.print(menuTitles[route]);
}

void incState() {
	if(state.route < 3) applyState(state.route + 1);
}

void decState() {
	if(state.route > 0) applyState(state.route - 1);
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
