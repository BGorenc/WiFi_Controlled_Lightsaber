#include <SPI.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"
#include <FastLED.h>

#define DATA_PIN 2
#define setArduinoIP IPAddress(192, 168, 1, 151) // set the IP address of the Arduino

// LED strip setup values
#define NUM_LEDS 144
#define CHIP_SET WS2812B
#define COLOR_ORDER GRB
#define BRIGHTNESS 200
#define MAX_VOLTS 5
#define MAX_AMPS 4500
#define BAUD 57600

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASSWORD;
int keyIndex = 0;
CRGB leds[NUM_LEDS];

int status = WL_IDLE_STATUS;
WiFiServer server(80);

String readString;

void turnOffAll() {

  //delay(1); // wait for 5 seconds before turning off the LEDs
  for(int i = (NUM_LEDS - 1); i >= 0; i--) {
    leds[i] = CRGB::Black; // turn off the i'th LED
    FastLED.show();
  }
  delay(1); // wait for 1 second before repeating the loop

}

void lightUpColor(CRGB color) {

  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = color;
    FastLED.show();
  }

  delay(1); // wait for 1 second before repeating the loop

}

void setup() {
  //pinMode(led, OUTPUT);
  FastLED.addLeds<CHIP_SET, DATA_PIN, COLOR_ORDER> (leds,NUM_LEDS);
	FastLED.setBrightness(BRIGHTNESS);
  FastLED.setMaxPowerInVoltsAndMilliamps(MAX_VOLTS, MAX_AMPS);
  Serial.begin(BAUD);

  WiFi.config(setArduinoIP);

  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(10000);
  }
  server.begin();

  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("new client");

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (readString.length() < 100) {
          readString += c;
          Serial.write(c);

          if (c == '\n') {
            client.println("<html>");
            client.println("<head><title>Jedi Lightsaber</title></head>");
            client.println("<body>");
            client.println("<h1>Jedi Lightsaber Control</h1>");
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
            client.println("<br>");
            client.println("<br>");
            client.println("<input type=\"submit\" value=\"Submit\">");
            client.println("</form>");
            client.println("</body>");
            client.println("</html>");

            delay(1);

            if (readString.indexOf("ledState=AllOff") > 0) {
              turnOffAll();
              //delay(1);
            } else if (readString.indexOf("ledState=Green") > 0) {
              turnOffAll();
              //delay(1);
              CRGB colorChoice = CRGB::Green;
              lightUpColor(colorChoice);
              //delay(1);
            } else if (readString.indexOf("ledState=Red") > 0) {
              turnOffAll();
              //delay(1);
              CRGB colorChoice = CRGB::Red;
              lightUpColor(colorChoice);
              //delay(1);
            } else if (readString.indexOf("ledState=Pink") > 0) {
              turnOffAll();
              //delay(1);
              CRGB colorChoice = CRGB::DeepPink;
              lightUpColor(colorChoice);
              //delay(1);
            } else if (readString.indexOf("ledState=Blue") > 0) {
              turnOffAll();
              //delay(1);
              CRGB colorChoice = CRGB::Blue;
              lightUpColor(colorChoice);
              //delay(1);
            } else if (readString.indexOf("ledState=Purple") > 0) {
              turnOffAll();
              //delay(1);
              CRGB colorChoice = CRGB::DarkViolet;
              lightUpColor(colorChoice);
              //delay(1);
            } else if (readString.indexOf("ledState=Yellow") > 0) {
              turnOffAll();
              //delay(1);
              CRGB colorChoice = CRGB::Yellow;
              lightUpColor(colorChoice);
              //delay(1);
            }

            readString = "";

            delay(1);
            client.stop();
            Serial.println("client disconnected");
          }
        }
      }
    }
  }
}
