#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <Keypad.h>

LiquidCrystal_I2C LCD = LiquidCrystal_I2C(0x27, 16, 2);

#define NTP_SERVER     "vn.pool.ntp.org"
#define UTC_OFFSET     7
#define UTC_OFFSET_DST 0
#define DHTPIN 5           // Digital pin connected to the DHT sensor 
#define DHTTYPE DHT11     // DHT 22 (AM2302)

DHT dht(DHTPIN, DHTTYPE);

const uint8_t ROWS = 4;
const uint8_t COLS = 4;
char keys[ROWS][COLS] = 
{
  {'1', '4', '7', '*'},
  {'2', '5', '8', '0'},
  {'3', '6', '9', '#'},
  {'A', 'B', 'C', 'D'}
};

uint8_t rowPins[ROWS] = {13, 12, 14, 27} ; // Kết nối các hàng của keypad đến các chân digital
uint8_t colPins[COLS] = {26, 25, 33, 32};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

uint8_t state = 0;
unsigned long previousMillis = 0;
const long interval = 2000; // Update sensor data every 2 seconds

void sensor()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if(isnan(h) || isnan(t))
  {
    LCD.clear();
    LCD.setCursor(0, 1);
    LCD.printf("Error");
    return;
  }
  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.print("Temp: ");
  LCD.print(t);
  LCD.print(" 0C");
  LCD.setCursor(0, 1);
  LCD.print("Humd: ");
  LCD.print(h);
  LCD.print(" %RH");
}

void spinner() 
{
  static int8_t counter = 0;
  const char* glyphs = "\xa1\xa5\xdb";
  LCD.setCursor(15, 1);
  LCD.print(glyphs[counter++]);
  if (counter == strlen(glyphs)) {
    counter = 0;
  }
}

void printLocalTime() 
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) 
  {
    LCD.setCursor(0, 1);
    LCD.println("Connection error");
    return;
  }

  LCD.setCursor(0, 0);
  LCD.println(&timeinfo, "    %H:%M:%S    ");

  LCD.setCursor(0, 1);
  LCD.println(&timeinfo, "%d/%m/%Y   %Z");
}

void button() 
{
  // static unsigned long keyPressTime = 0;
  char key = keypad.getKey();

  switch(key)
  {
    case 'A':
    state = 0;
    break;
    case 'B':
    state = 1;
    break;
  }
}

void stateClock()
{
  if(state == 0) printLocalTime();
  else if(state == 1)
  {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) 
    {
      previousMillis = currentMillis;
      sensor();
    }
  }
}

void setup() 
{
  Serial.begin(9600);

  dht.begin();

  LCD.init();
  LCD.backlight();
  LCD.setCursor(0, 0);
  LCD.print("Connecting to ");
  LCD.setCursor(0, 1);
  LCD.print("WiFi ");


  WiFi.begin("GiayDep ToanDien", "vudotoandien", 0);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(250);
    spinner();
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  LCD.clear();
  LCD.setCursor(0, 1);
  LCD.println("Updating time...");

  configTime(UTC_OFFSET * 3600, UTC_OFFSET_DST, NTP_SERVER);
}

void loop() 
{
  stateClock();
  button();
  delay(100);
}
