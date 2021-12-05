
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <ESP32Servo.h>


Servo myservo;  // create servo object to control a servo
// 16 servo objects can be created on the ESP32
 
int pos = 0;    // variable to store the servo position
// Recommended PWM GPIO pins on the ESP32 include 2,4,12-19,21-23,25-27,32-33 
int servoPin = 32;



// hier WIFI-Details eintragen
const char* ssid = "ZYXEL-437";
const char* password = "5561693895110300";

// hier den ID-Parameter ändern (zum Testen auf 1 lassen)
const char* serverName = "http://pandharmonium.andreasrau.eu/_api/read.php?id=16";

WiFiClient wifi;
HTTPClient http;

int activity = 0;
int lastTriggered = 0;
int lastUpdated = 0;

// wie lang sind die Pausen zwischen den Beats...
int loActivityBreakDuration = 2000; // ... bei 1% Aktivität (in ms)
int hiActivityBreakDuration = 10; //... bei 100% Aktivität (in ms)

void setup() {
  WiFi.begin(ssid, password);
  pinMode (26, OUTPUT);

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);    // standard 50 hz servo
  myservo.attach(servoPin); 
}

void updateActivity() {
  http.begin(wifi, serverName);  
  int httpResponseCode = http.GET();
  String activity_level = String(activity);
  
  if (httpResponseCode>0) {
    activity_level = http.getString();
  }
  http.end();

  int a = activity_level.toInt();
  if (a >= 0 && a <= 100) {
    activity = a;
  }
  
}

void loop() {

  digitalWrite(26, LOW);
  //alle 10ms den neuen Stand vom Server abfragen
  if (millis() > lastUpdated + 10) {
    updateActivity();  
  }

  //ohne Aktivität nix los
  if (activity == 0) {
    return;
      }

  //wie lang ist die Pause beim aktuellen Aktivitätsniveau?
  int b = map(activity, 0, 100, loActivityBreakDuration, hiActivityBreakDuration);
  
  //wenn die Pause vorbei ist, wieder triggern
  if (millis() > lastTriggered + b) {
    lastTriggered = millis();
    myservo.write(0);
    digitalWrite(26, HIGH);
    delay(400);
    myservo.write(90);
    digitalWrite(26, LOW);
    delay(400);

  }
}
