#include <Arduino.h>
#include "WiFi.h"
#include "AsyncUDP.h"
#include <string>

using namespace std;

#include <sstream>

#define LED_STATE_PIN 27

#define PORT_CH 4448

//const char* ssid     = "hacklab_2_4g";
//const char* password = "tiwanonoffice";
const char* ssid     = "kmmc";
const char* password = "matemaker";

AsyncUDP udp;

unsigned long time_;
bool stateLED = false;

void setup()
{
  Serial.begin(115200);
  analogReadResolution(10);
  pinMode(LED_STATE_PIN, OUTPUT);
  pinMode(14, INPUT);
  digitalWrite(LED_STATE_PIN, LOW);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed");
    while (1) {
      delay(200);
      double batt = (analogRead(35) * 8.4) / 1024;
      if (batt <= 6.2) blinks();
      else {
        for (int i = 0 ; i < 10 ; i++) {
          digitalWrite(LED_STATE_PIN, HIGH);
          delay(20);
          digitalWrite(LED_STATE_PIN, LOW);
          delay(20);
        }
      }
    }
  }
  if (udp.connect(IPAddress(192, 168, 1, 155), PORT_CH)) {
    Serial.println("UDP connected");
    udp.onPacket([](AsyncUDPPacket packet) {
      //      Serial.print("UDP Packet Type: ");
      //      Serial.print(packet.isBroadcast() ? "Broadcast" : packet.isMulticast() ? "Multicast" : "Unicast");
      //      Serial.print(", From: ");
      //      Serial.print(packet.remoteIP());
      //      Serial.print(":");
      //      Serial.print(packet.remotePort());
      //      Serial.print(", To: ");
      //      Serial.print(packet.localIP());
      //      Serial.print(":");
      //      Serial.print(packet.localPort());
      //      Serial.print(", Length: ");
      //      Serial.print(packet.length());
      //Serial.print(", Data: ");
      //Serial.write(packet.data(), packet.length());
      //Serial.println();
      //reply to the client
      //packet.printf("Got %u bytes of data", packet.length());
    });
    //Send unicast
    udp.print("Hello Server!");
  }

  time_ = millis();
  Serial.setTimeout(100);
}

void blinks() {
  if (millis() - time_ >= 500) {
    digitalWrite(LED_STATE_PIN, stateLED);
    stateLED = !stateLED;
    time_ = millis();
  }
}

void loop()
{
  delay(100);

  int command = 0;
  stringstream ss;

  double batt = (analogRead(35) * 8.4) / 1024;
  if (batt <= 6.2) blinks();
  else digitalWrite(LED_STATE_PIN, HIGH);
  Serial.println("Batt : " + String(batt));

  int valX = analogRead(36);
  int valY = analogRead(39);
  int button = !digitalRead(14);
  Serial.println(String(valX) + "\t" + String(valY) + "\t" + String(button));
  //Serial.println(map(valX - 360, 0, 511 - 360, 0, 40));
  //ss << valX;

  if (valY < 10) command = 1;
  else if (valY > 1020) command = 2;
  else if (valX < 200) command = 3;
  else if (valX > 800) command = 4;
  else if (button) command = 5;
  //else command = "0";

  //command = command + "&" + String(button);
  Serial.println(String(command) + " '");
  ss << command;
  std::string str = ss.str();
  const char *cstr = str.c_str();
  Serial.println(str.c_str());
  udp.broadcastTo(cstr, PORT_CH);
}
