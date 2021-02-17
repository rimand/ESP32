#include <Arduino.h>
#include "AccelStepper.h"
#include <SPI.h>
#include <Wire.h>
#include "LiquidCrystal_I2C.h"
#include "Bounce2.h"

//#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>
//#include <TM1637Display.h>
//#include <Smoothed.h>

//#define __MY_ARDUINO_
#define __MY_ESP32_

// #define _NEMA8_
//#define _NEMA23_
#define _NEMA23_GEAR50_

#define _ENABLE_SERIAL_

// Sec Per Rev
#define SPR_MAX 200
#define SPR_MIN 10

//================== Button ===================
#ifdef __MY_ARDUINO_
#define BUTTON_PIN_1 2
#define BUTTON_PIN_2 3
#define BUTTON_PIN_3 4
#define BUTTON_PIN_4 5
#endif

#ifdef __MY_ESP32_
//#define BUTTON_PIN_1 19  // 19
//#define BUTTON_PIN_2 5   // 5
//#define BUTTON_PIN_3 16  // 16
//#define BUTTON_PIN_4 17  // 17

#define BUTTON_PIN_1 16 // 19
#define BUTTON_PIN_2 17 // 5
#define BUTTON_PIN_3 5  // 16
#define BUTTON_PIN_4 19 // 17
#endif

Bounce btn1 = Bounce();
Bounce btn2 = Bounce();
Bounce btn3 = Bounce();
Bounce btn4 = Bounce();

bool stateBtn1 = 0;
bool stateBtn2 = 0;
bool stateBtn3 = 0;
bool stateBtn4 = 0;

double btnPlus = 1.0;

unsigned long button1_PressTimeStamp = 0;
unsigned long button2_PressTimeStamp = 0;

unsigned long button1_PressTimeStamp_more = 0;
unsigned long button2_PressTimeStamp_more = 0;

//================================ OLED ==============================
//#define SCREEN_WIDTH 128 // OLED display width, in pixels
//#define SCREEN_HEIGHT 64 // OLED display height, in pixels
//#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
//#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//================================ LCD I2c =============================

LiquidCrystal_I2C lcd(0x27, 16, 2);

//================== analog in ==================

#ifdef __MY_ARDUINO_
int knob_pin = 0;
int MAXADC = 1023;
#endif

#ifdef __MY_ESP32_
int knob_pin = 15;
int MAXADC = 4095;
#endif

//Smoothed <float> mySensor;
//float smoothedSensorValueAvg;
//float last_smoothedSensorValueAvg;

//================================ Stepper motor  =====================

#define motorInterfaceType 1
double RPMval = 1;
double SECPERREV = 60;     //60
double SECPERREVtemp = 60; //60
double lastSECPERREV = 60; //60
int angle_step = 0;

#ifdef __MY_ARDUINO_
#define dirPin 8
#define stepPin 9
#endif

#ifdef __MY_ESP32_
#define dirPin 4  //4
#define stepPin 2 //2
#endif

#ifdef _NEMA8_
double gear = 1;
#define pulsePerrev 200 * 16
#endif

#ifdef _NEMA23_
double gear = 1;
#define pulsePerrev 1600
#endif

#ifdef _NEMA23_GEAR50_
double gear = 50;
#define pulsePerrev 3200
// #define pulsePerrev 1600
#endif

AccelStepper stepper = AccelStepper(motorInterfaceType, stepPin, dirPin);

//=============================== 4 digi ==============================
//#define CLK 6
//#define DIO 7
//TM1637Display display(CLK, DIO);
//int count = 60;

//============================== General =========================

#define MAX_MODE 1
int _MODE_ = 0;
bool _lockMODE1_ = true;

unsigned long time_ = 0;
bool changeState = false;

int valKnob = 0;
int lastvalKnob = 0;

// ================================ Function ==========================

double ANGLE(int angle, float stepPerRev)
{
  double stepPerSec = 0;
  stepPerSec = ((stepPerRev * gear) * angle) / 360.0;
  return stepPerSec;
}

double RPM(float rpm, float stepPerRev)
{
  float stepPerSec = 0;
  stepPerSec = (rpm / 60.0) * stepPerRev * gear;
  return stepPerSec;
}

double sprTorpm(double spr_)
{
  if (spr_ == 0)
    return 0;
  else
    return (60.0 / spr_);
}

void update_button()
{
  btn1.update();
  btn2.update();
  btn3.update();
  btn4.update();
}

