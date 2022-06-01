#include <Arduino.h>
#include <DMDESP.h>
#include "RTClib.h"
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include <fonts/ElektronMart6x8.h>
#include <fonts/Mono5x7.h>
#define DISPLAYS_WIDE 1
#define DISPLAYS_HIGH 1
DMDESP Disp(DISPLAYS_WIDE, DISPLAYS_HIGH);
int _day, _month, _year, _hour24, _hour12, _minute, _second, _dtw;
String st;
int hr24;
int set_minutes = 0;
int set_hours = 0;
long date_loop = 0;
long data_times_all = 0;
long counterMin = 0;
const char* ssid = "Altro";
const char* pass = "Roda2810";
unsigned long interval = 50;
unsigned long prevoius = 0;
unsigned long interval_date = 1000;
unsigned long previous_date = 0;
unsigned long interval_slide2 = 50;
unsigned long previous_slide2 = 0;
unsigned long interval_host = 1000;
unsigned long previous_host = 0;
unsigned long interval_alarm = 2000;
unsigned long previous_alarm = 0;
const int relay = D6;
String kondisi = "off";
String strSecondRow;
String slide1;
String slide_1, slide_2, slide_3;
int hr_masuk, mn_masuk, hr_keluar, mn_keluar, set_hr, set_mn, set_month, set_year, set_day;
char chrSecondRow[60];
int i = 32;
int j;
int k = 32;
int l = 0;
int sr = 1;
char *Title[] = {"SELAMAT DATANG DI SMAN 1 METRO"};
String tanggal;
char rows[60];
char nameoftheday[7][12] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jum`at", "Sabtu"};
char month_name[12][12]  = {"Januari", "Febuari", "Maret", "April", "Mei", "Juni", "Juli", "Agustus", "September", "Oktober", "November", "Desember"};
const char* host = "http://data-absensi.motor-smart.com/api/get-slide";
RTC_DS1307 rtc;
//ESP8266WiFiMulti WiFiMulti;
AsyncWebServer server(80);
char hr_24 [3];
String str_hr_24;
char mn [3];
String str_mn;
char sc [3];
String str_sc;
int count = 0;
const uint32_t connectTimeoutMs = 500;
boolean akses = true;
boolean akses1 = true;
boolean jam_pelajaran = false;
int minutes_ak;
int minutes_ak1;

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}
void setup() {
  Serial.begin(115200);
  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }

  if(WiFi.status() == WL_CONNECTED){
    Serial.println("WiFi Connected : " + String(ssid));
    Serial.print("IP : ");
    Serial.println(WiFi.localIP());
    String dataIp = String(WiFi.localIP().toString());
    updateIp(dataIp);
    exportJsonData(getResponse(host));
    getDataAlarm();
  }
  Disp.start();
  Disp.setBrightness(100);
  Disp.setFont(Mono5x7);
  server.on("/ip-address", HTTP_GET, [](AsyncWebServerRequest * request) {
    kondisi = "updates";
    DynamicJsonDocument doc(512);
    doc["SSID"]       = ssid;
    doc["IP_ADDRESS"] = WiFi.localIP().toString();
    doc["GATEWAY"]    = WiFi.gatewayIP().toString();
    doc["SUBNET"]     = WiFi.subnetMask().toString();
    String buf;
    serializeJson(doc, buf);
    setHeader(request, buf, "application/json");
  });
  server.onNotFound(notFound);
  server.begin();
}
void setHeader(AsyncWebServerRequest *request, String function, String type)
{
  AsyncWebServerResponse *response = request->beginResponse(200, type, function);
  response->addHeader("Server", "ESP Async Web Server");
  response->addHeader("Access-Control-Allow-Origin", "*");
  response->addHeader("Access-Control-Max-Age", "600");
  response->addHeader("Access-Control-Allow-Methods", "PUT,POST,GET,OPTIONS");
  response->addHeader("Access-Control-Allow-Headers", "600");
  request->send(response);
}

void loop() {
   Disp.loop();
  if ((WiFi.status() == WL_CONNECTED)) {

  if (kondisi == "updates") {
    Serial.println("Updates Data Running Text");
    String ipData = String(WiFi.localIP().toString());
    updateIp(ipData);
    exportJsonData(getResponse(host));
    getDataAlarm();
    menitToSecond();
  }
  unsigned long currentDataeMillis = millis();
  if (count == 30) {
    if (currentDataeMillis - previous_slide2 >= interval_slide2) {
      previous_slide2 = currentDataeMillis;
      k--;
      tanggal = String(nameoftheday[_dtw]) + ", " + String(_day) + "-" + String(month_name[_month - 1]) + "-" + String(_year);
      tanggal.toCharArray(rows, 60);
      l = strlen(rows) + (strlen(rows) * 5);
      Disp.drawText(k, 0, rows, strlen(rows));

      if (k <= ~l) {
        k = 32;
        count = 0;
      }
    }
  } else {
    if (currentDataeMillis - previous_date >= interval_date) {
      previous_date = currentDataeMillis;
      GetDateTime();
      str_hr_24 = String(_hour24);
      str_hr_24.toCharArray(hr_24, 3);

      if (_hour24 < 10) {
        Disp.drawText(2, 0, "0");
        Disp.drawText(8, 0, hr_24);
      }
      else {
        Disp.drawText(2, 0, hr_24);
      }
      if (_second % 2 == 0) {
        Disp.drawText(14, 0, ":");
      }
      else {
        Disp.drawText(14, 0, " ");
      }
      str_mn = String(_minute);
      str_mn.toCharArray(mn, 3);

      if (_minute < 10) {
        Disp.drawText(19, 0, "0");
        Disp.drawText(26, 0, mn);
      }
      else {
        Disp.drawText(19, 0, mn);
      }
      count++;
    }
  }
  unsigned long currentMillis = millis();
  if (currentMillis - prevoius >= interval) {
    prevoius = currentMillis;
    Scrolling();
  }
  unsigned long dataMillis = millis();
  if(dataMillis - previous_host >= interval_host){
    previous_host = dataMillis;
     menitToSecond();
     if(jam_pelajaran == true){
        counterMin++;
    }
    Serial.println(counterMin);
  }
  if(counterMin == date_loop){
      for(int i = 0; i < 2; i++){
        digitalWrite(relay, HIGH);
        delay(1000);
        digitalWrite(relay, LOW);
        delay(3000);
      }
      counterMin = data_times_all;
    }
    unsigned long currentRelay = millis();
    if (currentRelay - previous_alarm >= interval_alarm) {
      previous_alarm = currentRelay;
      DateTime now = rtc.now();
      setJamMasuk(hr_masuk, mn_masuk);
      setJamPulang(hr_keluar, mn_keluar);
    }
  }
}
void Scrolling() {
  Disp.setFont(Mono5x7);
  switch (sr) {
    case 1:
      strSecondRow = slide_1;
      strSecondRow.toCharArray(chrSecondRow, 60);
      j = strlen(chrSecondRow) + (strlen(chrSecondRow) * 5);
      break;
    case 2:
      strSecondRow = slide_2;
      strSecondRow.toCharArray(chrSecondRow, 60);
      j = strlen(chrSecondRow) + (strlen(chrSecondRow) * 5);
      break;
    case 3:
      strSecondRow = slide_3;
      strSecondRow.toCharArray(chrSecondRow, 60);
      j = strlen(chrSecondRow) + (strlen(chrSecondRow) * 5);
      break;
  }
  i--;
  Disp.drawText(i, 9, chrSecondRow, strlen(chrSecondRow));

  if (i <= ~j) {
    i = 32;
    sr++;
    if (sr > 3) sr = 1;
  }
}
void GetDateTime() {
  DateTime now = rtc.now();
  _day = now.day();
  _month = now.month();
  _year = now.year();
  _hour24 = now.hour();
  _minute = now.minute();
  _second = now.second();
  _dtw = now.dayOfTheWeek();

  hr24 = _hour24;
  if (hr24 > 12) {
    _hour12 = hr24 - 12;
  }
  else if (hr24 == 0) {
    _hour12 = 12;
  }
  else {
    _hour12 = hr24;
  }

  if (hr24 < 12) {
    st = "AM";
  }
  else {
    st = "PM";
  }
}

//jam Masuk
void setJamMasuk(int a, int b) {
  if (a == _hour24 && b == _minute && akses == true) {
    minutes_ak = _minute;
    if(minutes_ak == _minute){
      for(int i = 1; i <= 3; i++){
        digitalWrite(relay, HIGH);
        delay(1000);
        digitalWrite(relay, LOW);
        delay(3000);

        if(i == 3){
          akses = false;
          jam_pelajaran = true;
          counterMin = 0;
        }
      }
    }
  }
  if(minutes_ak != _minute){
      akses = true;
      minutes_ak =  0;
    }
}
//Jam Pulang
void setJamPulang(int a, int b) {
    if (a == _hour24 && b == _minute && akses1== true) {
        minutes_ak1 = _minute;
        if(minutes_ak1 == _minute){
          for(int i = 1; i <= 4; i++){
            digitalWrite(relay, HIGH);
            delay(1000);
            digitalWrite(relay, LOW);
            delay(3000);
    
            if(i == 4){
              counterMin = 1;
              akses1 = false;
              jam_pelajaran = false;
            }
          }
        }
    }
    if(minutes_ak1 != _minute){
          akses1 = true;
          minutes_ak1=  0;
        }
}

 String getResponse(String url) {
  String input;
  WiFiClient client;
  HTTPClient http;

  if(http.begin(client, "http://data-absensi.motor-smart.com/api/get-slide")){
   int httpCode = http.GET();
      if (httpCode > 0) {
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          input = http.getString();
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
  return input;
}
void updateIp(String ip){
  WiFiClient client;
  HTTPClient http;

  if(http.begin(client, "http://data-absensi.motor-smart.com/api/update-ip?ip_data=" + String(ip))){
   int httpCode = http.GET();
      if (httpCode > 0) {
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
}
void getDataAlarm(){
  WiFiClient client;
  HTTPClient http;

  if(http.begin(client, "http://motor-smart.com/authsim/v1/times")){
   int httpCode = http.GET();
      if (httpCode > 0) {
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          exportSetPelajaran(payload);
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
}
void exportSetPelajaran(String a){
  const int size = JSON_OBJECT_SIZE(2) + 2 * JSON_OBJECT_SIZE(4) +
  4 * JSON_OBJECT_SIZE(6) + 6 * JSON_OBJECT_SIZE(8) + 8 * JSON_OBJECT_SIZE(10);
  DynamicJsonDocument doc(size);
  DeserializationError error = deserializeJson(doc, a);
  if (error == DeserializationError::Ok) {
    set_minutes = String(doc["set_time"].as<String>()).toInt();
    set_hours = String(doc["set_hours"].as<String>()).toInt();
  }
}

void exportJsonData(String payload) {
  const int size = JSON_OBJECT_SIZE(2) + 2 * JSON_OBJECT_SIZE(4) +
  4 * JSON_OBJECT_SIZE(6) + 6 * JSON_OBJECT_SIZE(8) + 8 * JSON_OBJECT_SIZE(10);
  DynamicJsonDocument doc(size);
  DeserializationError error = deserializeJson(doc, payload);
  if (error == DeserializationError::Ok) {
    Serial.println(payload);
    slide_1 = String(doc["slide_1"].as<String>());
    slide_2 = String(doc["slide_2"].as<String>());
    slide_3 = String(doc["slide_3"].as<String>());
    set_year = String(doc["set_year"].as<String>()).toInt();
    set_month = String(doc["set_month"].as<String>()).toInt();
    set_hr = String(doc["set_jam"].as<String>()).toInt();
    set_mn = String(doc["set_menit"].as<String>()).toInt();
    set_day = String(doc["set_day"].as<String>()).toInt();
    hr_masuk = String(doc["jam_masuk"].as<String>()).toInt();
    mn_masuk = String(doc["menit_masuk"].as<String>()).toInt();
    hr_keluar = String(doc["jam_keluar"].as<String>()).toInt();
    mn_keluar = String(doc["menit_keluar"].as<String>()).toInt();
    rtc.adjust(DateTime(set_year, set_month, set_day, set_hr, set_mn, 0));
    kondisi = "off";
  }
}
void menitToSecond(){
  if(set_minutes != 0 && set_hours != 0){
    int countM = (set_hours * 60) * 60;
    date_loop = countM;
    data_times_all = countM;
  }
  if(set_hours == 0 && set_minutes != 0){
    int mH = set_minutes * 60;
    date_loop = mH;
    data_times_all = mH;
  }
}

puntenn Atuh
