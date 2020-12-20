#include "secrets.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"

#include <SPI.h>
#include <MFRC522.h>
#include <IRremote.h>

//Remote
int RECV_PIN = 15; // the pin where you connect the output pin of IR sensor     
IRrecv irrecv(RECV_PIN);     
decode_results results;  

//RFID
#define SS_PIN 21
#define RST_PIN 22
MFRC522 mfrc522(SS_PIN, RST_PIN);   

//Relay
int relayPin = 5;

//LDR
int pinLdr = 34;
int cahaya = 0;
int lux = 0;

//Wifi
const char ssid[] = "Iromejan GK 3";
const char pass[] = "kosanputratahun2019";

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);

int statusLampu = 0;
int statusMode = 0;
int akses = 0;

String user = "";
String code = "";

void connectWifi() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.print("\nconnecting...");
  Serial.println("\nconnected!");
}

void connectToAWS()
{
  // Configure WiFiClientSecure to use the AWS certificates we generated
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  // Try to connect to AWS and count how many times we retried.
  int retries = 0;
  Serial.print("Connecting to AWS IOT");

  while (!client.connect(THINGNAME) && retries < AWS_MAX_RECONNECT_TRIES) {
    Serial.print(".");
    delay(100);
    retries++;
  }

  // Make sure that we did indeed successfully connect to the MQTT broker
  // If not we just end the function and wait for the next loop.
  if(!client.connected()){
    Serial.println(" Timeout!");
    return;
  }

  // If we land here, we have successfully connected to AWS!
  // And we can subscribe to topics and send messages.
  Serial.println("Connected!");
}

void sendJsonToAWS()
{
  StaticJsonDocument<128> jsonDoc;
  JsonObject stateObj = jsonDoc.createNestedObject("state");
  JsonObject reportedObj = stateObj.createNestedObject("reported");
  
  // Write the user, akses, mode, lampu, ldr
  reportedObj["user"] = user;
  if(akses == 1){
    reportedObj["akses"] = "true";
  }else{
    reportedObj["akses"] = "false";
  }
  if(statusMode == 1){
    reportedObj["mode"] = "remote";
  }else if(statusMode == 2){
    reportedObj["mode"] = "ldr";
  }else{
    reportedObj["mode"] = "";
  }
  if(statusLampu == 1){
    reportedObj["lampu"] = "hidup";
  }else{
    reportedObj["lampu"] = "mati";  
  }
  reportedObj["ldr"] = lux;
  //reportedObj["wifi_strength"] = WiFi.RSSI();
  
  // Create a nested object "location"
  JsonObject locationObj = reportedObj.createNestedObject("location");
  locationObj["name"] = "Home";
  Serial.println("Publishing message to AWS...");
  //serializeJson(doc, Serial);
  char jsonBuffer[512];
  serializeJson(jsonDoc, jsonBuffer);

  client.publish(AWS_IOT_TOPIC, jsonBuffer);
}

void remotControl(){
  if (irrecv.decode(&results)){ // Returns 0 if no data ready, 1 if data ready.
    int value = results.value;// Results of decoding are stored in value 
    switch(value){     
       case 16753245: //keypad 1
        statusMode = 1;
        if(statusMode == 1){
          statusLampu = 1;
          digitalWrite(relayPin, LOW);  
        }
        Serial.println("1"); 
        break;
       case 16736925: //keypad 2
        statusMode = 2;
        Serial.println("2"); 
        break;
       case 16750695: //keypad 0
        if(statusMode == 1){
          statusLampu = 0;
          digitalWrite(relayPin, HIGH);
        }
        Serial.println("0"); 
        break;
    }
    Serial.print("Remote: ");
    Serial.println(value);
    irrecv.resume(); // Restart the ISR state machine and Receive the next value 
  }     
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  connectWifi();
  connectToAWS();
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH); 
  irrecv.enableIRIn(); // Start the receiver     
  SPI.begin();      
  mfrc522.PCD_Init();   
}

void loop() {
  // put your main code here, to run repeatedly:
  baca();
  //tulis();
  cahaya = analogRead(pinLdr);  //baca dari sensor LDR
  lux = 0.009768*cahaya+10;     //nilai LDR di konversi ke lux 
  if(akses == 1){
    remotControl();
    Serial.print("LUX: ");
    Serial.println(lux);
    if(statusMode == 2){
        if(lux >= 10 && lux < 14){
          digitalWrite(relayPin, LOW); 
          statusLampu = 1;
        }else{
          digitalWrite(relayPin, HIGH); 
          statusLampu = 0;
        }
    }
  }else{
    lux = 0;
    statusLampu = 0;
    statusMode = 0;
    digitalWrite(relayPin, HIGH); 
  }
  delay(1000);
  sendJsonToAWS();
  client.loop();
  delay(2000);
}

void tulis(){
  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print(F("Card UID:"));    //Dump UID
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.print(F(" PICC type: "));   // Dump PICC type
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));

  byte buffer[34];
  byte block;
  MFRC522::StatusCode status;
  byte len;

  Serial.setTimeout(20000L) ;     // wait until 20 seconds for input from serial
  // Ask personal data: name
  Serial.println(F("Type name, ending with #"));
  len = Serial.readBytesUntil('#', (char *) buffer, 30) ; // read name from serial
  for (byte i = len; i < 30; i++) buffer[i] = ' ';     // pad with spaces

  block = 1;
  //Serial.println(F("Authenticating using key A..."));
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("PCD_Authenticate() success: "));

  // Write block
  status = mfrc522.MIFARE_Write(block, buffer, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("MIFARE_Write() success: "));

  block = 2;
  //Serial.println(F("Authenticating using key A..."));
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Write block
  status = mfrc522.MIFARE_Write(block, &buffer[16], 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("MIFARE_Write() success: "));

  //pass: smarthome
  // Ask personal data: password
  Serial.println(F("Type password, ending with #"));
  len = Serial.readBytesUntil('#', (char *) buffer, 20) ; // read password from serial
  for (byte i = len; i < 20; i++) buffer[i] = ' ';     // pad with spaces

  block = 4;
  //Serial.println(F("Authenticating using key A..."));
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Write block
  status = mfrc522.MIFARE_Write(block, buffer, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("MIFARE_Write() success: "));

  block = 5;
  //Serial.println(F("Authenticating using key A..."));
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Write block
  status = mfrc522.MIFARE_Write(block, &buffer[16], 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  else Serial.println(F("MIFARE_Write() success: "));


  Serial.println(" ");
  mfrc522.PICC_HaltA(); // Halt PICC
  mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
}

void baca(){
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  byte block;
  byte len;
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  MFRC522::StatusCode status;

  Serial.println(F("**Card Detected:**"));
  //-------------------------------------------
  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); //dump some details about the card

  byte buffer1[18];
  block = 4;
  len = 18;

  //Ambil Password
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  
  code="";
  for (uint8_t i = 1; i < 16; i++)  {
     code+=(char)buffer1[i];
  }
  Serial.println("Password: "+code);
  
  //Ambil data nama user
  byte buffer2[18];
  block = 1;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid)); //line 834
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer2, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
 
  user="";
  for (uint8_t i = 0; i < 16; i++) {
    user+=(char)buffer2[i];
  }
  
  Serial.println("");
  Serial.println("Nama: "+user);
  //------------------------------------------------------------------------------------------------------------------------
  Serial.println(F("\n**End Reading**\n"));
  delay(1000); //change value if you want to read cards faster
  if(code=="smarthome      "){
    akses = 1;
    Serial.println("Sukses");
  }else{
    akses = 0;
    Serial.println("Gagal");
  }
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();  
}
