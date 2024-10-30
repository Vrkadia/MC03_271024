#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>
#include <ThingSpeak.h>

// Informasi ThingSpeak
char thingSpeakAddress[] = "api.thingspeak.com";
unsigned long channelID = 2721931;
const char* writeAPIKey = "P7MN3SIWBMP1ZLMU";
const char* ssid = "SEI";
const char* password = "kizusei<3";

// Variables
bool isLocked = true;
bool isUnlocking = false;
unsigned long unlockStartTime;
int servoClosedPos = 0;
int servoOpenPos = 90;
int delayDuration = 5; // Duration in seconds
Servo myservo;

// Deklarasi server dan client untuk ThingSpeak
ESP8266WebServer Server(80);
WiFiClient client;

void setup() {
    Serial.begin(9600);
    myservo.attach(D1);
    myservo.write(servoClosedPos);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    Serial.print("Local IP Address: ");
    Serial.println(WiFi.localIP());

    Server.on("/", HTTP_GET, handleRoot);
    Server.on("/toggle", HTTP_GET, handleToggle);
    Server.begin();
    ThingSpeak.begin(client);
}

void loop() {
    Server.handleClient();
    static unsigned long lastThingSpeakUpdate = 0;
    unsigned long currentTime = millis();

    if (currentTime - lastThingSpeakUpdate >= 15000) {
        ThingSpeak.writeField(channelID, 1, isLocked ? 0 : 1, writeAPIKey);
        ThingSpeak.writeField(channelID, 2, isUnlocking ? 2 : (isLocked ? 0 : 1), writeAPIKey);
        ThingSpeak.writeField(channelID, 3, delayDuration, writeAPIKey);
        lastThingSpeakUpdate = currentTime;
    }

    if (isUnlocking && millis() - unlockStartTime >= (delayDuration * 1000)) {
        myservo.write(servoClosedPos);
        isLocked = true;
        isUnlocking = false;
        ThingSpeak.writeField(channelID, 2, 0, writeAPIKey);
    }
}

void handleRoot() {
    String statusMessage = isLocked ? "Terkunci" : (isUnlocking ? "Membuka..." : "Terbuka");
    String errorMessage = "";

    if (Server.hasArg("error")) {
        errorMessage = "<p style='color: red;'>Password Salah</p>";
    }

    String html = "<html><head><style>"
                  "body { font-family: Arial, sans-serif; background-color: #f2f2f2; text-align: center; }"
                  ".header { background-color: #333; color: #fff; padding: 20px; }"
                  ".container { margin-top: 20px; padding: 20px; }"
                  ".button { display: inline-block; padding: 20px 40px; font-size: 24px; margin: 10px; border-radius: 10px; transition: background-color 0.3s; cursor: pointer; }"
                  ".button:hover { background-color: #555; }"
                  ".button-number { background-color: #3498db; color: #ffffff; }"
                  ".button-clear { background-color: #e74c3c; color: #ffffff; }"
                  ".button-enter { background-color: #2ecc71; color: #ffffff; }"
                  ".button-wrapper { display: grid; grid-template-columns: repeat(3, 1fr); gap: 20px; }"
                  ".status { font-size: 24px; margin-top: 20px; }"
                  ".password-input { font-size: 24px; padding: 10px; width: 80px; text-align: center; border: 1px solid #ccc; border-radius: 5px; }"
                  "</style><script>"
                  "function appendDigit(digit) { var passwordField = document.getElementById('passwordField'); if (passwordField.value.length < 6) { passwordField.value += digit; } }"
                  "function clearPassword() { document.getElementById('passwordField').value = ''; }"
                  "function toggleDoor() {"
                  " var passwordField = document.getElementById('passwordField').value;"
                  " if (passwordField.length === 6) {"
                  "  var xhr = new XMLHttpRequest();"
                  "  xhr.open('GET', '/toggle?password=' + passwordField, true);"
                  "  xhr.onreadystatechange = function() {"
                  "   if (xhr.readyState === 4) {"
                  "    if (xhr.status === 200) { document.getElementById('status').textContent = 'Status Pintu: ' + xhr.responseText; }"
                  "    else { document.getElementById('error').innerHTML = 'Password Salah'; }"
                  "   }"
                  "  };"
                  "  xhr.send();"
                  " } else { document.getElementById('error').innerHTML = 'Panjang password tidak valid'; }"
                  "}"
                  "</script></head><body><div class='header'><h1>Kontrol Pintu</h1></div><div class='container'>"
                  "<p id='status' class='status'>Status Pintu: " + statusMessage + "</p>"
                  "<div id='error'>" + errorMessage + "</div>"
                  "<input type='password' id='passwordField' maxlength='6' readonly class='password-input'>"
                  "<div class='button-wrapper'>";

    for (int i = 1; i <= 9; i++) {
        html += "<button type='button' class='button button-number' onclick='appendDigit(" + String(i) + ")'>" + String(i) + "</button>";
    }

    html += "<button type='button' class='button button-clear' onclick='clearPassword()'>C</button>"
            "<button type='button' class='button button-number' onclick='appendDigit(0)'>0</button>"
            "<button type='button' class='button button-enter' onclick='toggleDoor()'>E</button></div></div></body></html>";

    Server.send(200, "text/html", html);
}

void handleToggle() {
    String requestPassword = Server.arg("password");

    if (requestPassword.length() == 6) {
        if (requestPassword == "123456") {
            if (isLocked) {
                myservo.write(servoOpenPos);
                isLocked = false;
                isUnlocking = true;
                unlockStartTime = millis();
                ThingSpeak.writeField(channelID, 2, 1, writeAPIKey);
            } else {
                myservo.write(servoClosedPos);
                isLocked = true;
                isUnlocking = false;
                ThingSpeak.writeField(channelID, 2, 0, writeAPIKey);
            }

            Server.send(200, "text/plain", isLocked ? "Terkunci" : "Terbuka");
        } else {
            Server.send(200, "text/plain", "Password Salah");
        }
    } else {
        Server.send(400, "text/plain", "Panjang password tidak valid");
    }
}
