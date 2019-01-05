#include <Wire.h>
#include <LiquidCrystal.h>
#include "RTClib.h"
#include <stdlib.h>
#include <time.h>

#if defined(ARDUINO_ARCH_SAMD)
#define Serial SerialUSB
#endif

RTC_DS1307 rtc;

//DS1307 clock variables
char daysOfTheWeek[7][12] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

//Menu variables
String menuItems[] = {"SET ALARM", "STOPWATCH", "CHANGE TIME", "EXIT"};
int menuLength = sizeof(menuItems) / sizeof(String);
bool menu = false, first = false ;
int menuState = 0, selection = 0, index = 0;
int lastSelection = -1;

//Stopwatch variables
int previousTime[] = { -1, -1, -1};
int lastTime[] = { -1, -1, -1};
int currentTime[] = {0, 0, 0};
int pauseTime[] = {0, 0, 0};
bool paused = false, startTimer = false, timer = false;

//Alarm variables
int alarmValues[] = {0, 0, 0};
int mathInput[] = {0, 0, 0, 0};
bool alarm = false, alarmSet = false;

//Time-change variables
int nowTime[] = {0, 0, 0};
bool timeChange = false;

//Time-display variables
int lastSeconds;

//Math variables
int n1 = 0, n2 = 0, n3 = 0, answer = 0, e = 1;
bool math = false;

//Constants for wiring
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
const int buzzer = 9, buttonL = 6, buttonC = 7, buttonR = 8, buttonM = 10;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  //DS1307 Initialization
#ifndef ESP8266
#endif
  Serial.begin(57600);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (! rtc.isrunning()) {
    //Resets the time if necessary
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  //Buzzer Initialization
  pinMode(buzzer, OUTPUT);
  //LCD Initialization
  lcd.begin(16, 2);
  //Button Initialization
  pinMode(buttonL, INPUT);
  pinMode(buttonM, INPUT);
  pinMode(buttonR, INPUT);
  pinMode(buttonC, INPUT);
  //random seed
  srand(time(NULL));

  DateTime now = rtc.now();
  lastSeconds = now.second();
}

void loop() {
  menuState = digitalRead(buttonM);
  if (alarmSet) {
    DateTime now = rtc.now();
    if (daysOfTheWeek[now.dayOfTheWeek()] == daysOfTheWeek[alarmValues[0]] && now.hour() == alarmValues[2] && now.minute() == alarmValues[1] && now.second() == 0) {
      tone(buzzer, 1000);
      math = true;
      first = true;
      selection = 0;
      delay(1000);
    }
  }
  if (math) {
    mathDisplay();
  }
  else if (menu) {
    menuDisplay();
    if (menuState == HIGH) {
      if (menuItems[selection] == "EXIT") {
        menu = false;
        selection = 0;
        lastSelection = -1;
      }
      else if (menuItems[selection] == "SET ALARM") {
        alarm = true;
        menu = false;
        selection = 0;
        lastSelection = -1;
        first = true;
      }
      else if (menuItems[selection] == "STOPWATCH") {
        timer = true;
        menu = false;
        selection = 0;
        lastSelection = -1;
      }
      else if (menuItems[selection] == "CHANGE TIME") {
        first = true;
        timeChange = true;
        menu = false;
        selection = 0;
        lastSelection = -1;
      }
      delay(100);
    }
    else if (digitalRead(buttonL) == HIGH) {
      selection--;
      if (selection < 0) {
        selection = menuLength - 1;
      }
      delay(100);
    }
    else if (digitalRead(buttonR) == HIGH) {
      selection++;
      if (selection > menuLength - 1) {
        selection = 0;
      }
      delay(100);
    }
  }
  else if (menuState == HIGH && !menu && !math) {
    menu = true;
    alarm = false;
    timer = false;
    timeChange = false;
    selection = 0;
    lastSelection = -1;
    delay(100);
  }
  else if (alarm) {
    alarmDisplay();
  }
  else if (timer) {
    timerDisplay();
  }
  else if (timeChange) {
    timeChangeDisplay();
  }
  else {
    timeDisplay();
  }
  delay(100);
}

