#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// --- Pin Definitions ---
#define PIN_LDR     21       // Digital Input
#define PIN_DS18B   17       // DS18B20 Temp Sensor
#define PIN_SOIL    32       // ADC1 Soil Sensor
#define PIN_RAIN    34       // Digital Input
#define PIN_DHT     4        // DHT22 Sensor
#define PIN_RELAY   18       // Relay Output

// --- Sensor Configuration ---
#define DHT_TYPE    DHT22    
DHT dht(PIN_DHT, DHT_TYPE);

OneWire oneWire(PIN_DS18B);
DallasTemperature soilTempSensor(&oneWire);

// --- Constants ---
const int SOIL_DRY = 4095;
const int SOIL_WET = 1500;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting ESP32 (Internal Pull-up Mode)...");

  // 1. Enable Internal Pull-up for DS18B20
  // This attempts to use the ESP32's internal ~45k resistor
  pinMode(PIN_DS18B, INPUT_PULLUP); 

  // 2. Setup Other Inputs
  pinMode(PIN_LDR, INPUT);
  pinMode(PIN_RAIN, INPUT);
  
  // 3. Setup Relay
  pinMode(PIN_RELAY, INPUT);
   

  // 4. Start Sensors
  dht.begin();
  soilTempSensor.begin();

  delay(2000); 
}

void loop() {
  // --- READ DATA ---
  float airHum = dht.readHumidity();
  float airTemp = dht.readTemperature();

  soilTempSensor.requestTemperatures(); 
  float soilTempC = soilTempSensor.getTempCByIndex(0);

  int soilRaw = analogRead(PIN_SOIL);
  int soilPercent = map(soilRaw, SOIL_DRY, SOIL_WET, 0, 100); 
  soilPercent = constrain(soilPercent, 0, 100);

  bool isRaining = digitalRead(PIN_RAIN) == LOW; 
  bool isLightDetected = digitalRead(PIN_LDR) == LOW; 

  // --- SERIAL OUTPUT ---
  Serial.println("--------------------------------");

  if (isnan(airHum) || isnan(airTemp)) {
    Serial.println("DHT22 Error");
  } else {
    Serial.print("Air: "); Serial.print(airTemp); Serial.print("C | Hum: "); Serial.print(airHum); Serial.println("%");
  }

  // Check for DS18B20 Errors
  if (soilTempC == -127.00) {
    Serial.println("Soil Temp: ERROR (Check connections/pull-up)");
  } else {
    Serial.print("Soil Temp: "); Serial.print(soilTempC); Serial.println(" C");
  }

  Serial.print("Soil Moisture: "); Serial.print(soilPercent); Serial.println("%");
  Serial.print("Rain: "); Serial.println(isRaining ? "YES" : "NO");
  Serial.print("Light: "); Serial.println(isLightDetected ? "YES" : "NO");

  // --- RELAY LOGIC ---
  if (soilPercent < 25 && !isRaining) {
    pinMode(PIN_RELAY, OUTPUT);
    digitalWrite(PIN_RELAY, LOW);
    
    
  } else {
    pinMode(PIN_RELAY, INPUT);
  }

  delay(3000); 
}