#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>



// hier WIFI-Details eintragen
const char* ssid = "ZYXEL-437";
const char* password = "5561693895110300";

WiFiClient wifi;
HTTPClient http;

const char* serverName = "http://pandharmonium.andreasrau.eu/_api/readmultiple.php?ids=";
char url[64];

// wie viele Klangobjekte soll der ESP steuern?
const int objectCount = 4;

// hier die ID-Parameter eintragen, für die Website
const char* ids[objectCount] = {"8", "5", "2", "3"};

// hier die Pins für die dazugehörigen Klangobjekte eintragen
int pins[objectCount] = {25, 26, 27, 19};

// wie lang sind die Pausen zwischen den Beats...
int loActivityBreakDurations[objectCount] = {700, 700}; // ... bei 1% Aktivität (in ms)
int hiActivityBreakDurations[objectCount] = {10, 10}; // ... bei 100% Aktivität (in ms)

int activities[objectCount] = {0, 0};
int lastTriggered[objectCount] = {0, 0};

int lastUpdated = 0;

void setup() {
  WiFi.begin(ssid, password);

  for (int i = 0; i < objectCount; i++) {
    pinMode (pins[i], OUTPUT);
    if (i == 0) {
      strcpy(url,serverName); 
    }
    strcat(url,ids[i]);
    if (i < objectCount-1) {
      strcat(url,"-");  
    }
  }
  
}

void updateActivities() {
  http.begin(wifi, url);
  int httpResponseCode = http.GET();
  
  if (httpResponseCode>0) {
    String json = http.getString(); 
    JSONVar raw_activities = JSON.parse(json);

    for (int i = 0; i < objectCount; i++) {
      int activity_level = raw_activities[i];
      if (activity_level >= 0 && activity_level <= 100) {
        activities[i] = activity_level;
      }
    }
  }
  http.end();
}

void loop() {
  //alle 10ms den neuen Stand vom Server abfragen
  if (millis() > lastUpdated + 10) {
    updateActivities();
  }

  for (int i = 0; i < objectCount; i++) {
    //ohne Aktivität nix los
    if (activities[i] == 0)
      continue;

    //wie lang ist die Pause beim aktuellen Aktivitätsniveau?
    int b = map(activities[i], 0, 100, loActivityBreakDurations[i], hiActivityBreakDurations[i]);
    
    if (millis() > lastTriggered[i] + b) {
      lastTriggered[i] = millis();
      digitalWrite(pins[i], HIGH);
    }
  }
  delay(50);
  for (int i = 0; i < objectCount; i++) {
    digitalWrite(pins[i], LOW);

  delay(5);  
  }
}
