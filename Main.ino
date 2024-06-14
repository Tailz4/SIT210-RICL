#include <WiFi.h>

const char* ssid = "";
const char* password = "";
const char* ifttt_key = "";
const char* ifttt_event = "";

int trigpin = 3;
int echopin = 2;
int ledpin = 4; // Assuming LED is connected to pin 4
float depth = 26.0;  // Depth of the tank in cm

bool ledOn = false; // Variable to track the LED state

void setup() {
  pinMode(trigpin, OUTPUT);
  pinMode(echopin, INPUT);
  pinMode(ledpin, OUTPUT);
  Serial.begin(9600);

  // Connecting to WiFi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
}

void triggerIFTTT() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    const char* server = "maker.ifttt.com";
    String url = "/trigger/" + String(ifttt_event) + "/with/key/" + String(ifttt_key);

    Serial.print("Connecting to ");
    Serial.println(server);

    if (client.connect(server, 80)) {
      Serial.print("Requesting URL: ");
      Serial.println(url);

      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: " + server + "\r\n" + 
                   "Connection: close\r\n\r\n");

      while (client.connected() && !client.available()) {
        delay(10);
      }

      while (client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line);
      }

      client.stop();
      Serial.println("IFTTT request sent");
    } else {
      Serial.println("Connection to IFTTT failed");
    }
  } else {
    Serial.println("WiFi not connected");
  }
}

void loop() {
  // Trigger the ultrasonic pulse
  digitalWrite(trigpin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigpin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigpin, LOW);

  // Measure the echo pulse
  long duration = pulseIn(echopin, HIGH);
  
  // Calculate the distance to the water surface
  float d = (duration / 2.0) * 0.0343;  // Speed of sound in air is 343 m/s

  // Calculate the water level as a percentage
  float level = (1 - d / depth) * 100.0;
  
  Serial.print("The distance to the water surface is: ");
  Serial.print(d);
  Serial.println(" cm");
  Serial.print("Water level: ");
  Serial.print(level);
  Serial.println("%"); 

  // Check if the water level is at or above 90%
  if (level >= 90.0) {
    if (!ledOn) {
      digitalWrite(ledpin, HIGH);
      ledOn = true;
      triggerIFTTT();
    }
  } else {
    if (ledOn) {
      digitalWrite(ledpin, LOW);
      ledOn = false;
    }
  }

  delay(2000);  // Delay before the next measurement
}
