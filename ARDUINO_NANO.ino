#include <EEPROM.h>

#define P_stat digitalRead(4)==LOW
#define P_stop digitalRead(5)==LOW
#define P_emg digitalRead(6)==HIGH
int eep;
String inputString;
boolean stringComplete, stat = false;
boolean statuss;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  //pinMode(2, INPUT_PULLUP);
  //pinMode(3, INPUT_PULLUP);
  attachInterrupt(0, limitMaju, FALLING);
  attachInterrupt(1, limitMundur, FALLING);
  Serial.println(eep);
  mati();
}
void mati() {
  digitalWrite(A0, HIGH);
  digitalWrite(A1, HIGH);
}

void buka() {
  digitalWrite(A0, LOW);
  digitalWrite(A1, HIGH);
  statuss = 0;
  Serial.println("membuka...");
}

void nutup() {
  digitalWrite(A0, HIGH);
  digitalWrite(A1, LOW);
  statuss = 1;
  Serial.println("menutup...");
}
void limitMaju() {
  if (statuss == 0) {
    mati();
    Serial.println("okMaju");
    stat = false;
  }
}

void limitMundur() {
  if (statuss == 1) {
    mati();
    Serial.println("okMundur");
    stat = false;
  }
}

void baca_serial() {
  while (Serial.available()) {
    stat = true;
    char inChar = (char)Serial.read();
    Serial.println(inChar);
    if (inChar == '1') {
      digitalWrite(11, HIGH);
      stringComplete = false;
    }
    else {
      digitalWrite(11, LOW);
      stringComplete = true;
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  baca_serial();
  if ((stat == true && stringComplete == false) || P_stop ) {
    nutup();
    delay(500);
  }
  if ((stat == true && stringComplete == true) || P_stat) {
    buka();
    delay(500);
  }
  //if((!P_stat && stringComplete == false)||(!P_stat && P_stop && digitalRead(2)== LOW)){buka();}
  //if((!P_stat && stringComplete == true)||(!P_stop && P_stat && digitalRead(3)== LOW)){nutup();}
  if (!P_emg) {
    mati();
  }
}