void mathDisplay() {
  if (first) {
    n1 = rand() % 100;
    n2 = rand() % 100;
    n3 = rand() % 100;
    if (n1 < 10) {
      n1 += 10;
    }
    if (n2 < 10) {
      n2 += 10;
    }
    if (n3 < 10) {
      n3 += 10;
    }
    answer = n1 * n2 + n3;
  }
  int confirm = digitalRead(buttonC);
  int left = digitalRead(buttonL);
  int right = digitalRead(buttonR);
  int enter = digitalRead(buttonM);
  if (confirm == HIGH || left == HIGH || right == HIGH || enter == HIGH || first) {
    delay(100);
    if (first) {
      first = false;
    }
    else if (enter == HIGH) {
      e = 1000;
      for (int i = 0; i < 4; i++) {
        answer -= mathInput[i] * e;
        e /= 10;
      }
      if (answer == 0) {
        noTone(buzzer);
        math = false;
      }
      else {
        answer = n1 * n2 + n3;
        for (int i = 0; i < 4; i++) {
          mathInput[i] = 0;
          selection = 0;
        }
      }
    }
    else if (left == HIGH) {
      mathInput[selection]++;
    }
    else if (right == HIGH) {
      mathInput[selection]--;
    }
    else if (confirm == HIGH) {
      selection++;
    }
    for (int i  = 0; i < 4; i++) {
      if (mathInput[i] > 9) {
        mathInput[i] = 0;
      }
      else if (mathInput[i] < 0) {
        mathInput[i] = 9;
      }
    }
    if (selection < 0) {
      selection = 3;
    }
    else if (selection > 3) {
      selection = 0;
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(n1);
    lcd.print("x");
    lcd.print(n2);
    lcd.print("+");
    lcd.print(n3);
    lcd.setCursor(12 + selection, 0);
    lcd.print("v");
    lcd.setCursor(12, 1);
    for (int i = 0; i < 4; i++) {
      lcd.print(mathInput[i]);
    }
  }
}

void alarmDisplay() {
  int confirm = digitalRead(buttonC);
  int left = digitalRead(buttonL);
  int right = digitalRead(buttonR);
  if (confirm == HIGH || left == HIGH || right == HIGH || first) {
    delay(100);
    if (first) {
      first = false;
    }
    else if (confirm == HIGH) {
      selection++;
      if (selection > 2) {
        alarmSet = true;
        selection = 0;
      }
    }
    else if (left == HIGH) {
      alarmValues[selection]++;
    }
    else if (right == HIGH) {
      alarmValues[selection]--;
    }
    if (alarmValues[0] > 6) {
      alarmValues[0] = 0;
    }
    else if (alarmValues[0] < 0) {
      alarmValues[0] = 6;
    }
    if (alarmValues[1] > 59) {
      alarmValues[1] = 0;
    }
    else if (alarmValues[1] < 0) {
      alarmValues[1] = 59;
    }
    if (alarmValues[2] > 23) {
      alarmValues[2] = 0;
    }
    else if (alarmValues[2] < 0) {
      alarmValues[2] = 23;
    }
    lcd.clear();
    if (selection == 0) {
      lcd.setCursor(0, 0);
    }
    else if (selection > 0) {
      lcd.setCursor(0, 1);
    }
    lcd.print(">");
    lcd.setCursor(1, 0);
    lcd.print("SET DATE: ");
    lcd.print(daysOfTheWeek[alarmValues[0]]);
    lcd.setCursor(1, 1);
    lcd.print("SET TIME: ");
    formatPrint(alarmValues[2]);
    lcd.print(":");
    formatPrint(alarmValues[1]);
  }

}

void timerDisplay() {
  DateTime now = rtc.now();
  //increments time
  if (lastTime[0] == -1) {
    lastTime[0] = now.hour();
    lastTime[1] = now.minute();
    lastTime[2] = now.second();
  }
  if (digitalRead(buttonL) == HIGH) {
    paused = true;
  }
  if (paused) {
    pauseTime[0] = currentTime[0];
    pauseTime[1] = currentTime[1];
    pauseTime[2] = currentTime[2];
    if (digitalRead(buttonR) == HIGH) {
      paused = false;
      lastTime[0] = now.hour();
      lastTime[1] = now.minute();
      lastTime[2] = now.second();
    }
  }
  else {
    currentTime[0] = now.hour();
    currentTime[1] = now.minute();
    currentTime[2] = now.second();
    currentTime[0] = currentTime[0] - lastTime[0] + pauseTime[0];
    currentTime[1] = currentTime[1] - lastTime[1] + pauseTime[1];
    currentTime[2] = currentTime[2] - lastTime[2] + pauseTime[2];
    if (currentTime[2] < 0) {
      currentTime[2] += 60;
      currentTime[1]--;
    }
    if (currentTime[1] < 0) {
      currentTime[1] += 60;
      currentTime[0]--;
    }
  }

  //prints time if changed
  if (previousTime[0] == -1 || previousTime[0] != currentTime[0] || previousTime[1] != currentTime[1] || previousTime[2] != currentTime[2] && !paused) {
    lcd.clear();
    lcd.print("STOPWATCH");
    lcd.setCursor(0, 1);
    formatPrint(currentTime[0]);
    lcd.print(":");
    formatPrint(currentTime[1]);
    lcd.print(":");
    formatPrint(currentTime[2]);
    lcd.setCursor(0, 2);
  }
  if (digitalRead(buttonC) == HIGH) {
    lastTime[0] = -1;
    pauseTime[0] = 0;
    pauseTime[1] = 0;
    pauseTime[2] = 0;
  }
  previousTime[0] = currentTime[0];
  previousTime[1] = currentTime[1];
  previousTime[2] = currentTime[2];
}

void timeChangeDisplay() {
  int confirm = digitalRead(buttonC);
  int left = digitalRead(buttonL);
  int right = digitalRead(buttonR);
  menuState = digitalRead(buttonL);
  DateTime now = rtc.now();
  if (first || confirm == HIGH || left == HIGH || right == HIGH) {
    delay(100);
    if (first) {
      nowTime[0] = now.hour();
      nowTime[1] = now.minute();
      nowTime[2] = now.second();
      first = false;
    }
    else {
      if (confirm == HIGH) {
        selection++;
        if (selection > 2) {
          selection = 0;
          rtc.adjust(DateTime(now.year(), now.month(), now.day(), nowTime[0], nowTime[1], nowTime[2]));
        }
      }
      else if (left == HIGH) {
        nowTime[selection]++;
      }
      else if (right == HIGH) {
        nowTime[selection]--;
      }
      if (selection == 0) {
        if (nowTime[selection] == 24) {
          nowTime[selection] = 0;
        }
        else if (nowTime[selection] == -1) {
          nowTime[selection] = 23;
        }
      }
      else {
        if (nowTime[selection] == 60) {
          nowTime[selection] = 0;
        }
        else if (nowTime[selection] == -1) {
          nowTime[selection] = 59;
        }
      }
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("CHANGE");
    lcd.setCursor(8 + selection * 3, 0);
    lcd.print("v");
    lcd.setCursor(0, 1);
    lcd.print("TIME");
    lcd.setCursor(8, 1);
    formatPrint(nowTime[0]);
    lcd.print(":");
    formatPrint(nowTime[1]);
    lcd.print(":");
    formatPrint(nowTime[2]);
  }

}

void menuDisplay() {
  if (lastSelection != selection) {
    lcd.clear();
    lcd.setCursor(6, 0);
    lcd.print("MENU");
    lcd.setCursor(0, 1);
    lcd.print(selection + 1);
    lcd.print(". " + menuItems[selection]);
    lastSelection = selection;
  }
}

void timeDisplay() {
  DateTime now = rtc.now();
  if (now.second() != lastSeconds) {
    lcd.clear();
    lcd.setCursor(0, 0);
    formatPrint(now.hour());
    lcd.print(":");
    formatPrint(now.minute());
    lcd.print(":");
    formatPrint(now.second());
    lcd.setCursor(0, 1);
    formatPrint(now.year());
    lcd.print("/");
    formatPrint(now.month());
    lcd.print("/");
    formatPrint(now.day());
    lcd.print(" (");
    lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
    lcd.print(")");
    lastSeconds = now.second();
  }
}

void formatPrint(int n) {
  if (n < 10) {
    lcd.print(0);
  }
  lcd.print(n);
}
