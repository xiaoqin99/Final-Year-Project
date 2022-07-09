#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <FirebaseESP8266.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

#define WIFI_SSID "AndroidAPE006"
#define WIFI_PASSWORD "1223334444"

#define API_KEY "AIzaSyBNiEt7OEJgUYb7tHEix-C6Awu-ObY-1xY"
#define DATABASE_URL "https://smart-traffic-light-syst-69598-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define USER_EMAIL "sherryqin1999@gmail.com"
#define USER_PASSWORD "zhengwingchun"

int IRPin = 12;
int GREENlight = 16;
int YELLOWlight = 5;
int REDlight = 4;
int IRDETECTED = 0;

unsigned long previousRedMillis = 0;
unsigned long previousYellowMillis = 0;
unsigned long previousGreenMillis = 0;
unsigned long previousDetectMillis = 0;

const long redInterval = 5000;
const long yellowInterval = 2000;
const long greenInterval = 5000;
const long detectInterval = 10000;

const long utcOffsetInSeconds = 28800;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

int Mode = 1;
int setlight = 0;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void setup() {
  Serial.begin(115200);
  pinMode(IRPin, INPUT);
  pinMode(GREENlight, OUTPUT);
  pinMode(YELLOWlight, OUTPUT);
  pinMode(REDlight, OUTPUT);
  WiFiSetup();
  FBSetup();
  delay(1000);
  timeClient.begin();
  FireBaseIntUpdate("/FYP/Mode", 1);
  FireBaseIntUpdate("/FYP/setlight", 0);
}

void loop() {
  timeClient.update();

  Mode = FireBaseGet("/FYP/Mode");
  if (Mode == 0) {
    setlight = FireBaseGet("/FYP/setlight");
    Serial.println("Manual Mode");
    Serial.println(" ");
    if (setlight == 0) {
      digitalWrite(GREENlight, LOW);
      digitalWrite(YELLOWlight, LOW);
      digitalWrite(REDlight, LOW);
    }
    else if (setlight == 1) {
      greenLight();
    }
    else if (setlight == 2) {
      yellowLight();
    }
    else {
      redLight();
    }
    delay(1000);
  }
  else {
    Serial.println("Auto Mode");
    if (IRdetect()) {
      Serial.println("Something detected!!!");
      Serial.print(daysOfTheWeek[timeClient.getDay()]);
      Serial.print(", ");
      Serial.print(timeClient.getHours());
      Serial.print(":");
      Serial.print(timeClient.getMinutes());
      Serial.print(":");
      Serial.println(timeClient.getSeconds());
      Serial.println(" ");
      delay(1000);
      greenLight();
      previousYellowMillis = millis(); //reset
      IRDETECTED = 1;
    }
    else {
      Serial.println("No detection!!!");
      Serial.println(" ");
      if (IRDETECTED) {
        greenLight();
        delay(detectInterval);
        IRDETECTED = 0;
      }
      else {
        if (millis() - previousYellowMillis < yellowInterval) {
          yellowLight();
          previousRedMillis = millis();
        }
        else {
          if (millis() - previousRedMillis < redInterval) {
            redLight();
            previousGreenMillis = millis();
          }
          else {
            if (millis() - previousGreenMillis < greenInterval) {
              greenLight();
            }
            else {
              previousYellowMillis = millis();
            }
          }
        }
      }
    }
  }
}



bool IRdetect() {
  int irDetect;
  irDetect = digitalRead(IRPin);
  if (!irDetect) {
    return true;
  }
  else {
    return false;
  }
}

void redLight() {
  digitalWrite(GREENlight, LOW);
  digitalWrite(YELLOWlight, LOW);
  digitalWrite(REDlight, HIGH);
}

void yellowLight() {
  digitalWrite(GREENlight, LOW);
  digitalWrite(YELLOWlight, HIGH);
  digitalWrite(REDlight, LOW);
}

void greenLight() {
  digitalWrite(GREENlight, HIGH);
  digitalWrite(YELLOWlight, LOW);
  digitalWrite(REDlight, LOW);
}

void WiFiSetup() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void FBSetup() {
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Firebase.setDoubleDigits(5);
}

void FireBaseIntUpdate(String var, int intsta)
{
  if (Firebase.ready())
  {

    if (Firebase.setInt(fbdo, var, intsta))
    {
//      Serial.println("settings -- FB Int Updated");

    }
    else
    {
      Serial.println("FB Int Failed Update");
    }
  }
}

void FireBaseStringUpdate(String var, String stringdata)
{
  if (Firebase.ready())
  {

    if (Firebase.setString(fbdo, var, stringdata))
    {
//      Serial.println("FB String Updated");
    }
    else
    {
      Serial.println("FB String Failed Update");
    }
  }
}

int FireBaseGet(String chk) {
  if (Firebase.getInt(fbdo, chk))
  {
//    Serial.println("get Successful...");
    return fbdo.intData();
  }
  else
  {
    Serial.println("CO get Failed...");
    return 2;
  }
}
