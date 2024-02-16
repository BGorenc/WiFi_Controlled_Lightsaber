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
#include <map>

/* **********************SETTINGS********************** */
/* Settings for Arduino */
#define DATA_PIN 2                               // First LED Pin
#define DATA_PIN_CLONE 3                         // Second LED Pin
#define setArduinoIP IPAddress(192, 168, 1, 155) // Set the IP address of the Arduino
String webpageTitle = "Test Lightsaber Control"; // Title for the webpage
unsigned long randomModeDurationMinutes = 10;    // Set the duration in minutes

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
CRGB leds[NUM_LEDS];       // Second LED Strip
CRGB ledsClone[NUM_LEDS];  // Second LED Strip
WiFiServer server(80);
String readString;
std::map<String, CRGB> ledColors; // Hold the translations for the color selection to the CRGB LED color
bool randomMode = false;
unsigned long randomModeStartTime = 0;
unsigned long randomModeDuration = randomModeDurationMinutes * 60 * 1000; // limit to trigger in milliseconds

void setup(){

  LEDconfig();
  Serial.begin(BAUD);
  WiFi.config(setArduinoIP);
  connectToWiFi();
  server.begin();
  populateColorMap();
  randomSeed(analogRead(A0)); // seed the random function using a floating analog value
  startRandomMode();

}

void loop(){

  checkWiFi();
  checkRandomMode();
  runWebServer();

}

void LEDconfig(){

  FastLED.addLeds<CHIP_SET, DATA_PIN, COLOR_ORDER> (leds,NUM_LEDS);
  FastLED.addLeds<CHIP_SET, DATA_PIN_CLONE, COLOR_ORDER> (ledsClone,NUM_LEDS); // Second LED Strip
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setMaxPowerInVoltsAndMilliamps(MAX_VOLTS, MAX_AMPS);

}

void checkWiFi(){

  if (WiFi.status() != WL_CONNECTED){
    Serial.println("Wi-Fi connection lost. Reconnecting...");
    WiFi.disconnect();
    connectToWiFi(); // Reconnect to Wi-Fi if connection is lost
    Serial.println("Wi-Fi reconnected.");
  }

}

void connectToWiFi(){

  int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED){
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(10000);
  }

  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

}

void manualDisconnect(){

  // function exists for debugging disconnects
  WiFi.disconnect(); // Disconnect from Wi-Fi
  Serial.println("Manually disconnected from Wi-Fi.");

}

void checkRandomMode(){

  if (randomMode){
    unsigned long currentTime = millis();
    if (currentTime - randomModeStartTime >= randomModeDuration){
      Serial.println("Random Mode timer triggered");
      randomModeStartTime = currentTime;
      setRandomHue();
    }
  }

}

