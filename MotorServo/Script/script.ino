#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>
#include <ThingSpeak.h>

Servo myservo;

// Define your variables
const int servoClosedPos = 0; // Position for closed state
const int servoOpenPos = 90;   // Position for open state
bool isLocked = true;          // Lock state
bool isUnlocking = false;      // Unlocking state
const int passwordLength = 6;  // Password length
char correctPassword[] = "123456"; // Replace with your correct password
char enteredPassword[passwordLength + 1];
unsigned long unlockStartTime;
unsigned long delayDuration = 5; // Delay duration in seconds

// ThingSpeak information
const char* thingSpeakAddress = "api.thingspeak.com";
unsigned long channelID = YOUR_CHANNEL_ID; // Replace with your channel ID
const char* writeAPIKey = "YOUR_WRITE_API_KEY"; // Replace with your API key

const char* ssid = "YOUR_SSID"; // Replace with your SSID
const char* password = "YOUR_PASSWORD"; // Replace with your password

ESP8266WebServer server(80);

void setup() {
    Serial.begin(115200);
    myservo.attach(D8);
    myservo.write(servoClosedPos);

    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    Serial.print("Local IP Address: ");
    Serial.println(WiFi.localIP());

    // Setup web server routes
    server.on("/", HTTP_GET, handleRoot);
    server.on("/toggle", HTTP_GET, handleToggle);
    server.begin();

    ThingSpeak.begin(client); // Initialize ThingSpeak
}

void loop() {
    server.handleClient();
    static unsigned long lastThingSpeakUpdate = 0;
    unsigned long currentTime = millis();

    // Send data to ThingSpeak every 15 seconds
    if (currentTime - lastThingSpeakUpdate >= 15000) {
        ThingSpeak.writeField(channelID, 1, isLocked ? 0 : 1, writeAPIKey);
        ThingSpeak.writeField(channelID, 2, isUnlocking ? 2 : (isLocked ? 0 : 1), writeAPIKey);
        ThingSpeak.writeField(channelID, 3, delayDuration, writeAPIKey);
        lastThingSpeakUpdate = currentTime;
    }
}

void handleRoot() {
    String statusMessage = isLocked ? "Terkunci" : (isUnlocking ? "Membuka ..." : "Terbuka");
    String errorMessage = "";

    if (server.hasArg("error")) {
        errorMessage = "<p style='color: red;'>Password Salah</p>";
    }

    String html = "<html><head>";
    html += "<style>body { font-family: Arial, sans-serif; background-color: #f2f2f2; text-align: center; margin: 0; padding: 0; }";
    html += ".header { background-color: #333; color: #fff; padding: 20px; }";
    html += ".container { margin-top: 20px; padding: 20px; }";
    html += ".button { display: inline-block; padding: 20px 40px; font-size: 24px; text-decoration: none; margin: 10px; border-radius: 10px; transition: background-color 0.3s ease-in-out; cursor: pointer; }";
    html += ".button:hover { background-color: #555; }";
    html += ".button:active { background-color: #333; }";
    html += ".button-number { background-color: #3498db; color: #ffffff; }";
    html += ".button-clear { background-color: #e74c3c; color: #ffffff; }";
    html += ".button-enter { background-color: #2ecc71; color: #ffffff; }";
    html += ".button-wrapper { display: grid; grid-template-columns: repeat(3, 1fr); gap: 20px; justify-content: center; }";
    html += ".status { font-size: 24px; margin-top: 20px; }";
    html += ".password-input { font-size: 24px; padding: 10px; width: 80px; text-align: center; border: 1px solid #ccc; border-radius: 5px; }</style>";
    html += "<script>function appendDigit(digit) { var passwordField = document.getElementById('passwordField'); if (passwordField.value.length < 6) { passwordField.value += digit; }}";
    html += "function clearPassword() { var passwordField = document.getElementById('passwordField'); passwordField.value = ''; }";
    html += "function toggleDoor() { var passwordField = document.getElementById('passwordField').value; if (passwordField.length === 6) { var xhr = new XMLHttpRequest(); xhr.open('GET', '/toggle?password=' + passwordField, true); xhr.onreadystatechange = function() { if (xhr.readyState === 4) { if (xhr.status === 200) { document.getElementById('status').textContent = 'Status Pintu: ' + xhr.responseText; document.getElementById('error').innerHTML = ''; passwordField.value = ''; } else { document.getElementById('error').innerHTML = 'Password Salah'; }}}; xhr.send(); } else { document.getElementById('error').innerHTML = 'Panjang password tidak valid'; }};</script>";
    html += "</head><body><div class='header'><h1>Kontrol Pintu</h1></div><div class='container'>";
    html += "<p id='status' class='status'>Status Pintu: " + statusMessage + "</p>";
    html += "<div id='error'>" + errorMessage + "</div>";
    html += "<input type='password' id='passwordField' name='password' maxlength='6' readonly class='password-input'>";
    html += "<div class='button-wrapper>";

    for (int i = 1; i <= 9; i++) {
        html += "<button type='button' class='button button-number' onclick='appendDigit(" + String(i) + ")'>" + String(i) + "</button>";
    }

    html += "<button type='button' class='button button-clear' onclick='clearPassword()'>C</button>";
    html += "<button type='button' class='button button-number' onclick='appendDigit(0)'>0</button>";
    html += "<button type='button' class='button button-enter' onclick='toggleDoor()'>E</button>";
    html += "</div></div></body></html>";

    server.send(200, "text/html", html);
}

void handleToggle() {
    String requestPassword = server.arg("password");
    if (requestPassword.length() == passwordLength) {
        requestPassword.toCharArray(enteredPassword, passwordLength + 1);
        if (strcmp(enteredPassword, correctPassword) == 0) {
            if (isLocked) {
                myservo.write(servoOpenPos);
                isLocked = false;
                isUnlocking = true; // Mark as unlocking
                unlockStartTime = millis(); // Record unlock start time
                ThingSpeak.writeField(channelID, 2, 1, writeAPIKey); // Send status: door opened
            } else {
                myservo.write(servoClosedPos);
                isLocked = true;
                isUnlocking = false; // Reset unlocking status
                ThingSpeak.writeField(channelID, 2, 0, writeAPIKey); // Send status: door locked
            }
            server.send(200, "text/plain", (isLocked ? "Terkunci" : "Terbuka"));
        } else {
            server.send(200, "text/plain", "Password Salah");
        }
    } else {
        server.send(400, "text/plain", "Panjang password tidak valid");
    }

    // Check if the door has finished unlocking after the delay
    if (isUnlocking && millis() - unlockStartTime >= (delayDuration * 1000)) {
        myservo.write(servoClosedPos); // Lock the door after the duration
        isLocked = true;
        isUnlocking = false; // Reset unlocking status
        ThingSpeak.writeField(channelID, 2, 0, writeAPIKey); // Send status: door locked
    }
}