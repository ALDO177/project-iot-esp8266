#include <DMDESP.h>
#include "RTClib.h"
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
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
long date_loop = 60;
String ssid = "Altro";
String pass = "Roda2810";
unsigned long interval = 50;
unsigned long prevoius = 0;
unsigned long interval_date = 1000;
unsigned long previous_date = 0;
unsigned long interval_slide2 = 50;
unsigned long previous_slide2 = 0;
unsigned long interval_host = 1000;
unsigned long previous_host = 0;
const int relay = 12;
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
char month_name[12][12]  = {"Januai", "Febuari", "Maret", "April", "Mei", "Juni", "Juli", "Agustus", "September", "Oktober", "November", "Desember"};
String host = "https://data-absensi.motor-smart.com/api/get-slide";
RTC_DS1307 rtc;
AsyncWebServer server(80);
char hr_24 [3];
String str_hr_24;
char mn [3];
String str_mn;
char sc [3];
String str_sc;
int count = 0;

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}
void setup() {
  Serial.begin(9600);
  pinMode(relay, OUTPUT);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
  }
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("Connected : " + ssid);
  Serial.print("IP : ");
  Serial.println(WiFi.localIP());
  Disp.start();
  Disp.setBrightness(100);
  Disp.setFont(Mono5x7);
  exportJsonData(getResponse(host));
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

  if (kondisi == "updates") {
    exportJsonData(getResponse(host));
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
      GetDateTime();
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
    date_loop--;
    Serial.println(date_loop);
    if(date_loop == 0){
      for(int i = 0; i< 3; i++){
        digitalWrite(relay, LOW);
        delay(500);
        digitalWrite(relay, HIGH);
        delay(500);
      }
      date_loop = 60;
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

  setJamMasuk(now, hr_masuk, mn_masuk);
  setJamPulang(now, hr_keluar, mn_keluar);

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
void setJamMasuk(DateTime time, int a, int b) {
  time = rtc.now();
  if (a == time.hour() && b == time.minute()) {
    digitalWrite(relay, LOW);
  }
}

void setJamPulang(DateTime time, int a, int b) {
  time = rtc.now();
  if (a == time.hour() && b == time.minute()) {
    digitalWrite(relay, LOW);
  }
}

String getResponse(String url) {
  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  client->setInsecure();
  HTTPClient https;
  String payload;
  https.begin(*client, url);
  int code = https.GET();
  payload = https.getString();
  return payload;
}
void exportJsonData(String payload) {
  const int size = JSON_OBJECT_SIZE(2) + 2 * JSON_OBJECT_SIZE(4) + 4 * JSON_OBJECT_SIZE(6) + 6 * JSON_OBJECT_SIZE(8) + 8 * JSON_OBJECT_SIZE(10);
  DynamicJsonDocument doc(size);
  DeserializationError error = deserializeJson(doc, payload);
  if (error == DeserializationError::Ok) {
    Serial.println("updates-success");
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
    digitalWrite(relay, HIGH);
    kondisi = "off";
  }
}
