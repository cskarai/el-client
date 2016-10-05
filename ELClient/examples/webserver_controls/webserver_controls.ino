/**
 * This is a sample for trying out each HTML control
 * 
 * Test board:
 *    ESP8266 RX    <--level shifter-->    Arduino TX
 *    ESP8266 TX    <--level shifter-->    Arduino RX
 *    ESP8266 GPIO0 <--level shifter-->    Arduino RESET   (optional)
 *    
 *    Arduino LED is on PIN 13
 *    Connect an 1K trimmer to Arduino (voltage):  VCC <-> A0 <-> GND
 * 
 * 
 * Video:
 *    https://www.youtube.com/watch?v=vBESCO0UhYI
 */

#include <ELClient.h>
#include <ELClientWebServer.h>
#include "Pages.h"

// Initialize a connection to esp-link using the normal hardware serial port both for
// SLIP and for debug messages.
ELClient esp(&Serial, &Serial);

// Initialize the MQTT client
ELClientWebServer webServer(&esp);


// Callback made form esp-link to notify that it has just come out of a reset. This means we
// need to initialize it!
void resetCb(void) {
  Serial.println("EL-Client (re-)starting!");
  bool ok = false;
  do {
    ok = esp.Sync();      // sync up with esp-link, blocks for up to 2 seconds
    if (!ok) Serial.println("EL-Client sync failed!");
  } while(!ok);
  Serial.println("EL-Client synced!");
  
  webServer.registerCallback();
}

void setup()
{
  Serial.begin(115200);
  
  esp.SetReceiveBufferSize(384);
  esp.resetCb = resetCb;
  
  ledInit();
  userInit();
  voltageInit();
  webServer.setup();
}

void loop()
{
  esp.Process();

  ledLoop();
  voltageLoop();
}

