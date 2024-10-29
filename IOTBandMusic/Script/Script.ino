#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ThingSpeak.h>
#include <Wire.h>

const char* ssid = "SEI";
const char* password = "kizusei<3";
const char* thingSpeakApiKey = "QPBNRDJY0TIBSJEQ";
const int thingSpeakChannel = 2719793;
const int buzzerPin = D1;
String passcode = "1234"; // Set your desired passcode

ESP8266WebServer server(80);
WiFiClient client;

unsigned long lastUpdateTime = 0; // Time of the last data sent
const unsigned long updateInterval = 5000; // Data sending interval in milliseconds (5 seconds)

void setup() {
    pinMode(buzzerPin, OUTPUT);
    digitalWrite(buzzerPin, LOW);
    
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
        String html = "<!DOCTYPE html><html><head><title>Masukan Kode Musik</title><style>";
        html += "body {background: linear-gradient(to bottom, pink, white); display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0;}";
        html += "form { display: block; margin-top: 20px;}";
        html += "input { margin: 5px; width: 100px; height: 50px; background-color: pink; color: white; border: none; border-radius: 10px;}";
        html += "button { width: 50px; height: 50px; background-color: pink; color: white; border: none; border-radius: 10px; cursor: pointer; transition: background-color 0.3s, transform 0.3s;}";
        html += ".red-button:hover { background-color: darkred;}";
        html += ".green-button:hover { background-color: darkgreen;}";
        html += "</style></head><body>";
        html += "<form action='/play' method='post'>";
        html += "<label for='passcode'>Masukan Kode Musik:</label>";
        html += "<input type='password' id='passcode' name='passcode' maxlength='4' pattern='\\d{4}' required>";
        html += "<br><button type='submit' class='green-button'>Oke</button>";
        html += "</form></body></html>";
        server.send(200, "text/html", html);
    });

    server.on("/play", HTTP_POST, []() {
        if (server.hasArg("passcode")) {
            String enteredPasscode = server.arg("passcode");
            if (enteredPasscode == passcode) {
                playNotes();
                server.send(200, "text/plain", "OK");
            } else {
                server.send(200, "text/plain", "Invalid Passcode");
            }
        }
    });

    server.begin();
    ThingSpeak.begin(client);
}

void loop() {
    server.handleClient();
    if (millis() - lastUpdateTime >= updateInterval) {
        lastUpdateTime = millis();
        updateThingSpeak(1, 1);
    }
}

void playNotes() {
    int notes[] = {261, 294, 329, 349, 392, 440, 493, 512};
    for (int note : notes) {
        playNote(note);
    }
}

void playNote(int frequency) {
    tone(buzzerPin, frequency, 500); // Play tone for 500ms
    delay(600); // Delay between notes
    noTone(buzzerPin); // Stop tone
}

void playNoteByName(String note) {
    if (note == "C") playNote(261);
    else if (note == "D") playNote(294);
    else if (note == "E") playNote(329);
    else if (note == "F") playNote(349);
    else if (note == "G") playNote(392);
    else if (note == "A") playNote(440);
    else if (note == "B") playNote(493);
    else if (note == "C2") playNote(512);
}

void updateThingSpeak(int field, int status) {
    ThingSpeak.setField(field, status);
    ThingSpeak.writeFields(thingSpeakChannel, thingSpeakApiKey);
}

void addPasscode(String value) {
    if (passcode.length() < 4) {
        passcode += value;
    }
}

void deleteLastChar() {
    if (passcode.length() > 0) {
        passcode = passcode.substring(0, passcode.length() - 1);
    }
}
