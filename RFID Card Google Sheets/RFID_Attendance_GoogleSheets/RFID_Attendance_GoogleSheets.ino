#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//-----------------------------------------
#define RST_PIN  D3
#define SS_PIN   D4
#define BUZZER   D8
//-----------------------------------------
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;
//-----------------------------------------
int blockNum = 2;
byte bufferLen = 18;
byte readBlockData[18];
//-----------------------------------------
String card_holder_name;
const String sheet_url = "https://script.google.com/macros/s/AKfycbzZOBSw-wFLgTfwVl3lxkSco6JHxHJb9MjCkHSgbRiMDY1pU9aOlob8NQkmiVyiSP_o/exec";  // Replace with your actual script URL
//-----------------------------------------
#define WIFI_SSID "Ravindra"         // Replace with your WiFi SSID
#define WIFI_PASSWORD "ravi12345"     // Replace with your WiFi Password
//-----------------------------------------

// OLED configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/****************************************************************************************************
 * setup() function
 ****************************************************************************************************/
void setup()
{
  Serial.begin(9600);

  // OLED Initialization
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED init failed"));
    while (true);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.println("Initializing...");
  display.display();
  delay(3000);

  // WiFi Connection
  Serial.println();
  Serial.print("Connecting to AP");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  // BUZZER as OUTPUT
  pinMode(BUZZER, OUTPUT);

  // SPI and RFID initialization
  SPI.begin();
  mfrc522.PCD_Init();
}


/****************************************************************************************************
 * loop() function
 ****************************************************************************************************/
void loop()
{
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("Scan your Card");
  display.display();

  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  Serial.println(F("Reading data from RFID..."));
  ReadDataFromBlock(blockNum, readBlockData);

  Serial.print(F("Last data in RFID: "));
  Serial.print(blockNum);
  Serial.print(F(" --> "));

  String name = "";
  for (int i = 0; i < 16; i++) {
  // Only add printable characters
  if (readBlockData[i] >= 32 && readBlockData[i] <= 126) {
    name += (char)readBlockData[i];
   }
  }
name.trim();  // Remove extra spaces

  Serial.println(name);

  // Show name on OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Hey");
  display.setTextSize(2);
  display.setCursor(0, 20);
  display.println(name);
  display.display();

mfrc522.PICC_HaltA();      // Halt the current card
mfrc522.PCD_StopCrypto1(); // Stop encryption on the PCD

  // Buzzer beep
  for (int i = 0; i < 2; i++) {
    digitalWrite(BUZZER, HIGH);
    delay(200);
    digitalWrite(BUZZER, LOW);
    delay(200);
  }

  // Send data to Google Sheet
  if (WiFi.status() == WL_CONNECTED) {
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    client->setInsecure();

    card_holder_name = sheet_url + "?name=" + name;
    card_holder_name.trim();
    Serial.println(card_holder_name);

    HTTPClient https;
    Serial.print(F("[HTTPS] begin...\n"));

    if (https.begin(*client, card_holder_name)) {
      int httpCode = https.GET();
      if (httpCode > 0) {
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
        display.setTextSize(1);
        display.setCursor(0, 50);
        display.println("Data Recorded");
        display.display();
        delay(2000);
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }
      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
    delay(1000);
  }
}


/****************************************************************************************************
 * ReadDataFromBlock() function
 ****************************************************************************************************/
void ReadDataFromBlock(int blockNum, byte readBlockData[])
{
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));

  if (status != MFRC522::STATUS_OK) {
    Serial.print("Authentication failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  } else {
    Serial.println("Authentication success");
  }

  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
  } else {
    Serial.println("Block was read successfully");
  }
}
