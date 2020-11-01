#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

char auth[] = "gkTJVIBRUt7dp1zsp4Xz3sExbVycW5j1";
char ssid[] = "Iromejan GK 3";
char pass[] = "kosanputratahun2019";

#define DHTPIN 4         //pin DHT
#define DHTTYPE DHT11    //tipe DHT 11
DHT dht(DHTPIN, DHTTYPE);

#define BLYNK_GREEN     "#23C48E"
#define BLYNK_RED       "#D3435C"

WidgetLED ledCahaya(V6);
WidgetLED ledSuhu(V7);

int pinPot = 34;
int pinLedCahaya = 25;
int pinLedSuhu = 27;

int batasSuhu;
int batasCahaya;
int suhu;
int cahaya;

bool hidup = false;

void warnaLed()
{
  ledCahaya.on();
  if(cahaya > batasCahaya){ // cahaya melebihi threshold pada slider
    ledCahaya.setColor(BLYNK_RED); //mengubah warna widget LED menjadi merah
    Serial.println("LED Indikator Cahaya: Merah");
    digitalWrite(pinLedCahaya, HIGH); //lampu nyala
  }else{
    ledCahaya.setColor(BLYNK_GREEN); //mengubah warna widget LED menjadi suhu
    Serial.println("LED Indikator Cahaya: Hijau");
    digitalWrite(pinLedCahaya, LOW); //lampu mati
  }
  
  ledSuhu.on();
  if(suhu > batasSuhu){ // suhu melebihi threshold pada slider
    ledSuhu.setColor(BLYNK_RED); //mengubah warna widget LED menjadi merah
    Serial.println("LED Indikator Suhu: Merah");
    digitalWrite(pinLedSuhu, HIGH); //lampu nyala
  }else{
    ledSuhu.setColor(BLYNK_GREEN); //mengubah warna widget LED menjadi hijau
    Serial.println("LED Indikator Suhu: Hijau");
    digitalWrite(pinLedSuhu, LOW); //lampu mati
  } 
}

void sendSensor()
{
  delay(10000);
  //cahaya
  cahaya = analogRead(pinPot);    // baca nilai analog 
  int lux = 0.009768*cahaya+10;   //konversi ke lux 
  if (isnan(lux)) {
    Serial.println("Failed to read from LDR sensor!");
    return;
  }
  Blynk.virtualWrite(V0, lux);    //menggambarkan nilai lux pada superchart
  Blynk.virtualWrite(V2, lux);    //memasukkan nilai cahaya ke label cahaya
  
  
  //suhu
  int temp = dht.readTemperature(); // baca suhu
  suhu = temp;
  if (isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Blynk.virtualWrite(V1, temp);  //menggambarkan nilai suhu pada superchart
  Blynk.virtualWrite(V3, temp);  //memasukkan nilai suhu ke label suhu
}


BLYNK_WRITE(V8)
{
  int pinValue = param.asInt(); // memasukkan nilai status button
  if(pinValue == 1){            // menghidupkan sistem
    Serial.println("Sistem Hidup");
    hidup = true;
  }else{                        // mematikan sistem
    Serial.println("Sistem Mati");
    hidup = false;

  }
}

BLYNK_WRITE(V4)
{
  int pinValue = param.asInt(); // memasukkan nilai slider cahaya
  batasCahaya = pinValue;
  Serial.print("Nilai TH Cahaya: ");
  Serial.println(pinValue);
}

BLYNK_WRITE(V5)
{
  int pinValue = param.asInt(); // memasukkan nilai slider suhu
  batasSuhu = pinValue;
  Serial.print("Nilai TH Suhu: ");
  Serial.println(pinValue);
}

void setup()
{
  // Debug console
  Serial.begin(115200);
  ledCahaya.on();
  ledSuhu.on();
  pinMode(pinLedCahaya, OUTPUT);
  pinMode(pinLedSuhu, OUTPUT);
  dht.begin();
  Blynk.begin(auth, ssid, pass, IPAddress(192,168,100,17), 8080);
}

void loop()
{
  Blynk.run();
  if(hidup){
    sendSensor();
    warnaLed();
  }else{
    ledCahaya.off();
    ledSuhu.off();
    digitalWrite(pinLedSuhu, LOW); //lampu indikator suhu mati
    digitalWrite(pinLedCahaya, LOW); //lampu indikator cahaya mati
  }
}
