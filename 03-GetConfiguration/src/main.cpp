#include <Arduino.h>
#include <ESP8266httpUpdate.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <otadrive_esp.h>

// configuration variables
int onDelay;
int offDelay;

// To inject firmware info into binary file, You have to use following macro according to let
// OTAdrive to detect binary info automatically
#define ProductKey "1c70ece5-07b2-4ee3-9ee3-9b9f566dfb2e" // Replace with your own APIkey
#define Version "1.0.0.7"
#define MakeFirmwareInfo(k, v) "&_FirmwareInfo&k=" k "&v=" v "&FirmwareInfo_&"

int *eeIntPtr;

void update();
void updateConfigs();
void saveConfigs();
void loadConfigs();

void setup()
{
  OTADRIVE.setInfo(ProductKey, Version);
  // put your setup code here, to run once:
  pinMode(2, OUTPUT);
  WiFi.begin("smarthomehub", "****");

  EEPROM.begin(32);
  eeIntPtr = (int *)EEPROM.getDataPtr();
  loadConfigs();

  Serial.begin(115200);
}

void loop()
{
  // put your main code here, to run repeatedly:
  digitalWrite(2, 1);
  delay(onDelay);
  digitalWrite(2, 0);
  delay(offDelay);
  Serial.printf("blink\n");

  if (WiFi.status() == WL_CONNECTED)
  {
    // every 120 seconds
    if (OTADRIVE.timeTick(120))
    {
      update();
      updateConfigs();
    }
  }
}

void saveConfigs()
{
  Serial.printf("save configs, on:%d, off:%d\n", onDelay, offDelay);
  eeIntPtr[1] = onDelay;
  eeIntPtr[2] = offDelay;
  EEPROM.commit();
}

void loadConfigs()
{
  // check configs initialized in eeprom or not
  if (eeIntPtr[0] != 0x4A)
  {
    // configs not initialized yet. write for first time
    eeIntPtr[0] = 0x4A; // memory sign
    eeIntPtr[1] = 200;  // onDelay
    eeIntPtr[2] = 250;  // offDelay
    EEPROM.commit();
  }
  else
  {
    // configs initialized and valid. read values
    onDelay = eeIntPtr[1];
    offDelay = eeIntPtr[2];
  }
}

void updateConfigs()
{
  String payload = OTADRIVE.getConfigs();
  DynamicJsonDocument doc(512);
  deserializeJson(doc, payload);

  Serial.printf("http content: %s\n", payload.c_str());

  if (doc.containsKey("onDelay") &&
      doc.containsKey("offDelay"))
  {
    onDelay = doc["onDelay"].as<int>();
    offDelay = doc["offDelay"].as<int>();
    saveConfigs();
  }
}

void update()
{
  auto r = OTADRIVE.updateFirmware();
  Serial.printf("Update result is: %s\n", r.toString().c_str());
}
