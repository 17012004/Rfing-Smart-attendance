#include <Adafruit_Fingerprint.h>
#include <MFRC522.h>
#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

#define SS_PIN 10
#define RST_PIN 9
#define BUZZER_PIN 7
#define SERVO_PIN 6  // Pin connected to the servo motor

MFRC522 rfid(SS_PIN, RST_PIN);
SoftwareSerial mySerial(2, 3);  // RX, TX
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

LiquidCrystal_I2C lcd(0x27, 16, 2); // Change the I2C address and dimensions according to your LCD module
Servo servo;                        // Create a servo object

struct FingerprintCardPair {
  int fingerId;
  String cardUid;
};

const int MAX_PAIRS = 2;  // Maximum number of fingerprint-card pairs
FingerprintCardPair fingerprintCardPairs[MAX_PAIRS] = {
  {1, "c1ebef19"},  // Fingerprint 1 is associated with Card UID 1
  {2, "d18b4924"}   // Fingerprint 2 is associated with Card UID 2
};

void setup()
{
  Serial.begin(9600);
  while (!Serial)
  ;  // Wait for serial connection

  finger.begin(57600);
  if (finger.verifyPassword())
  {
    Serial.println("Found fingerprint sensor!");
  }
  else
  {
    Serial.println("Did not find fingerprint sensor :(");
    while (1)
      ;
  }

  SPI.begin();
  rfid.PCD_Init();
  pinMode(BUZZER_PIN, OUTPUT);
  servo.attach(SERVO_PIN); // Attach the servo to the specified pin

  digitalWrite(BUZZER_PIN, HIGH);
  delay(1000);
  digitalWrite(BUZZER_PIN, LOW);
  lcd.begin();  // Initialize the LCD module
  lcd.setCursor(0, 0);
  lcd.print("Fingerprint ");
  lcd.setCursor(0, 1);
  lcd.print("Access System");
  delay(2000);
  lcd.clear();
}

void loop()
{
  lcd.setCursor(0, 0);
  lcd.print("Finger Found!");

  int verifiedFingerId = verifyFingerprint();
  if (verifiedFingerId != -1)
  {
    lcd.setCursor(0, 0);
    lcd.print("Fingerprint");
    lcd.setCursor(0, 1);
    lcd.print("verified");
    Serial.println("Fingerprint verified");
    delay(2000);
    lcd.clear();

    // Delay before scanning the card
    lcd.setCursor(0, 0);
    lcd.print("Scan your card...");
    Serial.println("Scan your card...");
    delay(2000);

    if (scanCard(verifiedFingerId))
    {
      lcd.setCursor(0, 0);
      lcd.print("Valid card");
      lcd.setCursor(0, 1);
      lcd.print("Access granted");
      Serial.println("Valid card - Access granted");
      //delay(2000);
      unlockDoor();
    }
    else
    {
      lcd.setCursor(0, 0);
      lcd.print("Invalid card");
      lcd.setCursor(0, 1);
      lcd.print("Access denied");
      Serial.println("Invalid card - Access denied");
      delay(4000);
      lcd.clear();
    }
  }
  else
  {
    lcd.setCursor(0, 0);
    lcd.print("Scan finger..");
    Serial.println("Fingerprint not found");
    delay(2000);
    lcd.clear();
  }

  delay(2000);
}

int verifyFingerprint()
{
  int id = finger.getImage();
  if (id != FINGERPRINT_OK)
  {
    return -1;
  }

  id = finger.image2Tz();
  if (id != FINGERPRINT_OK)
  {
    return -1;
  }

  id = finger.fingerFastSearch();
  if (id == FINGERPRINT_OK)
  {
    int fingerId = finger.fingerID;
    int confidence = finger.confidence;
    Serial.print("Scanned Finger ID: ");
    Serial.println(fingerId);
    if (fingerId == 1){
      lcd.clear();
      lcd.println("Hello Vansh");
      
      Serial.println("Student name: Vansh");
      delay(800);
      lcd.clear();
    }
    else if (fingerId == 2){
      lcd.clear();
      lcd.println("Hello Goransh");
      Serial.println("Student name: Goransh");
      delay(800);
      lcd.clear();
    }
    Serial.print("Confidence Level:  ");
    Serial.println(confidence);
    return fingerId;
  }
  else
  {
    Serial.println("Invalid fingerprint");
    delay(1000);
    return -1;
  }
}

bool scanCard(int verifiedFingerId)
{
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())
  {
    String cardUid = "";
    for (byte i = 0; i < 4; i++)
    {
      if (rfid.uid.uidByte[i] < 0x10)
      {
        cardUid += "0";
      }
      cardUid += String(rfid.uid.uidByte[i], HEX);
    }

    Serial.print("Card UID: ");
    Serial.println(cardUid);

    for (int i = 0; i < MAX_PAIRS; i++)
    {
      if (verifiedFingerId == fingerprintCardPairs[i].fingerId && cardUid == fingerprintCardPairs[i].cardUid)
      {
        return true;  // Fingerprint and card match
      }
    }

    return false;  // Fingerprint and card do not match
  }

  return false;  // No card present
}

void unlockDoor()
{
  digitalWrite(BUZZER_PIN, HIGH); // Activate the buzzer
  servo.write(0);                  // Rotate the servo to the locked position
  
  // Gradually open the door
  for (int pos = 0; pos <= 180; pos++)
  {
    servo.write(pos);      // Set the servo position
    delay(10);             // Wait for a small delay to create a gradual movement
  }
  lcd.clear();
  lcd.println("Door unlocked");
  delay(1000);
  lcd.clear();
  delay(1500);
  // Gradually close the door
  for (int pos = 180; pos >= 0; pos--)
  {
    servo.write(pos);      // Set the servo position
    delay(10);             // Wait for a small delay to create a gradual movement
  }
  
  digitalWrite(BUZZER_PIN, LOW);
  
  Serial.println("Door unlocked");
}
