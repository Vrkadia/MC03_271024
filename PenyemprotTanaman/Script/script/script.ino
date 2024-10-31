#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

int Pin1 = D1;
int Pin2 = D2;
int Pin3 = D5;
int Pin4 = D6;
int pole1[] = {0, 0, 0, 0, 0, 1, 1, 1, 0};
int pole2[] = {0, 0, 0, 1, 1, 1, 0, 0, 0};
int pole3[] = {0, 1, 1, 1, 0, 0, 0, 0, 0};
int pole4[] = {1, 1, 0, 0, 0, 0, 0, 1, 0};
int poleStep = 0;
int dirStatus = 0; // 0 = Motor Off, 1 = CW, -1 = CCW
const char *ssid = "SEI";
const char *password = "kizusei<3";
const char *thingSpeakApiKey = "QPBNRDJY0TIBSJEQ"; // Change to your ThingSpeak API Key
const char *thingSpeakField1 = "field1"; // For status
const char *thingSpeakField2 = "field2"; // For speed

ESP8266WebServer server(80);

void motorControl();
void sendDataToThingSpeak(int status, int speed);
void driveStepper(int c);

void setup() {
    Serial.begin(9600);
    pinMode(Pin1, OUTPUT);
    pinMode(Pin2, OUTPUT);
    pinMode(Pin3, OUTPUT);
    pinMode(Pin4, OUTPUT);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi ... ");
    }

    Serial.println("Connected to WiFi");
    Serial.print("Local IP Address: ");
    Serial.println(WiFi.localIP());

    server.on("/", HTTP_GET, []() {
        String HTML = "<!DOCTYPE html>\
        <html>\
        <head>\
        <title>ESP8266 Stepper Motor Control</title>\
        <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
        <style>\
        html, body {\
            width: 100%;\
            height: 100%;\
            margin: 0;\
            display: flex;\
            justify-content: center;\
            align-items: center;\
            flex-direction: column;\
        }\
        .container {\
            text-align: center;\
        }\
        .btn {\
            margin: 5px;\
            border: none;\
            display: inline-block;\
        }\
        .angleButton {\
            font-size: 30px;\
            border: 1px solid #ccc;\
            padding: 7px 10px;\
            text-decoration: none;\
            cursor: pointer;\
        }\
        h1 {\
            background-color: #90ee90;\
            padding: 10px;\
        }\
        h2 {\
            background-color: #FFFF00;\
            padding: 10px;\
        }\
        .cw-btn {\
            background-color: #90ee90;\
        }\
        .ccw-btn {\
            background-color: #90ee90;\
        }\
        .stop-btn {\
            background-color: #156464;\
        }\
        .angleButton:hover {\
            background-color: #add8e6;\
        }\
        </style>\
        </head>\
        <body>\
        <h1>ESP8266 Stepper Motor Control</h1>\
        <div class=\"container\">";

        if (dirStatus == 1) {
            HTML += "<h2><span style=\"background-color: #FFFF00\">Motor Running in CW</span></h2>";
        } else if (dirStatus == -1) {
            HTML += "<h2><span style=\"background-color: #FFFF00\">Motor Running in CCW</span></h2>";
        } else {
            HTML += "<h2><span style=\"background-color: #FFFF00\">Motor OFF</span></h2>";
        }

        HTML += "<div class=\"btn\"><a class=\"angleButton cw-btn\" href=\"/motor?cw=on\">CW</a></div>";
        HTML += "<div class=\"btn\"><a class=\"angleButton ccw-btn\" href=\"/motor?ccw=on\">CCW</a></div>";
        HTML += "<div class=\"btn\"><a class=\"angleButton stop-btn\" href=\"/motor?stop=on\">Stop</a></div>";
        HTML += "</div></body></html>";
        server.send(200, "text/html", HTML);
    });

    server.on("/motor", HTTP_GET, motorControl);
    server.begin();
}

void loop() {
    server.handleClient();
    if (dirStatus == 1) {
        poleStep++;
        driveStepper(poleStep);
    } else if (dirStatus == -1) {
        poleStep--;
        driveStepper(poleStep);
    } else {
        driveStepper(8);
    }
    
    if (poleStep > 7) {
        poleStep = 0;
    }
    if (poleStep < 0) {
        poleStep = 7;
    }
    delay(1);
}

void motorControl() {
    if (server.arg("cw") == "on") {
        dirStatus = 1; // CW
        sendDataToThingSpeak(dirStatus, 250); // Speed for CW
    } else if (server.arg("ccw") == "on") {
        dirStatus = -1; // CCW
        sendDataToThingSpeak(dirStatus, 250); // Speed for CCW
    } else if (server.arg("stop") == "on") {
        dirStatus = 0; // Stop
        sendDataToThingSpeak(dirStatus, 0); // Speed while stopped
        server.sendHeader("Location", "/", true);
        server.send(303);
    }
}

void driveStepper(int c) {
    digitalWrite(Pin1, pole1[c]);
    digitalWrite(Pin2, pole2[c]);
    digitalWrite(Pin3, pole3[c]);
    digitalWrite(Pin4, pole4[c]);
}

void sendDataToThingSpeak(int status, int speed) {
    // Create HTTP client object
    HTTPClient http;

    // Create URL to send data to ThingSpeak
    String url = "http://api.thingspeak.com/update?api_key=";
    url += thingSpeakApiKey;
    url += "&";
    url += thingSpeakField1;
    url += "=";
    url += String(status);
    url += "&";
    url += thingSpeakField2;
    url += "=";
    url += String(speed);

    // Create WiFiClient object
    WiFiClient client;

    // Send HTTP GET request to ThingSpeak
    http.begin(client, url);
    int httpCode = http.GET();
    delay(10);
    
    // If sending was successful
    if (httpCode == HTTP_CODE_OK) {
        Serial.println("Data sent to ThingSpeak successfully.");
    } else {
        Serial.println("Failed to send data to ThingSpeak.");
    }
    
    http.end();
}
