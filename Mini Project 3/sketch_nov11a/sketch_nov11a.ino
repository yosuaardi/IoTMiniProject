#include <ESP32Servo.h>
#include <WiFi.h>
#include <MQTT.h>

const char ssid[] = "Iromejan GK 3";
const char pass[] = "kosanputratahun2019";

Servo motorServo; 
WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;
int pinLdr = 34;
int pinMotorServo = 13;
int stats = 0; //status
int cahaya = 0;
int lux = 0;
void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nconnecting...");
  while (!client.connect("esp32", "06609a74", "c2bddaa62802a5b3")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");

  client.subscribe("/ldr");
  client.subscribe("/status");
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
  if(topic == "/status"){
      if(payload == "1"){
        stats = 1;
        Serial.println("Sistem Hidup");
      }else if(payload == "0"){
        stats = 0;
        Serial.println("Sistem Mati");
      }
  }
  // Note: Do not use the client in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `client.loop()`.
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  motorServo.attach(pinMotorServo); 
  client.begin("broker.shiftr.io", net);
  client.onMessage(messageReceived);
  connect();
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

void loop() {
  client.loop();
  delay(10);  // <- fixes some issues with WiFi stability

  if (!client.connected()) {
    connect();
  }
  
  // publish a message roughly every second.
  if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    if(stats == 1){
      cahaya = analogRead(pinLdr);    // baca nilai analog 
      lux = 0.009768*cahaya+10;             //konversi ke lux 
      if (isnan(lux)) {
        Serial.println("Failed to read from LDR sensor!");
        return;
      }else{
        Serial.print("LUX: ");
        Serial.println(lux);
      }
      String luxString = String(lux);
      client.publish("/ldr", luxString);
    }else if(stats == 0){
      client.publish("/ldr", "0");
    }
    if(lux >= 15 && lux < 20){
      delay(3000);
      gerakServo();
    }
  }
}
