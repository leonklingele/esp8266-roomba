#include <SPI.h>
#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "Tell-Your-WiFi-Said-Hi";
const char* pass = "secretpass";
const char* hostname = "broomi";

const char* basicAuthUser = "admin";
const char* basicAuthPass = "admin";

const int pin = 2;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

ESP8266WebServer server(80);

void connectToWifi() {
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.hostname(hostname);

  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("WiFi connected");

  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Signal strength (RSSI): ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");

  Serial.println("Starting server");
  server.begin();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("WiFi disconnected");

  Serial.println("Stopping server");
  server.stop();

  wifiReconnectTimer.once(2, connectToWifi);
}

void setup() {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);

  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  Serial.println("Serial inizialized");

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);
  }
  Serial.println("WiFi shield present");

  server.on("/", []() {
    if (!server.authenticate(basicAuthUser, basicAuthPass)) {
      return server.requestAuthentication();
    }

    if (server.method() == HTTP_POST) {
      digitalWrite(pin, HIGH);
      delay(200);
      digitalWrite(pin, LOW);
    }

    server.send(200, "text/html", "<!DOCTYPE html><html><head><title>Broomi control</title><style>html, body, #container { margin: 0; width: 100%; height: 100%; } #buttons { height: 100%; display: flex; justify-content: space-evenly; align-items: center; } button { font-size: 24vw; background-color: transparent; border: 0; border-radius: 12px; padding: 0; }</style></head><body><div id=\"container\"><div id=\"buttons\"><button type=\"button\" id=\"startstop\">&#x1f916;</button></div></div><script>(function () { document.getElementById('startstop').onclick = function () { fetch('/', {  method: 'POST' }); }; })();</script></body>\n");
  });

  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
  connectToWifi();
}

void loop() {
  server.handleClient();
}
