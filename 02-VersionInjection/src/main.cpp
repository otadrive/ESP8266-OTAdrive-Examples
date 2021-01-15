#include <Arduino.h>
#include <ESP8266httpUpdate.h>

// To inject firmware info into binary file, You have to use following macro according to let
// OTAdrive to detect binary info automatically
#define ProductKey "1c70ece5-07b2-4ee3-9ee3-9b9f566dfb2e" // Replace with your own APIkey
#define Version "1.0.0.7"
#define MakeFirmwareInfo(k, v) "&_FirmwareInfo&k=" k "&v=" v "&FirmwareInfo_&"

void update();
void setup()
{
  // put your setup code here, to run once:
  pinMode(2, OUTPUT);
  WiFi.begin("smarthomehub", "*****");
}

uint32_t updateCounter = 0;

void loop()
{
  // put your main code here, to run repeatedly:
  digitalWrite(2, 1);
  delay(350);
  digitalWrite(2, 0);
  delay(450);

  if (WiFi.status() == WL_CONNECTED)
  {
    updateCounter++;
    if (updateCounter > 20)
    {
      updateCounter = 0;
      update();
    }
  }
}

void update()
{
  String url = "http://otadrive.com/deviceapi/update?";
  url += MakeFirmwareInfo(ProductKey, Version);
  url += "&s=" + String(ESP.getChipId());

  WiFiClient client;
  ESPhttpUpdate.update(client, url, Version);
}
