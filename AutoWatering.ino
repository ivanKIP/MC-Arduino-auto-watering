#include <DHT_U.h>
#include <DHT.h>
#include <LowPower.h>

const byte SB1_PIN = 2;                             // кнопка 1 (наполнить бочку)
const byte SB2_PIN = 3;                             // кнопка 2 (включить полив)
const byte LEVEL_LOW = 4;                           // датчик "нижний уровень"
const byte LEVEL_HIGH = 5;                          // датчик "верхний уровень"
const byte DHT_PIN = 6;                             // датчик температуры и влажности DHT22/AM2302
const byte WATERING_OUT = 11;                       // клапан включения полива
const byte FILL_TANK_OUT = 12;                      // клапан наполнения бочки
const byte BAT_VCC = A0;                            // вход измерения напряжения на аккумуляторе
const unsigned long wateringTime = 2*60*60*1000UL;  // время полива в мс (n часов * 60 мин * 60 сек)
const unsigned int longPressTime = 5000;            // время длительного нажатия
flagFillingTank
const byte debounceDelay = 40;                      // фильтр дребезга (мс)
unsigned long lastDebounceTime1 = 0;
unsigned long lastDebounceTime2 = 0;
bool lastButtonStateSB1 = LOW;
bool lastButtonStateSB2 = LOW;
bool buttonStateSB1 = LOW;
bool buttonStateSB2 = LOW;
bool SB1, SB2;                                      // признак нажатия кнопки
bool flagPress1 = false;                            // флаг нажатия на кнопку SB1
bool flagPress2 = false;                            // флаг нажатия на кнопку SB2
bool flagLongPress1 = false;                        // флаг длинного нажатия на кнопку SB1
bool flagLongPress2 = false;                        // флаг длинного нажатия на кнопку SB2
unsigned long lastPress1Time = 0;                   // счетчик времени нажатия SB1
unsigned long lastPress2Time = 0;                   // счетчик времени нажатия SB2
bool flagWatering = false;                          // флаг идет полив
bool flagFillingTank = false;                       // флаг наполнения бочки
unsigned long startWateringTime = 0;                // время начала полива
unsigned long startFillingTankTime = 0;             // время начала наполнения бочки

void setup() {
  Serial.begin(9600);
  pinMode(SB1_PIN, INPUT);
  pinMode(SB2_PIN, INPUT);
  pinMode(LEVEL_LOW, INPUT);
  pinMode(LEVEL_HIGH, INPUT);
  pinMode(WATERING_OUT, OUTPUT);
  pinMode(FILL_TANK_OUT, OUTPUT);
  digitalWrite(FILL_TANK_OUT, HIGH);
  digitalWrite(WATERING_OUT, HIGH);
}

void loop() {
  SB1 = readDebounceSB1();
  SB2 = readDebounceSB2();

  if (SB1 && !flagPress1) {
    lastPress1Time = millis();
    flagPress1 = true;
  }

  if (SB2 && !flagPress2) {
    lastPress2Time = millis();
    flagPress2 = true;
  }

  if (flagPress1 && millis() - lastPress1Time >= longPressTime) {
    flagLongPress1 = true;
  }

  if (flagPress2 && millis() - lastPress2Time >= longPressTime) {
    flagLongPress2 = true;
  }

  // было короткое нажатие SB1
  if (!SB1 && flagPress1 && !flagLongPress1) {
    flagPress1 = false;
    doFillTank();
  }

  // было короткое нажатие SB2
  if (!SB2 && flagPress2 && !flagLongPress2 && digitalRead(LEVEL_LOW)) {
    flagPress2 = false;
    doWatering();
  }

  // было длинное нажатие SB1 или SB2
  if ((!SB1 && flagPress1 && flagLongPress1) || (!SB2 && flagPress2 && flagLongPress2)) {
    resetAll();
  }

  // если идет полив и время полива вышло
  if (flagWatering && millis() - startWateringTime >= wateringTime) {
    resetAll();
  }

  // если идет наполнение бочки и сработал верхний датчик уровня
  if (flagFillingTank && (digitalRead(LEVEL_HIGH) || millis() - startFillingTankTime >= wateringTime / 2)) {
    digitalWrite(FILL_TANK_OUT, HIGH);
    flagFillingTank = false;
  }
}

bool readDebounceSB1() {
  bool reading = digitalRead(SB1_PIN);

  if (reading != lastButtonStateSB1) {
    // reset the debouncing timer
    lastDebounceTime1 = millis();
  }

  if ((millis() - lastDebounceTime1) > debounceDelay) {
    // if the button state has changed:
    if (reading != buttonStateSB1) {
      buttonStateSB1 = reading;
    }
  }

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonStateSB1 = reading;

  return buttonStateSB1;
}

bool readDebounceSB2() {
  bool reading = digitalRead(SB2_PIN);

  if (reading != lastButtonStateSB2) {
    // reset the debouncing timer
    lastDebounceTime2 = millis();
  }

  if ((millis() - lastDebounceTime2) > debounceDelay) {
    // if the button state has changed:
    if (reading != buttonStateSB2) {
      buttonStateSB2 = reading;
    }
  }

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonStateSB2 = reading;

  return buttonStateSB2;
}

void doFillTank() {
  digitalWrite(FILL_TANK_OUT, LOW);
  startFillingTankTime = millis();
  flagFillingTank = true;
}

void doWatering() {
  digitalWrite(WATERING_OUT, LOW);
  startWateringTime = millis();
  flagWatering = true;
}

void resetAll() {
  flagPress1 = false;
  flagPress2 = false;
  flagLongPress1 = false;
  flagLongPress2 = false;
  lastPress1Time = 0;
  lastPress2Time = 0;
  flagWatering = false;
  flagFillingTank = false;
  digitalWrite(FILL_TANK_OUT, HIGH);
  digitalWrite(WATERING_OUT, HIGH);
}
