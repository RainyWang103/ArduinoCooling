#include <LiquidCrystal.h>
LiquidCrystal lcd(12,11,5,4,3,2);

const int ACPin = 1;
int value = 0;

void setup()
{
  Serial.begin(9600);
  lcd.begin(12,4);
}

void loop()
{
  value = analogRead(ACPin);
  Serial.println(value);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("raw: ");
  lcd.print(value);
  delay(200);
}
