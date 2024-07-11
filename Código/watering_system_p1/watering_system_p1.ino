#define BLYNK_TEMPLATE_ID "TMPL2mD3KtE3a"
#define BLYNK_TEMPLATE_NAME "Sistema Huertos"

#include <DHT.h>
#include <DHT_U.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define BLYNK_PRINT Serial
#define pinHumeTierra A0
#define pinDHT D4
#define pinLuz D7
#define pinBomba D3

BlynkTimer timer;
DHT dht(pinDHT, DHT11);
float temp = 0, humeAmbiente = 0;
int humeTierra = 0, luz = 0;
String mensajeLuz = "";
LiquidCrystal_I2C lcd(0x27, 16, 2);
bool Relay = 0;
char auth[] = "S_mGT1FwAB4WPgNUa3p2GC_1cMFeXHgW";
char ssid[] = "Bianca";
char pass[] = "cmcp7xnt";

void notifyOnTheft(){
  if (luz == LOW) {Blynk.logEvent("luz_directa", String("Se está recibiendo luz directa"));}
  //if (humeTierra > 60) {Blynk.logEvent("humedad_suelo_alto", String("Detén el riego. Humedad de suelo alta."));}
  //if (humeTierra < 40) {Blynk.logEvent("humedad_suelo_bajo", String("Activa el riego. Humedad de suelo baja"));}
  if (humeAmbiente > 75) {Blynk.logEvent("ambiente_alto", String("Enciende el deshumedecedor. Humedad de ambiente alta."));}
  if (humeAmbiente < 50) {Blynk.logEvent("ambiente_bajo", String("Apaga el deshumedecedor. Humedad de ambiente baja."));}
  if (temp > 22) {Blynk.logEvent("temperatura_alta", String("Moviliza la planta a otro ambiente. Temperatura alta."));}
  if (temp < 17) {Blynk.logEvent("temperatura_bajo", String("Moviliza la planta a otro ambiente. Temperatura baja."));}
}

BLYNK_WRITE(V1) {
  Relay = param.asInt();
  
  if (Relay == 1) {
    digitalWrite(pinBomba, LOW);
  } else {
    digitalWrite(pinBomba, HIGH);
  }
}

void readSoilMoistureSensor() {

  humeTierra = analogRead(pinHumeTierra);
  humeTierra = map(humeTierra, 0, 1024, 0, 100);
  humeTierra = (humeTierra - 100) * -1;

  Blynk.virtualWrite(V0, humeTierra);
  updateLCD();
}

void readLightSensor() {
  luz = digitalRead(pinLuz); 
  mensajeLuz = (luz == LOW) ? "Alert " : "Stable ";

  Blynk.virtualWrite(V3, !luz);
  updateLCD();
}

void readDHTSensor() {
  humeAmbiente = dht.readHumidity();
  temp = dht.readTemperature();
  
  Blynk.virtualWrite(V2, temp);
  Blynk.virtualWrite(V4, humeAmbiente);
  updateLCD();
}

void updateLCD() {
  lcd.setCursor(0, 0);
  lcd.print("HT:" + String(humeTierra) + "  L:" + String(mensajeLuz));
  lcd.setCursor(0, 1);
  lcd.print("T:" + String(temp) + "  HA:" + String(humeAmbiente) + "  ");
}

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  dht.begin();
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);

  lcd.setCursor(0, 0);
  lcd.print("Cargando");
  for (int a = 0; a <= 15; a++) {
    lcd.setCursor(a, 1);
    lcd.print(".");
    delay(500);
  }
  lcd.clear();
  pinMode(pinLuz, INPUT);
  pinMode(pinBomba, OUTPUT);
  digitalWrite(pinBomba, HIGH);

  timer.setInterval(5000L, readSoilMoistureSensor);
  timer.setInterval(5000L, notifyOnTheft);
  timer.setInterval(5000L, readDHTSensor);
  timer.setInterval(5000L, readLightSensor);
}

void loop() {
  Blynk.run(); 
  timer.run(); 
}