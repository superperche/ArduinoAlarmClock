#include <Wire.h>
#include <LiquidCrystal.h>

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2, buzzer = 9;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int hours = 0;
int minutes = 0;
int seconds = 0;

void setup() {
  pinMode(buzzer, OUTPUT);
  formatPrint(hours);
  lcd.print(":");
  formatPrint(minutes);
  lcd.print(":");
  formatPrint(seconds);
}

void loop() {
  delay(1000); 
  seconds++; 
  if (seconds > 59){
    minutes++;
    seconds = 0;
  }
  if (minutes > 59){
    hours++;
    minutes = 0;
  }
  if (seconds > 10) {
    tone(buzzer, 1000);
  }
  lcd.clear();
  formatPrint(hours);
  lcd.print(":");
  formatPrint(minutes);
  lcd.print(":");
  formatPrint(seconds);
} 

void formatPrint(int n) {
  if (n<10) {
    lcd.print(0);
  }
  lcd.print(n);
}
