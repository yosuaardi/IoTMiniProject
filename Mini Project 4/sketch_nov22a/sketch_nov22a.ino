#include <ESP32Servo.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 4         
#define DHTTYPE DHT11    
DHT dht(DHTPIN, DHTTYPE);

#define FIREBASE_HOST "iot-firebase-36809.firebaseio.com"
#define FIREBASE_AUTH "hvMO8JuvV0UZo9fTMs0N8TBIwZksLCZnlQ6lU9jD"

const char ssid[] = "Iromejan GK 3";
const char pass[] = "kosanputratahun2019";

Servo motorServo; 
WiFiClient net;

FirebaseData firebaseData;
String path = "/Node1";
unsigned long lastMillis = 0;
int pinLdr = 34;
int pinMotorServo = 13;
int stats = 0;  //status
int oldSuhu, newSuhu;
int oldLdr, newLdr;

void connect() {
  Serial.print("checking wifi...");
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nconnected!");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
  //Set database read timeout to 1 minute (max 15 minutes)
  Firebase.setReadTimeout(firebaseData, 1000 * 60);
  //tiny, small, medium, large and unlimited.
  //Size and its write timeout e.g. tiny (1s), small (10s), medium (30s) and large (60s).
  Firebase.setwriteSizeLimit(firebaseData, "tiny");
}



void gerakServo(){
  if(stats == 1){
    for(int angle = 0; angle <= 180; angle +=5) {
        motorServo.write(angle);
        delay(20);
    }
  
    for(int angle = 180; angle >= 0; angle -=5) {
        motorServo.write(angle);
        delay(20);
    }
  }
}


void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  motorServo.attach(pinMotorServo); 
  dht.begin();
  connect();
  oldLdr = analogRead(pinLdr);
  oldSuhu = dht.readTemperature();
}

void loop() {
  //mengambil nilai dari Node1/servo
  if(Firebase.getInt(firebaseData, path + "/servo")){
    if(firebaseData.intData() == 0){
      stats = 0;
    }else{
      stats = 1;
    }
  }
  delay(3000);
  gerakServo(); 
  
  //ambil nilai dari sensor LDR dan kirim ke firebase
  newLdr = analogRead(pinLdr);
  Serial.println(newLdr);
  if(newLdr != oldLdr){
    int lux = 0.009768*newLdr+10;     //konversi ke lux 
    Firebase.setDouble(firebaseData, path + "/ldr", lux);
    oldLdr = newLdr;
  }
 
  delay(10000);
  //ambil nilai dari sensor DHT11 dan kirim ke firebase
  int newSuhu = dht.readTemperature();
  Serial.println(newSuhu);
  if(newSuhu != oldSuhu){
    Firebase.setDouble(firebaseData, path + "/suhu", newSuhu);
    oldSuhu = newSuhu;
  }
}
