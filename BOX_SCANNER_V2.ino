#include <WiFi.h>
#include <HTTPClient.h>
#include <SoftwareSerial.h>
#include <HardwareSerial.h>
#include <ArduinoJson.h>
#include <Ultrasonic.h>
#include <LiquidCrystal_I2C.h>

HardwareSerial SerialBarcode(1);
#define RXBarcode 16 //6 - Rx
#define TXBarcode 17 //7 - Tx

HardwareSerial SerialPalang(1);
#define RXPalang 3
#define TXPalang 1 

//----------Ultrasonic----------
const int pinTRIGGER = 12;
const int pinECHO = 14;
long durasi, jarak;
long durasi2, jarak2;

//----------LCD----------
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

//----------Barcode----------
boolean stringComplete = false;
String databarcode="";
String barcode;

const char* ssid = "moonlight";
const char* password =  "catchthemoon";

void setup() {
  Serial.begin(9600);
  SerialPalang.begin(9600, SERIAL_8N1, RXPalang, TXPalang);     // PALANG
  SerialBarcode.begin(9600, SERIAL_8N1, RXBarcode, TXBarcode);  // BARCODE
  pinMode(pinTRIGGER, OUTPUT);                        // Pin Trigger HCSR
  pinMode(pinECHO, INPUT);                            // Pin ECHo HCSR
  lcd.init();                                         // initialize LCD                      
  lcd.backlight();                                    // turn on LCD backlight
  WiFi.begin(ssid, password);                         // Wi-Fi Begin
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
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
      lcd.print("AWAS HATI-HATI!");
      lcd.setCursor(0, 1);
      lcd.print("Menutup Palang..");
      delay(5000);
      SerialPalang.print('1');
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
      lcd.print("SILAHKAN MASUK");
      lcd.setCursor(0, 1);
      lcd.print("AREA PARKIR");
      delay(3000);
      palangtutup();
    }
}

void loop(){
 lcd.setCursor(0, 0);
 lcd.print("SILAHKAN SCAN");
 lcd.setCursor(0, 1);
 lcd.print("BARCODE ANDA :");
 delay(1000);
 lcd.clear();
 while (SerialBarcode.available()){
    HTTPClient http;
    
    //Scaning Kode Barcode
    char data_barcode = (char)SerialBarcode.read();
    databarcode += data_barcode;
    if(data_barcode == '\n'){
      stringComplete = true;
      }
      
    //Jika Scan Complete
    if (stringComplete) {
      barcode = databarcode.substring(0,10);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("ID PARKIR ANDA :");
      lcd.setCursor(0, 1);
      lcd.print(barcode);
      delay(2000);
      
      //getData (Http Get)
      String getData = "?id=" + barcode;      
      databarcode = "";
      stringComplete = false;
      
// mengirim data ke API , perhatikan parameter 
          
      http.begin("https://banyumanik.parkirsmg.com/Api/AuthGate" + getData);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Sending Request!");
      lcd.setCursor(0, 1);
      lcd.print("Please Waiting..");
      delay(1000);
    
      int httpCode = http.GET();                    //Make and send the request
    
      if (httpCode > 0) {                           //Check for the returning code
        String payload = http.getString();          //Get the response payload
        //Serial.print("HttpCode : ");                //Print HTTP return code
        //Serial.println(httpCode);                   //Print HTTP return code
        //Serial.println(payload);                    //Print request response payload

        // RESPON DARI PAYLOAD
        if(httpCode == 200){
          // Allocate JsonBuffer
          // Use arduinojson.org/assistant to compute the capacity.
          const size_t capacity = JSON_OBJECT_SIZE(2) + 50;
          DynamicJsonDocument doc(capacity);
  
          // Parse JSON object
          DeserializationError error = deserializeJson(doc, payload);
          if (error) {
            Serial.print("deserializeJson() failed with code");
            Serial.println(error.c_str());
            return;
            }
  
          // Decode JSON/Extract values
          int nilai = doc["status bayar"];
          //Serial.println(F("Response:"));
          //Serial.print("Status Bayar = ");
          //Serial.println(nilai);
          //Serial.println("");

          // PERINTAH KE PALANG
          //Serial.println("---PERINTAH KE PALANG---");
          if (nilai == 2){
            //Serial.println("BUKA GATE");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Transaksi Anda :");
            lcd.setCursor(0, 1);
            lcd.print("SELESAI");
            delay(2000);

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("TERIMA KASIH :)");
            lcd.setCursor(0, 1);
            lcd.print("Membuka Palang..");
            SerialPalang.print('0'); // Open Gate
            delay(3000);
            }
            
          else{
            //Serial.println("Silahkan Selesaikan Tagihan Parkir");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Transaksi Anda :");
            lcd.setCursor(0, 1);
            lcd.print("BELUM SELESAI");
            delay(5000);
            }
            
          }
          else {
            Serial.println("Error in response");
          }          
      }
      else {
        Serial.println("Error on HTTP request");
      }
      http.end();                                   //Free the resources      
     }
   }
   // Sensor Ultrasonic
   sensor();
}
