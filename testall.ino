#include "ThingSpeak.h"
#include "WiFi.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include<DHT.h>

//----------- Enter you Wi-Fi Details---------//
char ssid[] = "White_Hat"; //SSID
char pass[] = "9284351582"; // Password
//-------------------------------------------//

#define LDR_PIN 35//LDR INPUT 
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 4
#define DHTPIN 26
#define ballSwitchPin 2
const int irTransmitterPin = 25;  // Replace with your IR transmitter pin
const int irReceiverPin = 26; 
const int vibrationPin = 18;  // Connect the signal pin of the KY-002 to GPIO 2 on the ESP32
const int Buzzer = 17; // Use buzzer for alert 
const int FlamePin = 12;  // This is for input pin

float humidity;
float temperature;
int ldr_val;
int ballSwitchState,shockValue;
float windSpeed;
int temp,Flame,temp1;

volatile unsigned int interruptCount = 0;
unsigned long previousMillis = 0;
const unsigned long interval = 5000;  // Measurement interval in milliseconds 

DHT dht(26,DHT11);
WiFiClient  client;

Adafruit_SSD1306 isplay(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long myChannelField =  2421313; // Channel ID
const int ChannelField1 = 1; // Which channel to write data
const int ChannelField2=2;
const int ChannelField3 = 3; 
const int ChannelField4 = 4;
const int ChannelField5 = 5; // Which channel to write data
const int ChannelField6 =6;
const char * myWriteAPIKey = "I3YQP9DCA2G63PBA"; // Your write API Key

void IRAM_ATTR countInterrupt() {
  interruptCount++;
}

void setup() 
{
  pinMode(LDR_PIN,INPUT);
  pinMode(vibrationPin, INPUT);
  pinMode(irTransmitterPin, OUTPUT);
  pinMode(irReceiverPin, INPUT_PULLUP);
  pinMode(Buzzer, OUTPUT);
  pinMode(FlamePin, INPUT);
  attachInterrupt(digitalPinToInterrupt(irReceiverPin), countInterrupt, FALLING);
  isplay.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  dht.begin();
  isplay.display();
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);
  delay(2000);
}

void calculateWindSpeed() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    detachInterrupt(digitalPinToInterrupt(irReceiverPin));  // Disable interrupts
    float rotations = static_cast<float>(interruptCount) / 2.0;  // Two interruptions per rotation

    // Assuming each cup diameter is 10 cm 
    float circumference = 3.1416 * 0.1;  // Pi * diameter
    float distance = rotations * circumference;  // Distance traveled by cups

    // Calculate speed in m/s
    windSpeed = distance / (static_cast<float>(interval) / 1000.0);

    Serial.print("Wind Speed: ");
    Serial.print(windSpeed);
    Serial.println(" m/s");

    interruptCount = 0;
    previousMillis = currentMillis;

    attachInterrupt(digitalPinToInterrupt(irReceiverPin), countInterrupt, FALLING);  // Enable interrupts
  }
} 
void loop()
 {

  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  ldr_val = analogRead(LDR_PIN);
  shockValue = digitalRead(vibrationPin);
  Flame = digitalRead(FlamePin);
  
  digitalWrite(irTransmitterPin, HIGH);
  delay(1000);  // Adjust the delay as needed for your IR transmitter
  digitalWrite(irTransmitterPin, LOW);
  

  if (isnan(humidity) || isnan(temperature)) 
  {
        Serial.println("Failed to read from DHT sensor!");
        return;
  }
  if (ldr_val>1000)
  {
   Serial.println("HIGH Light Intensity ");
   digitalWrite(Buzzer, HIGH);
   delay(3000);
   digitalWrite(Buzzer, LOW);
  }
  else if(shockValue ==  1) 
  {
    Serial.println("EarthQuake occurs");
   digitalWrite(Buzzer, HIGH);
   delay(3000);
   digitalWrite(Buzzer, LOW);
  }
    else if(Flame == 1 ) 
  {
    Serial.println("Fire is There");
   digitalWrite(Buzzer, HIGH);
   delay(3000);
   digitalWrite(Buzzer, LOW);
  }
    else if(temperature > 28)
  {
    Serial.println("Temprature is HIGH");
   digitalWrite(Buzzer, HIGH);
   delay(3000);
   digitalWrite(Buzzer, LOW);
  }
    else if(windSpeed > 10)
  {
    Serial.println("WindStrom Occurs");
   digitalWrite(Buzzer, HIGH);
   delay(3000);
   digitalWrite(Buzzer, LOW);
  }
  else
  {
   Serial.println("Weather is Normal");

  }
  /*Display on serial mointor*/
  Serial.println("******************************************************************************");
  Serial.println("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C ");

  Serial.println("Humidity: ");
  Serial.print(humidity);
  Serial.println(" % ");  

  Serial.println("LDR Value: ");
  Serial.println(ldr_val);

  delay(1000);

  calculateWindSpeed();

  delay(1000);

if (shockValue == HIGH) {
        temp=0;
        Serial.println("No vibration detected!");

  } else {
        temp=1;
        Serial.println("Vibration detected!");
          }
if (Flame == HIGH)
  {
    temp1=1;
    digitalWrite(Buzzer, HIGH);
    Serial.println("Alarm Activated");
  }
  else
  {
    temp1=0;
    digitalWrite(Buzzer, LOW);
    Serial.println("Alarm Deactivated");
  }

Serial.println("******************************************************************************");

/*Display on OLED*/

  isplay.clearDisplay();
  isplay.setTextSize(1);
  isplay.setTextColor(SSD1306_WHITE);

  isplay.setCursor(0, 0);
  isplay.println(F("Temperature:"));
  isplay.setCursor(80, 1);
  isplay.println(temperature); 
  
  isplay.setCursor(0, 10);
  isplay.println(F("Humidity:"));
  isplay.setCursor(80, 10);
  isplay.println(humidity);

  isplay.setCursor(0, 20);
  isplay.println(F("LDR:"));
  isplay.setCursor(80, 20);
  isplay.println(ldr_val);

  isplay.setCursor(0, 30);
  isplay.println(F("Wind Speed:"));
  isplay.setCursor(80, 30);
  isplay.println(windSpeed);

  isplay.setCursor(0, 40);
  isplay.println(F("Vibration:"));
  isplay.setCursor(80, 40);
  isplay.println(temp);

  isplay.setCursor(0, 50);
  isplay.println(F("Flame:"));
  isplay.setCursor(80, 50);
  isplay.println(temp1);
  isplay.display();

  delay(1000);// Display for 1 second

  isplay.clearDisplay();  

if (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    while (WiFi.status() != WL_CONNECTED)
    {
      WiFi.begin(ssid, pass);
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\nConnected.");
  }
  ThingSpeak.writeField(myChannelField, ChannelField1, temperature, myWriteAPIKey);
  ThingSpeak.writeField(myChannelField, ChannelField2, humidity, myWriteAPIKey);
  ThingSpeak.writeField(myChannelField, ChannelField3, ldr_val, myWriteAPIKey);
  ThingSpeak.writeField(myChannelField, ChannelField4, windSpeed, myWriteAPIKey);
  ThingSpeak.writeField(myChannelField, ChannelField5, temp, myWriteAPIKey);
  ThingSpeak.writeField(myChannelField, ChannelField6, temp1, myWriteAPIKey);
 
}