void runWebServer(){
  
  // serve client requests
  WiFiClient client = server.available();
  if (client){
    Serial.println();
    Serial.println("new client");

    while (client.connected()){
      if (client.available()){
        char clientData = client.read();
        if (readString.length() < 100){
          readString += clientData;
          Serial.write(clientData);

          if (clientData == '\n'){

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

void populateColorMap(){

  // Populate the map with the translations for the color selection to the CRGB LED color
  ledColors = {
    {"AllOff", CRGB::Black},
    {"Green", CRGB::Green},
    {"Red", CRGB::Red},
    {"Pink", CRGB::DeepPink},
    {"Blue", CRGB::Blue},
    {"Purple", CRGB::DarkViolet},
    {"Yellow", CRGB::Yellow}
  };

  for (const auto& entry : ledColors){
    Serial.print("Color: ");
    Serial.print(entry.first); // Print the color name (key)
    Serial.print("  R:");
    Serial.print(entry.second.r); // Print the red component of the color (value)
    Serial.print("  G:");
    Serial.print(entry.second.g); // Print the green component of the color (value)
    Serial.print("  B:");
    Serial.println(entry.second.b); // Print the blue component of the color (value)
  }
  
}

void setRandomHue(){

  uint8_t hue = random(256);
  Serial.print("Random Hue: ");
  Serial.println(hue);
  setHueLED(hue);
  delay(1);
}

void turnOffAll(){

  for(int i = (NUM_LEDS - 1); i >= 0; i--){
    leds[i] = CRGB::Black; // turn off the i'th LED
    ledsClone[i] = CRGB::Black; // Second LED Strip
    FastLED.show();
  }
  delay(1);
}

void lightUpColor(CRGB color){

  for(int i = 0; i < NUM_LEDS; i++){
    leds[i] = color;
    ledsClone[i] = color; // Second LED Strip
    FastLED.show();
  }
  delay(1);
}

void setHueLED(uint8_t hue){
  turnOffAll();
  for(int i = 0; i < NUM_LEDS; i++){
    leds[i].setHue(hue);
    ledsClone[i].setHue(hue); // Second LED Strip
    FastLED.show();
  }
  delay(1);
}

void startRandomMode(){

  Serial.println("Random Mode active");
  randomMode = true;
  randomModeStartTime = millis();
  setRandomHue();
  delay(1);
}

void displayWebPage(WiFiClient& client){

  client.println("<html>");
  client.println("<head>");
  client.println("<link rel='icon' href='data:;base64,iVBORw0KGgo='>");
  client.println("<title>" + webpageTitle + "</title>");
  client.println("<style>");
  client.println("body { background-color: #f2f2f2; font-family: Arial, sans-serif; }");
  client.println("h1 { color: #333333; text-align: center; }");
  client.println("form { text-align: center; }");
  client.println(".color-slider { width: 80%; margin: auto; background: linear-gradient(to right, red, orange, yellow, green, blue, indigo, violet); border-radius: 10px; padding: 10px; accent-color: gray; }");
  client.println("input[type='range'] { width: 100%; margin: 5px 0; }");
  client.println("input[type='number'] { width: 60px; margin-left: 10px; font-size: 18px; }"); // Style for the number input
  client.println("input[type='submit'] { padding: 10px 20px; font-size: 18px; background-color: #4CAF50; color: white; border: none; cursor: pointer; }");
  client.println("input[type='button'] { padding: 10px 20px; font-size: 18px; background-color: red; color: white; border: none; cursor: pointer; }"); // Style for the TurnOff button
  client.println("</style>");
  client.println("</head>");
  client.println("<body>");
  client.println("<h1>" + webpageTitle + "</h1>");
  client.println("<form method=\"get\" action=\"\" onsubmit=\"return false;\">"); // prevent HTML form from being resubmitted when server sends back
  client.println("<div class=\"color-slider\">");
  client.println("<input type=\"range\" name=\"ledState\" min=\"0\" max=\"255\" value=\"128\" onchange=\"updateSliderValue(this.value)\">");
  client.println("<input type=\"number\" name=\"ledState\" id=\"ledStateNumber\" min=\"0\" max=\"255\" value=\"128\" oninput=\"updateSliderValue(this.value)\">"); // Added a number input
  client.println("</div>");
  client.println("<br><br>");
  client.println("<input type=\"submit\" value=\"Static Color\" onclick=\"staticColor()\">");
  client.println("<br><br>");
  client.println("<input type=\"submit\" value=\"Random Mode\" onclick=\"randomMode()\">");
  client.println("<br><br>");
  client.println("<input type=\"button\" value=\"TurnOff\" onclick=\"turnOffLED()\">");
  client.println("</form>");
  client.println("<script>");
  client.println("function updateSliderValue(value) { document.getElementById('ledStateNumber').value = value; document.querySelector('[name=\"ledState\"]').value = value; }"); // JavaScript to update the value dynamically
  client.println("function randomMode() { window.location.href = '/?ledState=Random'; }"); // JavaScript function for TurnOff Random
  client.println("function turnOffLED() { window.location.href = '/?ledState=TurnOff'; }"); // JavaScript function for TurnOff button
  client.println("function staticColor() { window.location.href = '/?ledState=' + document.querySelector('[name=\"ledState\"]').value; }"); // JavaScript function for Static Color button
  client.println("</script>");
  client.println("</body>");
  client.println("</html>");
  delay(1);

}


void parseClientData(const String& request){

  // Check to make sure the ledStateIndex exists
  int ledStateIndex = request.indexOf("ledState=");
  if (ledStateIndex >= 0){
    // Find the final index of the request to trim excess
    int ledStateEndIndex = request.indexOf(" ", ledStateIndex);
    if (ledStateEndIndex == -1){
      ledStateEndIndex = request.length(); // Use whole string when no match
    }
    // Get the ledState= value from the request find the CRGB translation in the map 
    // Trigger the turn off current color animation and light up the new color
    String ledState = request.substring(ledStateIndex + 9, ledStateEndIndex);
    Serial.println("ledState: " + ledState);
    // Parse Request
    if (ledState == "Random"){
      Serial.println("Random selection made");
      startRandomMode();
    } else if (ledState == "TurnOff") {
      Serial.println("Turn Off Lightsaber");
      turnOffAll();
    } else{
      Serial.println("Static color selection made");
      randomMode = false;
      CRGB colorChoice = ledColors[ledState];
      turnOffAll();
      lightUpColor(colorChoice);
    }

  } else{
    Serial.println("No Selection Found");
  }
  readString = ""; // Clear the readString variable
  delay(1);
}
