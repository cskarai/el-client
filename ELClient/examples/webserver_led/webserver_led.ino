/**
 * Simple example for LED flashing using web-server
 * 
 * Test board:
 *    ESP8266 RX    <--level shifter-->    Arduino TX
 *    ESP8266 TX    <--level shifter-->    Arduino RX
 *    ESP8266 GPIO0 <--level shifter-->    Arduino RESET   (optional)
 * 
 *    Arduino LED is on PIN 13
 * 
 * Video:
 *    https://www.youtube.com/watch?v=vBESCO0UhYI
 */

#include <ELClient.h>
#include <ELClientWebServer.h>

// flash LED on PIN 13
#define LED_PIN 13

// Initialize a connection to esp-link using the normal hardware serial port both for
// SLIP and for debug messages.
ELClient esp(&Serial, &Serial);

// Initialize the MQTT client
ELClientWebServer webServer(&esp);

void ledPageLoadAndRefreshCb(const char * url)
{
  if( digitalRead(LED_PIN) )
    webServer.setArgString(F("text"), F("LED is on"));
  else
    webServer.setArgString(F("text"), F("LED is off"));
}

void ledButtonPressCb(const char * btnId)
{
  String id = btnId;
  if( id == F("btn_on") )
    digitalWrite(LED_PIN, true);
  else if( id == F("btn_off") )
    digitalWrite(LED_PIN, false);
}

void setup()
{
  Serial.begin(115200);
  
  URLHandler *ledHandler = webServer.createURLHandler(F("/SimpleLED.html.json"));
  ledHandler->loadCb.attach(&ledPageLoadAndRefreshCb);
  ledHandler->refreshCb.attach(&ledPageLoadAndRefreshCb);
  ledHandler->buttonCb.attach(&ledButtonPressCb);

  webServer.setup();
}

static uint32_t last;

void loop()
{
  esp.Process();

  if ((millis()-last) > 4000) {
    webServer.registerCallback();  // just for esp-link reset: reregister callback at every 4s
    last = millis();
  }
}

