#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ThingSpeak.h>

const char *ssid = "SEI"; // Change to your WiFi name
const char *password = "kizusei<3"; // Change to your WiFi password

int motorPin1 = D1; // Connect this to one of the motor terminals
int motorPin2 = D2; // Connect this to the other motor terminal
ESP8266WebServer server(80); // Start server on port 80

bool tiraiTerbuka = false; // Initial status of the curtain

const char *html = R"(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Mengontrol Tirai Jarak Jauh</title>
  <style>
    body {
      font-family: 'Arial', sans-serif;
      background: linear-gradient(to bottom, #E3E912, #1BFFF);
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      margin: 0;
    }
    .control-container {
      background-color: rgba(255, 255, 255, 0.8);
      box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
      border-radius: 5px;
      display: inline-block;
      padding: 20px;
      max-width: 400px;
      text-align: center;
    }
    .button {
      font-size: 18px;
      padding: 10px 20px;
      background-color: #2196F3;
      color: #fff;
      border: none;
      border-radius: 5px;
      cursor: pointer;
      width: 100%;
      margin: 10px 0;
      text-decoration: none;
      transition: background-color 0.3s, transform 0.1s;
    }
    .button:hover {
      background-color: #1976D2;
      transform: scale(1.05);
    }
    h1 {
      color: #333;
      font-size: 24px;
      margin-bottom: 20px;
    }
  </style>
</head>
<body>
  <div class="control-container">
    <h1>Kendalikan Tirai Jarak Jauh</h1>
    <form action="/tirai" method="POST">
      <input class="button" type="submit" name="buka" value="Buka Tirai">
      <input class="button" type="submit" name="tutup" value="Tutup Tirai">
    </form>
  </div>
</body>
</html>
)";

// Code to connect to ThingSpeak
const char *thingSpeakApiKey = "QJ4XKVJFJNOYCPBQ"; // Change to your API key
const int thingSpeakChannel = 2721758; // Change to your channel ID
WiFiClient client;

void updateThingSpeak(int status) {
  ThingSpeak.begin(client);
  ThingSpeak.setField(1, status); // Field 1 for status open/close
  ThingSpeak.writeFields(thingSpeakChannel, thingSpeakApiKey);
}

// Code to operate motor and connect to WiFi
void setup() {
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
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
  server.on("/tirai", HTTP_POST, []() {
    if (server.hasArg("buka") && !tiraiTerbuka) {
      digitalWrite(motorPin1, LOW);
      digitalWrite(motorPin2, HIGH);
      delay(5000); // Adjust delay as needed
      digitalWrite(motorPin2, LOW);
      updateThingSpeak(1); // Send open status to ThingSpeak
      tiraiTerbuka = true;
    } else if (server.hasArg("tutup") && tiraiTerbuka) {
      digitalWrite(motorPin1, HIGH);
      digitalWrite(motorPin2, LOW);
      delay(5000); // Adjust delay as needed
      digitalWrite(motorPin2, LOW);
      updateThingSpeak(0); // Send close status to ThingSpeak
      tiraiTerbuka = false;
    }
    server.send(200, "text/html", html);
  });
  server.begin();
  
}
void loop() {
  server.handleClient();
  Serial.println(WiFi.localIP());
}