void CheckChange_Speed()
{
  if ((SECPERREV > 0 && lastSECPERREV < 0) || (SECPERREV < 0 && lastSECPERREV > 0) || (SECPERREV == 0) || (lastSECPERREV != SECPERREV))
  {
    changeState = true;
  }
}

void mini_lcd_update()
{
  lcd.setCursor(0, 0);
  if (_MODE_ == 0)
  {
    if (SECPERREV != 0)
      lcd.print(String(SECPERREVtemp) + " s" + " (" + String(sprTorpm(SECPERREVtemp)) + ")");
    else
      lcd.print(String(SECPERREVtemp) + " s" + " (0)");
  }
  else if (_MODE_ == 1)
  {
    if (angle_step != 0)
      lcd.print(String(angle_step) + " degree");
    else
      lcd.print(String(angle_step) + " degree");
  }
}

void lcd_update()
{
  //  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 0);

  if (_MODE_ == 0)
  {
    if (SECPERREV != 0)
      lcd.print(String(SECPERREV) + " s" + " (" + String(sprTorpm(SECPERREV)) + ")");
    else
      lcd.print(String(SECPERREV) + " s" + " (0)");
    lcd.setCursor(0, 1);
    lcd.print("Continue");
  }
  else if (_MODE_ == 1)
  {
    if (angle_step != 0)
      lcd.print(String(angle_step) + " degree");
    else
      lcd.print(String(angle_step) + " degree");

    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("Angle");
  }
}

void lcd_update_temp()
{
  //  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 0);

  if (_MODE_ == 0)
  {
    if (SECPERREVtemp != 0)
      lcd.print(String(SECPERREVtemp) + " s" + " (" + String(sprTorpm(SECPERREVtemp)) + ")");
    else
      lcd.print(String(SECPERREVtemp) + " s" + " (0)");
    lcd.setCursor(0, 1);
    lcd.print("Continue");
  }
  else if (_MODE_ == 1)
  {
    if (angle_step != 0)
      lcd.print(String(angle_step) + " degree");
    else
      lcd.print(String(angle_step) + " degree");

    lcd.setCursor(0, 1);
    lcd.print("Angle");
  }
}

void speed_update()
{
  stepper.setMaxSpeed(RPM(sprTorpm(SECPERREV), pulsePerrev));
  stepper.setAcceleration(RPM(sprTorpm(SECPERREV), pulsePerrev));
}

// ====================== MODE ===================

void MODE_0()
{
  CheckChange_Speed();
  // if (SECPERREV > 0)
  // {
  //   if (stepper.distanceToGo() <= RPM(sprTorpm(SECPERREV), pulsePerrev))
  //   {
  //     if (!changeState)
  //       stepper.move(RPM(sprTorpm(SECPERREV), pulsePerrev));
  //   }
  // }
  // else if (SECPERREV < 0)
  // {
  //   if (stepper.distanceToGo() >= RPM(sprTorpm(SECPERREV), pulsePerrev))
  //   {
  //     if (!changeState)
  //       stepper.move(RPM(sprTorpm(SECPERREV), pulsePerrev));
  //   }
  // }
  stepper.setSpeed(RPM(sprTorpm(SECPERREV), pulsePerrev));
  stepper.runSpeed();

  if (stepper.distanceToGo() == 0)
  {
    stepper.setMaxSpeed(RPM(sprTorpm(SECPERREV), pulsePerrev));
    stepper.setAcceleration(RPM(sprTorpm(SECPERREV), pulsePerrev));
    changeState = false;
  }

  lastSECPERREV = SECPERREV;
}

void MODE_1()
{
  stepper.setMaxSpeed(RPM(sprTorpm(SPR_MIN - 5), pulsePerrev));
  stepper.setAcceleration(RPM(sprTorpm(SPR_MIN - 5), pulsePerrev));
  if (stepper.distanceToGo() == 0)
  {
    stepper.move(ANGLE(angle_step, pulsePerrev));
  }
}

//================================== Update Function ===============

