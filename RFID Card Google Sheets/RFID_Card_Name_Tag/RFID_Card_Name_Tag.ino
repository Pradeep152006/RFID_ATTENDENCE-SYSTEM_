
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <MFRC522.h>

//---------------- RFID -----------------
#define RST_PIN D3
#define SS_PIN  D4
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

int blockNum = 2;
byte blockData[16] = {"bunty"};  // Name to write
byte bufferLen = 18;
byte readBlockData[18];
MFRC522::StatusCode status;

//---------------- OLED -----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//---------------------------------------
void setup() {
  Serial.begin(9600);

  // RFID
  SPI.begin();
  mfrc522.PCD_Init();
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  // OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Scan RFID Tag...");
  display.display();

  Serial.println("Scan a RFID Tag...");
}

//---------------------------------------
void loop() {
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  Serial.println("\n**Card Detected**");
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Card Detected!");

  // Write data
  Serial.println("Writing to RFID...");
  WriteDataToBlock(blockNum, blockData);

  // Read data
  Serial.println("Reading from RFID...");
  ReadDataFromBlock(blockNum, readBlockData);

  // Show name on OLED
  display.setCursor(0,20);
  display.print("Name: ");
  for (int i=0; i<16; i++) {
    display.write(readBlockData[i]);
    Serial.write(readBlockData[i]);
  }
  display.display();
  Serial.println();
  delay(2000);
}

//---------------------------------------
void WriteDataToBlock(int blockNum, byte blockData[]) {
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,
                                    blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.println("Auth failed (Write)");
    return;
  }
  status = mfrc522.MIFARE_Write(blockNum, blockData, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.println("Write failed");
    return;
  }
  Serial.println("Data written OK");
}

void ReadDataFromBlock(int blockNum, byte readBlockData[]) {
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,
                                    blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.println("Auth failed (Read)");
    return;
  }
  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK) {
    Serial.println("Read failed");
    return;
  }
  Serial.println("Data read OK");
}
