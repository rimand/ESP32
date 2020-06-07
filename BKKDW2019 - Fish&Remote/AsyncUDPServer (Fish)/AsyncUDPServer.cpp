#include <Arduino.h>
#include "WiFi.h"
#include "AsyncUDP.h"

#define power 40
#define power_2 50
#define PORT_CH 4448        // nemo 4444 , shark 4448

//const char* ssid     = "hacklab_2_4g";
//const char* password = "tiwanonoffice";
const char* ssid     = "kmmc";
const char* password = "matemaker";

AsyncUDP udp;

bool run_state = false;
bool btn = false;
bool prebtn = false;

unsigned long time_ = 0;
unsigned long time_2 = 0;

bool positions = false;
bool state_left = true;
bool state_right = true;

void setup()
{
  Serial.begin(115200);
  //pinMode(LED_BUILTIN, OUTPUT);
  //  pinMode(25, OUTPUT);
  //  pinMode(26, OUTPUT);
  //  pinMode(27, OUTPUT);
  //  pinMode(14, OUTPUT);
  //  pinMode(12, OUTPUT);

  ledcSetup(0, 5000, 8);  // ledcSetup(Channel, freq, resolution);   กำหนดค่าการทำงานของวงจรไทเมอร์
  ledcAttachPin(25, 0);   // ledcAttachPin(GPIO, Channel);            กำหนดขาพอร์ตที่ใช้เชื่อมต่อกับวงจรไทเมอร์
  ledcAttachPin(26, 1);
  ledcAttachPin(27, 2);
  ledcAttachPin(33, 3);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed");
    while (1) {
      delay(1000);
    }
  }
  if (udp.listen(PORT_CH)) {
    Serial.print("UDP Listening on IP: ");
    Serial.println(WiFi.localIP());
    udp.onPacket([](AsyncUDPPacket packet) {                  // calback fuction มันจะวน loop เองของมันในนี้ถึงแม้จะอยู่ที่ setup
      Serial.print("Data: ");
      //Serial.println((const char*)packet.data());
      //Serial.write(packet.data(), packet.length());

      String myString = (const char*)packet.data();
      int command = myString.toInt();
      Serial.println(command);

      if (command == 1) ledcWrite(0, power); //digitalWrite(25, HIGH);
      else ledcWrite(0, 0); //digitalWrite(25, LOW);
      if (command == 2) ledcWrite(1, power); //digitalWrite(26, HIGH);
      else ledcWrite(1, 0); //digitalWrite(26, LOW);

      if (command == 3) {
        //ledcWrite(2, power_2); //digitalWrite(27, HIGH);
        
//        if (millis() - time_2 > 400) {
//          if (state_left) {
//            ledcWrite(3, power_2 + 10);
//            state_left = false;
//          }
//          else {
//            ledcWrite(3, 0);
//            state_left = true;
//          }
//          time_2 = millis();
//        }
        
        ledcWrite(3, power_2 + 10);
        
      }
      else {
        if (!run_state)
          ledcWrite(3, 0);
      }

      if (command == 4) {
        //ledcWrite(3, power_2); //digitalWrite(14, HIGH);
        
//        if (millis() - time_2 > 400) {
//          if (state_right) {
//            ledcWrite(2, power_2 + 10);
//            state_right = false;
//          }
//          else {
//            ledcWrite(2, 0);
//            state_right = true;
//          }
//          time_2 = millis();
//        }
          ledcWrite(2, power_2 + 10);
        
      }
      else {
        if (!run_state)
          ledcWrite(2, 0);
      }

      if (command == 5) btn = true;
      else btn = false;
      if ((btn != prebtn) && btn) {
        run_state = !run_state;
      }

      prebtn = btn;
    });
  }
  //time_ = millis();
  time_2 = millis();
}

bool first = true;

void loop()
{
  delay(20);
  //udp.broadcast("Anyone here?");

  if (run_state) {
    first = true;
    if (millis() - time_2 > 500) {
      positions = !positions;
      time_2 = millis();
      if (positions) {
        ledcWrite(2, power_2 + 2);
        ledcWrite(3, 0);
        Serial.println("A");
      }
      else {
        ledcWrite(3, power_2 + 2);
        ledcWrite(2, 0);
        Serial.println("B");
      }
    }
  }
  else {
    if (first) {
      ledcWrite(3, 0);
      ledcWrite(2, 0);
      first = false;
    }
  }
}