void updateFunc()
{
  update_button();
  if (btn1.changed())
  {
    int deboucedInput = btn1.read();
    if (deboucedInput == LOW)
    {
      stateBtn1 = true;
      button1_PressTimeStamp = millis();
      button1_PressTimeStamp_more = millis();
      if (_MODE_ == 0)
      {
        SECPERREVtemp -= 0.1;
        if (SECPERREVtemp < -SPR_MAX)
          SECPERREVtemp = -SPR_MAX;
        if (SECPERREVtemp < SPR_MIN && SECPERREVtemp > 0)
          SECPERREVtemp = 0;
        if (SECPERREVtemp < 0 && SECPERREVtemp > -0.5)
          SECPERREVtemp = -SPR_MIN;
        Serial.println("Sec per Rev:" + String(SECPERREVtemp) + " (RPM:" + String(sprTorpm(SECPERREVtemp)) + ")");
      }
      else if (_MODE_ == 1)
      {
        angle_step -= 1;
        if (angle_step < -360)
          angle_step = 360;
        Serial.println("Angle : " + String(angle_step));
      }
      lcd_update_temp();
      lcd.setCursor(15, 1);
      lcd.print(" ");
    }
    else
    {
      stateBtn1 = false;
      btnPlus = 1.0;
      button1_PressTimeStamp = millis();
      button1_PressTimeStamp_more = millis();
    }
  }

  if (stateBtn1)
  {
    if (millis() - button1_PressTimeStamp >= 500)
    {
      button1_PressTimeStamp = millis();
      if (_MODE_ == 0)
      {
        SECPERREVtemp -= btnPlus;
        if (SECPERREVtemp < -SPR_MAX)
          SECPERREVtemp = -SPR_MAX;
        if (SECPERREVtemp < SPR_MIN && SECPERREVtemp > 0)
          SECPERREVtemp = 0;
        if (SECPERREVtemp < 0 && SECPERREVtemp > -0.5)
          SECPERREVtemp = -SPR_MIN;
        Serial.println("Sec per Rev:" + String(SECPERREVtemp) + " (RPM:" + String(sprTorpm(SECPERREVtemp)) + ")");
      }
      else if (_MODE_ == 1)
      {
        angle_step -= btnPlus;
        if (angle_step < -360)
          angle_step = 360;
        Serial.println("Angle : " + String(angle_step));
      }
      mini_lcd_update();
    }
    if (millis() - button1_PressTimeStamp_more >= 1500)
    {
      button1_PressTimeStamp_more = millis();
      btnPlus += 2.0;
    }
  }

  if (btn2.changed())
  {
    int deboucedInput = btn2.read();
    if (deboucedInput == LOW)
    {
      stateBtn2 = true;
      button2_PressTimeStamp = millis();
      button2_PressTimeStamp_more = millis();
      if (_MODE_ == 0)
      {
        SECPERREVtemp += 0.1;
        if (SECPERREVtemp > SPR_MAX)
          SECPERREVtemp = SPR_MAX;
        if (SECPERREVtemp > -SPR_MIN && SECPERREVtemp < 0)
          SECPERREVtemp = 0;
        if (SECPERREVtemp > 0 && SECPERREVtemp < 0.5)
          SECPERREVtemp = SPR_MIN;
        Serial.println("Sec per Rev:" + String(SECPERREVtemp) + " (RPM:" + String(sprTorpm(SECPERREVtemp)) + ")");
      }
      else if (_MODE_ == 1)
      {
        angle_step += 1;
        if (angle_step > 360)
          angle_step = -360;

        Serial.println("Angle : " + String(angle_step));
      }
      lcd_update_temp();
      lcd.setCursor(15, 1);
      lcd.print(" ");
    }
    else
    {
      stateBtn2 = false;
      btnPlus = 1.0;
      button2_PressTimeStamp = millis();
      button2_PressTimeStamp_more = millis();
    }
  }

  if (stateBtn2)
  {
    if (millis() - button2_PressTimeStamp >= 500)
    {
      button2_PressTimeStamp = millis();
      if (_MODE_ == 0)
      {
        SECPERREVtemp += btnPlus;
        if (SECPERREVtemp > SPR_MAX)
          SECPERREVtemp = SPR_MAX;
        if (SECPERREVtemp > -SPR_MIN && SECPERREVtemp < 0)
          SECPERREVtemp = 0;
        if (SECPERREVtemp > 0 && SECPERREVtemp < 0.5)
          SECPERREVtemp = SPR_MIN;
        Serial.println("Sec per Rev:" + String(SECPERREVtemp) + " (RPM:" + String(sprTorpm(SECPERREVtemp)) + ")");
      }
      else if (_MODE_ == 1)
      {
        angle_step += btnPlus;
        if (angle_step > 360)
          angle_step = -360;
        Serial.println("Angle : " + String(angle_step));
      }
      mini_lcd_update();
    }
    if (millis() - button2_PressTimeStamp_more >= 1500)
    {
      button2_PressTimeStamp_more = millis();
      btnPlus += 2.0;
    }
  }

  if (btn3.changed())
  {
    int deboucedInput = btn3.read();
    if (deboucedInput == LOW)
    {
      stepper.setCurrentPosition(0);
      _MODE_++;
      if (_MODE_ > MAX_MODE)
      {
        _MODE_ = 0;
        _lockMODE1_ = true;
      }
      if (_MODE_ == 0)
      {
        speed_update();
      }
      SECPERREVtemp = SECPERREV;

      // Serial.println("Sec per Rev:" + String(SECPERREV) + " (RPM:" + String(sprTorpm(SECPERREV)) + ")");
      Serial.println("Change Mode (MODE) : " + String(_MODE_));

      // if (_MODE_ != 1)
      //   stepper.setCurrentPosition(0);
      Serial.println("Position:" + String(stepper.distanceToGo()));

      lcd_update();
      lcd.setCursor(15, 1);
      lcd.print(" ");
    }
  }

  if (btn4.changed())
  {
    int deboucedInput = btn4.read();
    if (deboucedInput == LOW)
    {
      if (_MODE_ == 0)
      {
        stepper.setCurrentPosition(0);
        Serial.println("Sec per Rev:" + String(SECPERREV) + " (RPM:" + String(sprTorpm(SECPERREV)) + ")");
        if (SECPERREVtemp == SECPERREV && SECPERREVtemp != 0)
        {
          _lockMODE1_ = !_lockMODE1_;
        }
        else if (SECPERREVtemp != SECPERREV && SECPERREVtemp != 0)
        {
          _lockMODE1_ = false;
        }

        if (!_lockMODE1_)
          Serial.println("Motor Start");
        else
          Serial.println("Motor Stop");

        SECPERREV = SECPERREVtemp;

        // if (_MODE_ != 0)
        speed_update();

        if (!_lockMODE1_)
        {
          lcd.setCursor(15, 1);
          lcd.print("+");
        }
        else
        {
          stepper.stop();
          lcd.setCursor(15, 1);
          lcd.print(" ");
        }
      }
      else if (_MODE_ == 1)
      {
        MODE_1();
      }
      Serial.println();
    }
  }

  if (_MODE_ == 0 && !_lockMODE1_)
    MODE_0();
  if (_MODE_ == 1)
    stepper.run();
}

