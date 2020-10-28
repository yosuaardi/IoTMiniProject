const int ledPinMerah =  27;// pin LED merah
const int ledPinKuning =  26;// pin LED kuning
const int ledPinHijau =  25;// pin LED hijau

int ledState1 = LOW;  //state lampu mati
int ledState2 = HIGH; //state lampu hidup 

unsigned long previousMillis = 0;   //menyimpan perubahan waktu

long interval = 6000;           //interval waktu untuk LED
int counter = 0;                //menyimpan nilai counter yang akan menjadi parameter LED yang hidup

void setup() {
  pinMode(ledPinMerah, OUTPUT);
  pinMode(ledPinKuning, OUTPUT);
  pinMode(ledPinHijau, OUTPUT);
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    //menyimpan waktu terakhir kali LED menyala
    previousMillis = currentMillis;
    
    if(counter%3 == 0){ //LED merah menyala
      digitalWrite(ledPinMerah, ledState2);
      digitalWrite(ledPinHijau, ledState1);
      digitalWrite(ledPinKuning, ledState1);
      interval = 6000;
    }else if(counter%3 == 1){ //LED hijau menyala
      digitalWrite(ledPinMerah, ledState1);
      digitalWrite(ledPinHijau, ledState2);
      digitalWrite(ledPinKuning, ledState1);
      interval = 6000;
    }else if(counter%3 == 2){ //LED kuning menyala
      digitalWrite(ledPinMerah, ledState1);
      digitalWrite(ledPinHijau, ledState1);
      digitalWrite(ledPinKuning, ledState2);
      interval = 3000;
    }
    counter++;
  }
}
