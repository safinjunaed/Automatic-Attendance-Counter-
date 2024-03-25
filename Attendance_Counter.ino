#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Tinkercademy.h>
#include <TinyGsmClient.h>

#define OLED_RESET -1
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid = "Teachers-RUET";
const char* password = "RUET@2018";
const char* tinkerToken = "YourTinkerCadToken";
const char* gsmPhoneNumber = "01842037779";

const int capacity = 25;
int peopleCount = 0;

int sensor1Pin = 12; 
int sensor2Pin = 13;

bool sensor1State = LOW;
bool sensor2State = LOW;

TinyGsm modem(Serial2);

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17); // GSM module connected to UART2

  // Initialize OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.display();
  delay(2000);
  display.clearDisplay();

  pinMode(sensor1Pin, INPUT_PULLUP);
  pinMode(sensor2Pin, INPUT_PULLUP);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize Tinkercad
  Tinkercad.begin(tinkerToken);

  Tinkercad.addStream("PeopleCount", &peopleCount);

  modem.restart();
  delay(3000);
}

void loop() {
  bool sensor1Reading = digitalRead(sensor1Pin);
  bool sensor2Reading = digitalRead(sensor2Pin);

  if (sensor1Reading != sensor1State && sensor2Reading != sensor2State) {
    // Both sensors changed state simultaneously, ignore
  } else if (sensor1Reading == HIGH && sensor2Reading == LOW) {
    // Sensor 1 triggered first (someone entered)
    peopleCount++;
    displayCount(peopleCount);
    sendToTinkercad(peopleCount);
    checkCapacity(peopleCount);
  } else if (sensor2Reading == HIGH && sensor1Reading == LOW) {
    // Sensor 2 triggered first (someone left)
    if (peopleCount > 0) {
      peopleCount--;
      displayCount(peopleCount);
      sendToTinkercad(peopleCount);
    }
  }

  sensor1State = sensor1Reading;
  sensor2State = sensor2Reading;

  delay(100);
}

void displayCount(int count) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("People Count:");
  display.println(count);
  display.display();
}

void sendToTinkercad(int count) {
  Tinkercad.stream("PeopleCount", count);
}

void checkCapacity(int count) {
  if (count >= capacity) {
    sendAlert();
  }
}

void sendAlert() {
  // Send SMS alert using GSM module
  modem.sendSMS(gsmPhoneNumber, "Room has reached maximum capacity of 25 people!");
}
