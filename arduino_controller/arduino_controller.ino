/*
* The objective of this project is to employ an Arduino microcontroller 
* that can wirelessly control WS2812B LED strips over Wi-Fi, this 
* functionality is combined with a physical lightsaber hilt to provide 
* an aesthetically pleasing decoration for any room.
* Copyright (C) 2023  Benjamin Gorenc
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or any 
* later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program.  
* If not, see https://www.gnu.org/licenses/gpl-3.0.html.
*/


#include <SPI.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"
#include <FastLED.h>

/* **********************SETTINGS********************** */
/* Settings for Arduino */
#define DATA_PIN 2                               // First LED Pin
#define DATA_PIN_CLONE 3                         // Second LED Pin
#define setArduinoIP IPAddress(192, 168, 1, 150) // Set the IP address of the Arduino

/* Settings for LED Strip */
#define NUM_LEDS 144                             // Number of LEDS per Strip
#define CHIP_SET WS2812B                         // LED Chip
#define COLOR_ORDER GRB                          // Color Settings for LED Chip
#define BRIGHTNESS 200                           // LED Brightness
#define MAX_VOLTS 5
#define MAX_AMPS 4500
#define BAUD 57600
/* **********************SETTINGS********************** */

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASSWORD;
int keyIndex = 0;
CRGB leds[NUM_LEDS];
CRGB ledsClone[NUM_LEDS];  // Second LED Strip

int status = WL_IDLE_STATUS;
WiFiServer server(80);
String readString;

void setup() {
  FastLED.addLeds<CHIP_SET, DATA_PIN, COLOR_ORDER> (leds,NUM_LEDS);
  FastLED.addLeds<CHIP_SET, DATA_PIN_CLONE, COLOR_ORDER> (ledsClone,NUM_LEDS); // Second LED Strip
	FastLED.setBrightness(BRIGHTNESS);
  FastLED.setMaxPowerInVoltsAndMilliamps(MAX_VOLTS, MAX_AMPS);
  Serial.begin(BAUD);

  WiFi.config(setArduinoIP);
  connectToWiFi();
  server.begin();

  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi connection lost. Reconnecting...");
    WiFi.disconnect();
    connectToWiFi(); // Reconnect to Wi-Fi if connection is lost
    Serial.println("Wi-Fi reconnected.");
  }

  WiFiClient client = server.available();
  if (client) {
    Serial.println("new client");

    while (client.connected()) {
      if (client.available()) {
        char clientData = client.read();
        if (readString.length() < 100) {
          readString += clientData;
          Serial.write(clientData);

          if (clientData == '\n') {

            displayWebPage(client);
            parseClientData(readString);

            client.stop();
            Serial.println("client disconnected");
          }
        }
      }
    }
  }
}

void connectToWiFi() {
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(10000);
  }
}

void turnOffAll() {

  for(int i = (NUM_LEDS - 1); i >= 0; i--) {
    leds[i] = CRGB::Black; // turn off the i'th LED
    ledsClone[i] = CRGB::Black; // Second LED Strip
    FastLED.show();
  }
  delay(1);

}

void lightUpColor(CRGB color) {

  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = color;
    ledsClone[i] = color; // Second LED Strip
    FastLED.show();
  }
  delay(1);

}

void displayWebPage(WiFiClient& client) {

  client.println("<html>");
  client.println("<head>");
  client.println("<title>Sith Lightsaber</title>");
  client.println("<style>");
  client.println("body { background-color: #f2f2f2; font-family: Arial, sans-serif; }");
  client.println("h1 { color: #333333; text-align: center; }");
  client.println("form { text-align: center; }");
  client.println("select { padding: 10px; font-size: 18px; }");
  client.println("input[type='submit'] { padding: 10px 20px; font-size: 18px; background-color: #4CAF50; color: white; border: none; cursor: pointer; }");
  client.println("</style>");
  client.println("</head>");
  client.println("<body>");
  client.println("<h1>Sith Lightsaber Control</h1>");
  client.println("<form method=\"get\" action=\"\">");
  client.println("<select name=\"ledState\">");
  client.println("<option value=\"AllOff\">Turn Off</option>");
  client.println("<option value=\"Blue\">Blue</option>");
  client.println("<option value=\"Green\">Green</option>");
  client.println("<option value=\"Pink\">Pink</option>");
  client.println("<option value=\"Purple\">Purple</option>");
  client.println("<option value=\"Red\">Red</option>");
  client.println("<option value=\"Yellow\">Yellow</option>");
  client.println("</select>");
  client.println("<br><br>");
  client.println("<input type=\"submit\" value=\"Submit\">");
  client.println("</form>");
  client.println("</body>");
  client.println("</html>");
  delay(1);

}

void parseClientData(const String& request){

  if (request.indexOf("ledState=AllOff") > 0) {
    turnOffAll();
  } else if (request.indexOf("ledState=Green") > 0) {
    turnOffAll();
    CRGB colorChoice = CRGB::Green;
    lightUpColor(colorChoice);
  } else if (request.indexOf("ledState=Red") > 0) {
    turnOffAll();
    CRGB colorChoice = CRGB::Red;
    lightUpColor(colorChoice);
  } else if (request.indexOf("ledState=Pink") > 0) {
    turnOffAll();
    CRGB colorChoice = CRGB::DeepPink;
    lightUpColor(colorChoice);
  } else if (request.indexOf("ledState=Blue") > 0) {
    turnOffAll();
    CRGB colorChoice = CRGB::Blue;
    lightUpColor(colorChoice);
  } else if (request.indexOf("ledState=Purple") > 0) {
    turnOffAll();
    CRGB colorChoice = CRGB::DarkViolet;
    lightUpColor(colorChoice);
  } else if (request.indexOf("ledState=Yellow") > 0) {
    turnOffAll();
    CRGB colorChoice = CRGB::Yellow;
    lightUpColor(colorChoice);
  }
  
  readString = ""; // Clear the readString variable
  delay(1);

}
