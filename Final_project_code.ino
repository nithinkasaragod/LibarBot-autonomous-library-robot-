// === Arduino UNO + CNC Shield Book Picking Robot with Reshelving Mode ===

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <AccelStepper.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define X_STEP_PIN 2
#define X_DIR_PIN 5
#define Y_STEP_PIN 3
#define Y_DIR_PIN 6
#define Z_STEP_PIN 4
#define Z_DIR_PIN 7
#define STEPPER_ENABLE 8

AccelStepper stepperX(AccelStepper::DRIVER, X_STEP_PIN, X_DIR_PIN);
AccelStepper stepperY(AccelStepper::DRIVER, Y_STEP_PIN, Y_DIR_PIN);
AccelStepper stepperZ(AccelStepper::DRIVER, Z_STEP_PIN, Z_DIR_PIN);

#define X_LIMIT 11
#define Y_BACK_LIMIT 12
#define Z_LIMIT A0
#define Y_FRONT_LIMIT A1

#define NEXT_BUTTON 9
#define SELECT_BUTTON 10

const int numPositions = 4;
const char *positionTitles[numPositions] = {"Book 1", "Book 2", "Book 3", "Center"};
int positionValues[numPositions][2] = {
  {680, 700},
  {680, 1950},
  {2120, 1200},
  {1400, 1000}
};

int selectedBook = 0;
int selectedMode = 0; // 0 = Book Picking, 1 = Book Reshelving

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Select Mode:");

  pinMode(STEPPER_ENABLE, OUTPUT);
  digitalWrite(STEPPER_ENABLE, LOW);

  stepperX.setMaxSpeed(1000); stepperX.setAcceleration(500);
  stepperY.setMaxSpeed(1000); stepperY.setAcceleration(500);
  stepperZ.setMaxSpeed(1000); stepperZ.setAcceleration(500);

  pinMode(X_LIMIT, INPUT_PULLUP);
  pinMode(Y_BACK_LIMIT, INPUT_PULLUP);
  pinMode(Y_FRONT_LIMIT, INPUT_PULLUP);
  pinMode(Z_LIMIT, INPUT_PULLUP);

  pinMode(NEXT_BUTTON, INPUT_PULLUP);
  pinMode(SELECT_BUTTON, INPUT_PULLUP);

  homeAxes();
  updateLCD();
}

void loop() {
  if (digitalRead(NEXT_BUTTON) == LOW) {
    selectedMode = (selectedMode + 1) % 2;
    updateLCD();
    delay(300);
  }

  if (digitalRead(SELECT_BUTTON) == LOW) {
    delay(300);
    selectBook();
    if (selectedMode == 0) {
      pickBook(selectedBook);
    } else {
      reshelfBook(selectedBook);
    }
    updateLCD();
  }
}

void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  if (selectedMode == 0) lcd.print("Mode: Pick Book");
  else lcd.print("Mode: Reshelving");
  lcd.setCursor(0, 1);
  lcd.print("Press SELECT");
}

void selectBook() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Select Book:");
  int currentBook = 0;
  while (true) {
    lcd.setCursor(0, 1);
    lcd.print("                "); // Clear line
    lcd.setCursor(0, 1);
    lcd.print(positionTitles[currentBook]);
    if (digitalRead(NEXT_BUTTON) == LOW) {
      currentBook = (currentBook + 1) % (numPositions - 1); // Only 3 books
      delay(300);
    }
    if (digitalRead(SELECT_BUTTON) == LOW) {
      selectedBook = currentBook;
      delay(300);
      break;
    }
  }
  lcd.clear();
  lcd.print("Working...");
}

void homeAxes() {
  while (digitalRead(Y_BACK_LIMIT) == HIGH) {
    stepperY.setSpeed(-500);
    stepperY.runSpeed();
  }
  stepperY.setCurrentPosition(0);

  while (digitalRead(X_LIMIT) == HIGH) {
    stepperX.setSpeed(-500);
    stepperX.runSpeed();
  }
  stepperX.setCurrentPosition(0);

  while (digitalRead(Z_LIMIT) == HIGH) {
    stepperZ.move(-10);
    stepperZ.run();
  }
  stepperZ.setCurrentPosition(0);
}

void moveYToFront() {
  while (digitalRead(Y_FRONT_LIMIT) == HIGH) {
    stepperY.setSpeed(500);
    stepperY.runSpeed();
  }
  stepperY.setSpeed(0);
}

void moveYToBack() {
  while (digitalRead(Y_BACK_LIMIT) == HIGH) {
    stepperY.setSpeed(-500);
    stepperY.runSpeed();
  }
  stepperY.setSpeed(0);
}

void pickBook(int index) {
  int targetX = positionValues[index][0];
  int liftZ = positionValues[index][1];

  stepperZ.moveTo(liftZ);
  while (stepperZ.distanceToGo() != 0) stepperZ.run();

  stepperX.moveTo(targetX);
  while (stepperX.distanceToGo() != 0) stepperX.run();

  moveYToFront();

  stepperZ.moveTo(liftZ + 200);
  while (stepperZ.distanceToGo() != 0) stepperZ.run();

  moveYToBack();

  stepperX.moveTo(positionValues[3][0]);
  while (stepperX.distanceToGo() != 0) stepperX.run();

  stepperZ.moveTo(positionValues[3][1]);
  while (stepperZ.distanceToGo() != 0) stepperZ.run();

  moveYToFront();

  stepperZ.moveTo(0);
  while (stepperZ.distanceToGo() != 0) stepperZ.run();

  homeAxes();
}

void reshelfBook(int index) {
  homeAxes();

  stepperZ.moveTo(positionValues[3][1] - 300);
  while (stepperZ.distanceToGo() != 0) stepperZ.run();

  stepperX.moveTo(positionValues[3][0]);
  while (stepperX.distanceToGo() != 0) stepperX.run();

  moveYToFront();

  stepperZ.moveTo(positionValues[3][1] + 200);
  while (stepperZ.distanceToGo() != 0) stepperZ.run();

  moveYToBack();

  stepperX.moveTo(positionValues[index][0]);
  while (stepperX.distanceToGo() != 0) stepperX.run();

  stepperZ.moveTo(positionValues[index][1] + 300);
  while (stepperZ.distanceToGo() != 0) stepperZ.run();

  moveYToFront();

  stepperZ.moveTo(positionValues[index][1] - 500);
  while (stepperZ.distanceToGo() != 0) stepperZ.run();

  homeAxes();
}
