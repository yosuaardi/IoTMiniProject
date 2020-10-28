#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

const int pushButton = 33; //pin push button
const int ledPinMerah =  27;// pin LED merah
const int ledPinKuning =  26;// pin LED kuning
const int ledPinHijau =  25;// pin LED hijau

const int dhtPin = 4; //pin DHT11
#define DHTTYPE DHT11 //tipe DHT sensors
DHT dht (dhtPin, DHTTYPE) ;
bool hidup = false;

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(9600);
  pinMode(pushButton, INPUT);
  pinMode(ledPinMerah, OUTPUT);
  pinMode(ledPinKuning, OUTPUT);
  pinMode(ledPinHijau, OUTPUT);
  dht.begin();
}

// the loop routine runs over and over again forever:
void loop() {
  // membaca pin input
  int buttonState = digitalRead(pushButton);
  if(buttonState){
    if(hidup == true){
      digitalWrite(ledPinMerah, LOW);
      digitalWrite(ledPinHijau, LOW);
      digitalWrite(ledPinKuning, LOW);
      hidup = false;
      delay(1000);
      Serial.println("Mematikan sistem");
    }else{
      hidup = true;
      delay(1000);
      Serial.println("Menghidupkan sistem");
    }
  }
  if(hidup){
      delay(10000);
      float temp = dht.readTemperature();
      if (isnan(temp)) {
         Serial.println(F("Gagal membaca dari DHT sensor!"));
      }else{
         Serial.println(temp);
      } 
      if(temp < 35.0){ //suhu < 35
        digitalWrite(ledPinMerah, LOW);
        digitalWrite(ledPinKuning, LOW);
        digitalWrite(ledPinHijau, HIGH);
      }else if(temp >= 35.0 && temp <= 50){ // suhu 35 - 50
        digitalWrite(ledPinMerah, LOW);
        digitalWrite(ledPinKuning, HIGH);
        digitalWrite(ledPinHijau, HIGH);
      }else if(temp > 50){ //suhu > 50
        digitalWrite(ledPinMerah, HIGH);
        digitalWrite(ledPinKuning, HIGH);
        digitalWrite(ledPinHijau, HIGH);
      }
   }
}
