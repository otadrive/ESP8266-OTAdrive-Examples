#include <Arduino.h>
#include <ESP8266httpUpdate.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

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
  // put your setup code here, to run once:
  pinMode(2, OUTPUT);
  WiFi.begin("smarthomehub", "****");

  EEPROM.begin(32);
  eeIntPtr = (int *)EEPROM.getDataPtr();
  loadConfigs();

  Serial.begin(115200);
}

uint32_t updateCounter = 0;

void loop()
{
  // put your main code here, to run repeatedly:
  digitalWrite(2, 1);
  delay(onDelay);
  digitalWrite(2, 0);
  delay(offDelay);
  Serial.printf("blink: %d\n", updateCounter);

  if (WiFi.status() == WL_CONNECTED)
  {
    updateCounter++;
    if (updateCounter > 20)
    {
      updateCounter = 0;
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
  WiFiClient client;
  HTTPClient http;
  String url = "http://www.otadrive.com/deviceapi/getconfig?";
  url += MakeFirmwareInfo(ProductKey, Version);
  url += "&s=" + String(ESP.getChipId());
  client.setTimeout(1);
  http.setTimeout(1000);
  http.setTimeout(1000);

  Serial.println(url);

  if (http.begin(client, url))
  {
    int httpCode = http.GET();
    Serial.printf("http code: %d\n", httpCode);

    // httpCode will be negative on error
    if (httpCode == HTTP_CODE_OK)
    {
      String payload = http.getString();
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
