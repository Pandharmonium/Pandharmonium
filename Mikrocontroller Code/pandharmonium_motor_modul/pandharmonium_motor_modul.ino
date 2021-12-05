
#include <Robojax_L298N_DC_motor.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

// motor 1 settings
#define CHA 0
#define ENA 19 // this pin must be PWM enabled pin if Arduino board is used
#define IN1 18
#define IN2 5



// hier WIFI-Details eintragen
const char* ssid = "ZYXEL-437";
const char* password = "5561693895110300";

// hier den ID-Parameter ändern (zum Testen auf 1 lassen)
const char* serverName = "http://pandharmonium.andreasrau.eu/_api/read.php?id=9";

WiFiClient wifi;
HTTPClient http;

int activity = 0;
int lastTriggered = 0;
int lastUpdated = 0;

// wie lang sind die Pausen zwischen den Beats...
int loActivityBreakDuration = 950; // ... bei 1% Aktivität (in ms)
int hiActivityBreakDuration = 10; //... bei 100% Aktivität (in ms)


const int CCW = 2; // do not change
const int CW  = 1; // do not change


//LED DIMMUNG
const int ledControlPin1 = 13;
const int freq = 5000;
const int ledChannel = 12;
const int resolution = 8;

#define motor1 1 // do not change

// for single motor
Robojax_L298N_DC_motor robot(IN1, IN2, ENA, CHA, true);  


void setup() {
  WiFi.begin(ssid, password);  
  Serial.begin(115200);
  robot.begin();


    // configure LED PWM functionalitites
   ledcSetup(ledChannel, freq, resolution);
    // attach the channel to the GPIO to be controlled  
   ledcAttachPin(ledControlPin1, ledChannel);


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


 //alle 10ms den neuen Stand vom Server abfragen
  if (millis() > lastUpdated + 10) {
    updateActivity();  
  }

  //ohne Aktivität nix los
  if (activity == 0) {
     robot.brake(1);
     ledcWrite(ledChannel, activity);
    return;
  }

  //wie lang ist die Pause beim aktuellen Aktivitätsniveau?
  // int b = map(activity, 0, 100, loActivityBreakDuration, hiActivityBreakDuration);
  
  //wenn die Pause vorbei ist, wieder triggern
//  if (millis() > lastTriggered) {
  //  lastTriggered = millis();
    
  
  ledcWrite(ledChannel, activity/4);
  robot.rotate(motor1, activity/2+50, CW);

  delay(5);

 
  
  
  }
