const char *menuTitles[] = {
	"Home",
	"Race",
	"Settings",
};

struct Menu{
  byte page = 0;
  byte subPage = 0;

};

Menu menu;


void menuLeft() {
	if(menu.page<=0) menu.page = 2;
	menu.page--;
	lcd.clear();
	lcd.setCursor(0, 0);
  lcd.print(menuTitles[menu.page]);
};

void menuRight() {
	if(menu.page>=2) menu.page = 0;
	menu.page++;
	lcd.clear();
	lcd.setCursor(0, 0);
  lcd.print(menuTitles[menu.page]);
};

void enterPage() {
	menu.subPage++;
	lcd.setCursor(15, 0);
  lcd.print("+");
};

void exitPage() {
	menu.subPage--;
	lcd.setCursor(15, 0);
  lcd.print(" ");
};

