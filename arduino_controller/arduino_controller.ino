#include <SPI.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"

#define led 2
#define setArduinoIP IPAddress(192, 168, 1, 150) // set the IP address of the Arduino

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASSWORD;
int keyIndex = 0;

int status = WL_IDLE_STATUS;
WiFiServer server(80);

String readString;

void setup() {
  pinMode(led, OUTPUT);
  Serial.begin(9600);

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
            client.println("<head><title>Sith Lightsaber</title></head>");
            client.println("<body>");
            client.println("<h1>Sith Lightsaber Control</h1>");
            client.println("<form method=\"get\" action=\"\">");
            client.println("<select name=\"ledState\">");
            client.println("<option value=\"on\">Turn On Light</option>");
            client.println("<option value=\"off\">Turn Off Light</option>");
            client.println("</select>");
            client.println("<br>");
            client.println("<br>");
            client.println("<input type=\"submit\" value=\"Submit\">");
            client.println("</form>");
            client.println("</body>");
            client.println("</html>");

            delay(1);

            if (readString.indexOf("ledState=on") > 0) {
              digitalWrite(led, HIGH);
              delay(1);
            } else if (readString.indexOf("ledState=off") > 0) {
              digitalWrite(led, LOW);
              delay(1);
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