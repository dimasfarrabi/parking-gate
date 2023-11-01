#include <WiFi.h>
#include <HTTPClient.h>
#include <SoftwareSerial.h>
#include <HardwareSerial.h>  
#include <Ultrasonic.h>
#include <LiquidCrystal_I2C.h>
#include "Adafruit_Thermal.h"
#include <PString.h>
#include <Wire.h>
#include "RTClib.h"
RTC_DS1307 RTC;

HardwareSerial SerialPalang(1);
#define RXPalang 16
#define TXPalang 17

SoftwareSerial mySerial(25, 26);  //RX, TX
Adafruit_Thermal printer(&mySerial);
int hitung=0;

const int pinTRIGGER = 12;
const int pinECHO = 14;
long durasi, jarak;
long durasi2, jarak2;

int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};

Button button1 = {13, 0, false};

void IRAM_ATTR isr() {
  button1.numberKeyPresses += 1;
  button1.pressed = true;
}  

const char* ssid = "MST-III-HALL-TIMUR";
const char* password = "wifiharam";

void setup() {
  lcd.init();
  lcd.backlight();
  Serial.begin(115200);
  SerialPalang.begin(9600, SERIAL_8N1, RXPalang, TXPalang); // PALANG
  mySerial.begin(9600); // PRINTER
  printer.begin();
  Wire.begin();
  RTC.begin();
  if (! RTC.isrunning()) {
      Serial.println("RTC is NOT running!");  
      RTC.adjust(DateTime(__DATE__, __TIME__)); 
  }
  pinMode(button1.PIN, INPUT_PULLUP);
  attachInterrupt(button1.PIN, isr, FALLING);
  pinMode(pinTRIGGER, OUTPUT);
  pinMode(pinECHO, INPUT);
  
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void palangtutup()  {
  digitalWrite(pinTRIGGER, LOW);
  delayMicroseconds(5);
  digitalWrite(pinTRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTRIGGER, LOW);
        
  durasi2 = pulseIn(pinECHO, HIGH);
  jarak2 = (durasi2 * 0.034) / 2;
    
  if (jarak2 >= 160) {
      //Serial.println("TUTUP GATE");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("AWAS HATI-HATI");
      lcd.setCursor(0, 1);
      lcd.print("MENUTUP PALANG");
      delay(2000);
      SerialPalang.print('0');
      delay(1000);
      }
}

void sensor(){
    //Sensor Ultrasonic Awal
    digitalWrite(pinTRIGGER, LOW);
    delayMicroseconds(5);
    digitalWrite(pinTRIGGER, HIGH);
    delayMicroseconds(10);
    digitalWrite(pinTRIGGER, LOW);
        
    durasi = pulseIn(pinECHO, HIGH);
    jarak = (durasi * 0.034) / 2;

    if (jarak <= 80) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("SEDANG MASUK");
      lcd.setCursor(0, 1);
      lcd.print("AREA PARKIR");
      delay(3000);
      palangtutup();
    }
}

void loop(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SELAMAT DATANG");
  lcd.setCursor(0,1);
  lcd.print("PARKIR POLINES");
  delay(500);
      
// Mengirim data ke API . perhatikan alamat/ URL API + parameter yang dikirim       
  while (button1.pressed) {
    HTTPClient http;
    http.begin("https://banyumanik.parkirsmg.com/Api/AuthGate");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("MENCETAK KARCIS");
    lcd.setCursor(0,1);
    lcd.print("MOHON DITUNGGU");
    delay(1000);
    
    printer.wake();       
    printer.setDefault();
    // Layout Print Karcis
    printer.boldOn();
    printer.justify('C');
    printer.setSize('M');
    printer.println(F("TIKET PARKIR"));
    printer.setSize('S');
    printer.println(F("PARKIRAN ELEKTRO"));
    printer.println(F("POLINES"));
    printer.justify('C');
    printer.setSize('S');
    
   // Operasi ID PARKIR
    //hitung+=1;
    char buffer[40];
    PString str1(buffer, sizeof(buffer));
    str1.print(random(1000000000,10000000000));
    printer.print("ID PARKIR : ");
    printer.println(str1);
    
    String httpRequestData1 = "value1="+String(str1);

    // BARCODE
    printer.justify('C');
    printer.setSize('M');
    printer.print(F("BARCODE:"));
    printer.printBarcode(str1, CODE93);
        
    // Penggunaan RTC (DATE & TIME)
    printer.justify('C');
    printer.setSize('S');
    // Operasi Tanggal
    DateTime now = RTC.now(); 
    PString str2(buffer, sizeof(buffer));
    str2.begin();
    str2.print(now.year(), DEC);
    str2.print('-');
    str2.print(now.month(), DEC);
    str2.print('-');
    str2.print(now.day(), DEC);
    str2.print(' ');
    
    int z=0;
    z=now.hour(), DEC;
    if(z<10)str2.print('0');
      str2.print(z);
      str2.print(':');
    z=now.minute(), DEC;
    if(z<10)str2.print('0');
      str2.print(now.minute(), DEC);
      str2.print(':');
    z=now.second(), DEC;
    if(z<10)str2.print('0');
      str2.print(now.second(), DEC);
    printer.print("MASUK : ");
    printer.println(str2);

    String httpRequestData2 = "&value2="+String(str2);
    
    // ------LAYOUT KARCIS-------
    printer.justify('C');
    printer.setSize('S');
    printer.println("KUNJUNGI WEBSITE");
    printer.println("UNTUK PEMBAYARAN :");
    printer.println("banyumanik.parkirsmg.com");
    printer.setSize('M');
    printer.println(F("PERINGATAN!"));
    printer.setSize('S');
    printer.println("SIMPAN KARCIS.");
    printer.print("JANGAN HILANGKAN!!!");
    printer.feed(4);
    printer.sleep();

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("SILAKAN AMBIL");
    lcd.setCursor(0,1);
    lcd.print("KARCIS ANDA");
    delay(1000);

    Serial.print("httpRequestData: ");
    Serial.println(httpRequestData1 + httpRequestData2);

    int httpResponseCode = http.POST(httpRequestData1 + httpRequestData2);
    
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("BERHASIL KIRIM");
      lcd.setCursor(0,1);
      lcd.print("KE DATABASE");
      delay(1000);
      //Serial.println("MEMBUKA PALANG"); 
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("MEMBUKA PALANG");
      lcd.setCursor(0,1);
      lcd.print("TERIMAKASIH");
      SerialPalang.print('1');
      delay(1000);
      }
      
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("GAGAL KIRIM");
      lcd.setCursor(0,1);
      lcd.print("KE DATABASE");
      delay(1000);
      //Serial.println("MEMBUKA PALANG"); 
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("SILAKAN, TEKAN");
      lcd.setCursor(0,1);
      lcd.print("TOMBOL LAGI");
      delay(1000);
      }
      button1.pressed = false;
   }
   sensor();
}