//================================ Main ==============================

void setup()
{

  Serial.begin(115200);
  Serial.setTimeout(10);

  //========= (Button) ================

  btn1.attach(BUTTON_PIN_1, INPUT_PULLUP);
  btn2.attach(BUTTON_PIN_2, INPUT_PULLUP);
  btn3.attach(BUTTON_PIN_3, INPUT_PULLUP);
  btn4.attach(BUTTON_PIN_4, INPUT_PULLUP);

  btn1.interval(1);
  btn2.interval(1);
  btn3.interval(1);
  btn4.interval(1);

  // ============= (4 digi) =============
  //  display.setBrightness(0x0f);
  //  display.showNumberDec(RPMval, false);

  // ============= (OLED) =============
  //  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
  //    Serial.println(F("SSD1306 allocation failed"));
  //    for (;;); // Don't proceed, loop forever
  //  }
  //  display.clearDisplay();
  //  display.setTextSize(2);      // Normal 1:1 pixel scale
  //  display.setTextColor(SSD1306_WHITE); // Draw white text
  //  display.setCursor(0, 0);
  //  display.write("Hello");
  //  display.display();

  //============== (LCD I2c) ===================

  lcd.begin();
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 0);
  //  lcd.print("Hello, world!");
  lcd_update();

  // ============= (Stepper motor) =============
  stepper.setMaxSpeed(RPM(sprTorpm(SECPERREV), pulsePerrev));
  stepper.setAcceleration(RPM(sprTorpm(SECPERREV), pulsePerrev));

  //=========== analog in===============

  //  mySensor.begin(SMOOTHED_AVERAGE, 10);
  //  mySensor.clear();

  //  while (!Serial.available());
  time_ = millis();
}

void loop()
{
#ifdef _ENABLE_SERIAL_
  if (Serial.available())
  {
    double input = Serial.parseFloat();
    SECPERREV = input;

    Serial.println("input:" + String(input));
    Serial.println("SPR:" + String(SECPERREV) + " (RPM:" + String(sprTorpm(SECPERREV)) + ")");
    Serial.println("raw:" + String(RPM(sprTorpm(SECPERREV), pulsePerrev)));
    Serial.println();

    lcd_update();

    if (_MODE_ != 0)
    {
      speed_update();
    }
  }
#endif

  if (millis() - time_ >= 1000)
  {
    //    Serial.println(stepper.currentPosition());
    //    Serial.println(stepper.distanceToGo());
    time_ = millis();
  }
  updateFunc();
}