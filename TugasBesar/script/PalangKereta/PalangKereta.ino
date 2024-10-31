#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ThingSpeak.h>
#include <ESP8266HTTPClient.h>

const char *ssid = "SEI"; // Replace with your Wi-Fi name
const char *password = "kizusei<3"; // Replace with your Wi-Fi password
const char *thingspeakApiKey = "QJ4XKVJFJNOYCPBQ"; // Replace with your ThingSpeak API Key
const int thingspeakChannel = 2721758; // Replace with your ThingSpeak Channel

int ledPin1 = D0;
int motorPin1 = D2; 
int motorPin2 = D3;

WiFiClient client;
ESP8266WebServer server(80);

const char *html = R"(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Kontrol Palang</title>
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
<h1>Kontrol Palang</h1>
<div class="control-container">
    <form action="/led" method="POST">
        <input class="button" type="submit" name="palang_tutup" value="Palang Tutup">
        <input class="button" type="submit" name="palang_buka" value="Palang Buka">
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
    pinMode(motorPin1, OUTPUT);
    pinMode(motorPin2, OUTPUT);
    digitalWrite(ledPin1, LOW);
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);

    Serial.begin(9600);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    Serial.print("Local IP Address: ");
    Serial.println(WiFi.localIP());

    server.on("/", HTTP_GET, []() {
        server.send(200, "text/html", html);
    });

    server.on("/led", HTTP_POST, []() {
        if (server.hasArg("palang_tutup")) {
            server.send(200, "text/html", html);
            digitalWrite(ledPin1, HIGH);
            // Rotate motor counterclockwise
            digitalWrite(motorPin1, HIGH);
            digitalWrite(motorPin2, LOW);
            delay(2000);
            // Stop motor
            digitalWrite(motorPin1, LOW);
            digitalWrite(motorPin2, LOW);
            updateThingSpeak(1, 1);
        } else if (server.hasArg("palang_buka")) {
            server.send(200, "text/html", html);
            digitalWrite(ledPin1, LOW);
            // Rotate motor clockwise
            digitalWrite(motorPin1, LOW);
            digitalWrite(motorPin2, HIGH);
            delay(2000);
            // Stop motor
            digitalWrite(motorPin1, LOW);
            digitalWrite(motorPin2, LOW);
            updateThingSpeak(1, 0);
        } else {
            server.send(200, "text/html", html);
        }
    });

    server.begin();

    Serial.println(WiFi.localIP());
}

void loop() {
    server.handleClient();
}
