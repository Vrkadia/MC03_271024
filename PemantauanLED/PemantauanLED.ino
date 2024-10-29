#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ThingSpeak.h>
#include <ESP8266HTTPClient.h>

const char *ssid = "SEI"; // Ganti dengan nama Wi-Fi Anda
const char *password = "kizusei<3"; // Ganti dengan password Wi-Fi Anda
const char *thingspeakApiKey = "VQ9VSV8WDUFT8V18"; // Ganti dengan API Key ThingSpeak Anda
const int thingspeakChannel = 2717686; // Ganti dengan Channel ThingSpeak Anda

int ledPin1 = D1; // Hubungkan ini ke salah satu terminal motor
int ledPin2 = D2; // Hubungkan ini ke terminal lain motor DC;
int ledPin3 = D3; // Hubungkan ini ke terminal lain motor DC;
int ledPin4 = D4; // Hubungkan ini ke terminal lain motor DC;
WiFiClient client;
ESP8266WebServer server(80);

const char *html = R"(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Mengontrol LED</title>
<style>
body {
    font-family: 'Arial', sans-serif;
    background-color: #f0f0f0;
    text-align: center;
    padding: 20px;
}
h1 {
    color: #333;
}
.control-container {
    background-color: #008080;
    box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
    border-radius: 5px;
    display: inline-block;
    padding: 20px;
    max-width: 400px;
    margin: 0 auto;
}
.button {
    font-size: 18px;
    padding: 10px 20px;
    background-color: #4CAF50;
    color: #fff;
    border: none;
    border-radius: 5px;
    cursor: pointer;
    width: 100%;
    margin: 10px 0;
    text-decoration: none;
}
.button:hover {
    background-color: #4a5a49;
}
</style>
</head>
<body>
<h1>Kontrol LED</h1>
<div class="control-container">
    <form action="/led" method="POST">
        <input class="button" type="submit" name="nyala_1" value="Hidupkan Lampu 1">
        <input class="button" type="submit" name="mati_1" value="Matikan Lampu 1">
        <input class="button" type="submit" name="nyala_2" value="Hidupkan Lampu 2">
        <input class="button" type="submit" name="mati_2" value="Matikan Lampu 2">
        <input class="button" type="submit" name="nyala_3" value="Hidupkan Lampu 3">
        <input class="button" type="submit" name="mati_3" value="Matikan Lampu 3">
        <input class="button" type="submit" name="nyala_4" value="Hidupkan Lampu 4">
        <input class="button" type="submit" name="mati_4" value="Matikan Lampu 4">
    </form>
</div>
</body>
</html>

)";

void updateThingSpeak(int field, int status) {
    ThingSpeak.begin(client);
    ThingSpeak.setField(field, status);
    ThingSpeak.writeFields(thingspeakChannel, thingspeakApiKey);
}



void setup() {
    pinMode(ledPin1, OUTPUT);
    pinMode(ledPin2, OUTPUT);
    pinMode(ledPin3, OUTPUT);
    pinMode(ledPin4, OUTPUT);
    digitalWrite(ledPin1, LOW);
    digitalWrite(ledPin2, LOW);
    digitalWrite(ledPin3, LOW);
    digitalWrite(ledPin4, LOW);

    Serial.begin(9600);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    Serial.print("Alamat IP Lokal: ");
    Serial.println(WiFi.localIP());

    server.on("/", HTTP_GET, []() {
        server.send(200, "text/html", html);
    });

    server.on("/led", HTTP_POST, []() {
        if (server.hasArg("nyala_1")) {
            digitalWrite(ledPin1, HIGH);
            updateThingSpeak(1, 1);
        } else if (server.hasArg("mati_1")) {
            digitalWrite(ledPin1, LOW);
            updateThingSpeak(1, 0);
        } else if (server.hasArg("nyala_2")) {
            digitalWrite(ledPin2, HIGH);
            updateThingSpeak(2, 1);
        } else if (server.hasArg("mati_2")) {
            digitalWrite(ledPin2, LOW);
            updateThingSpeak(2, 0);
        } else if (server.hasArg("nyala_3")) {
            digitalWrite(ledPin3, HIGH);
            updateThingSpeak(3, 1);
        } else if (server.hasArg("mati_3")) {
            digitalWrite(ledPin3, LOW);
            updateThingSpeak(3, 0);
        } else if (server.hasArg("nyala_4")) {
            digitalWrite(ledPin4, HIGH);
            updateThingSpeak(4, 1);
        } else if (server.hasArg("mati_4")) {
            digitalWrite(ledPin4, LOW);
            updateThingSpeak(4, 0);
        }
        server.send(200, "text/html", html);
    });

    Serial.begin(9600);
    server.begin();
}


void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
  Serial.println(WiFi.localIP());
}
