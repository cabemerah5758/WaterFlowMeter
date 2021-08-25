#include <SPI.h>
#include <SD.h>
#include <EEPROM.h>
#include <Wire.h>
#include <DS3231.h>
#include <LCD_I2C.h>

File myFile;
RTClib myRTC;
DS3231 Clock;
const byte chipSelect = 10; //chip select sdcard modul
unsigned long previousMillis = 0, jeda, currentMillis;
const int interval = 1000;
const byte a = 7, b = 6, c = 5, d = 4, e = 3, f = 2; //a-e= Alokasi pin tombol, F= Alokasi pin Sensor
LCD_I2C lcd(0x27); //Alamat i2c LCD
float freq, nilai=0.00;
byte /*flagKartu = 0,*/ debounce = 0, waktu = EEPROM.read(500);
bool flagKartu = false,flagTombol = false;
byte /*flagTombol = 0,*/ menu = 0, tombol = 0, flagDelay = LOW, flagData = LOW;
byte urutData = 1;
int sensorBat = A0, batValue, r = 0, u = 0, k = 0,h = 0, o = 0;
volatile int detik = 0;
float konstanta = EEPROM.get(450,konstanta), nilaiAvg, rpm;
float satu,dua,tiga,empat,lima,enam,tujuh,delapan,sembilan,sepuluh;

void holder() { //debouncer tombol agar lancar dan tidak berulang
  delay(50);
  if (digitalRead(a) == 0 && debounce == 0) {
    debounce = 1;
  }
}
void hapusData() {
  nilaiAvg = 0;
  for (urutData = 0; urutData <= 10; urutData++) {
    k = urutData;
    tulisFloat();
    delay(10);
  }
  urutData = 1;
  k=0;
  nilaiAvg = 0.00;
}
void tampilan0() { // Tampilan Layar Menu Awal
  if (o == 0) {
    batValue = map(analogRead(sensorBat), 697, 860, 0, 100);
  }
  o--;
  lcd.setCursor(0, 0);
  lcd.print("HPS LHT UKUR ATR");
  lcd.setCursor(0, 1);
  lcd.print("K:");
  lcd.print(konstanta);
  lcd.setCursor(7, 1);
  lcd.print(batValue);
  lcd.print("% ");
  lcd.print("T:");
  lcd.print(waktu);
}
void tampilan1() { // Tampilan Layar Menu Hapus Data ?
  lcd.setCursor(2, 0);
  lcd.print("HAPUS DATA ?");
  lcd.setCursor(0, 1);
  lcd.print("TDK YA");
}
void tampilan2() { // Tampilan Layar Sedang Diproses
  lcd.setCursor(5, 0);
  lcd.print("SEDANG");
  lcd.setCursor(4, 1);
  lcd.print("DIPROSES");
}
void tampilan3() { // Tampilan Layar Proses Selesai
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("PROSES");
  lcd.setCursor(5, 1);
  lcd.print("SELESAI");
  delay(1000);
  lcd.clear();
}
void tampilan4() { // Tampilan Layar Data Ke...
  lcd.setCursor(0, 0);
  lcd.print("DATA KE: ");
  lcd.print(urutData);
  lcd.print("   ");
  lcd.setCursor(0, 1);
  k = urutData;
  bacaFloat();
  lcd.print(nilaiAvg, 3);
  //  nilaiAvg = 0;
  lcd.setCursor(7, 1);
  lcd.print("m/s");
}
void tampilan5() { // Tampilan Layar Kecepatan & Detik berjalan
  if(r == 1){
    rpm = nilaiAvg;
  }
  lcd.setCursor(0, 0);
  lcd.print("KCPTAN ");
  lcd.print(rpm, 3);
  lcd.setCursor(13, 0);
  lcd.print("m/s");
  lcd.setCursor(0, 1);
  lcd.print("WAKTU ");
  lcd.print(detik);
  lcd.print(" detik");
//  lcd.print(h);
}
void tampilan6() { // Tampilan Layar V-AVG & Data Ke...
  lcd.setCursor(0, 0);
  lcd.print("V-AVG: ");
  k = urutData;
  bacaFloat();
  lcd.print(nilaiAvg, 3);
  lcd.print(" m/s");
  lcd.setCursor(0, 1);
//  nilaiAvg = 0.00;
  if (urutData < 11) {
    lcd.print("DATA KE: ");
    lcd.print(urutData);
    delay(800);
  }
  else {
    lcd.print("  MEMORI PENUH");
    delay(800);
  }
}
void tampilan7() { // Tampilan Layar Menu Atur Konstanta
  lcd.setCursor(1, 0);
  lcd.print("ATUR KONSTANTA");
  lcd.setCursor(0, 1);
  lcd.print("K:");
  lcd.print(konstanta);
  lcd.print(" PERHATIAN");
}
void tampilan8() { // Tampilan Layar Atur Durasi Ukur
  lcd.setCursor(2, 0);
  lcd.print("DURASI UKUR");
  lcd.setCursor(0, 1);
  lcd.print("T :");
  lcd.print(waktu);
  lcd.print(" DETIK");
}
void tampilan9() { // Tampilan Layar Ekspor Ke Memory
  lcd.setCursor(0, 0);
  lcd.print("EKSPOR KE MEMORI");
  lcd.setCursor(0, 1);
  lcd.print("TDK YA");
}

void bacaTombol() { //
  if (digitalRead(e) == 0 && debounce == 0) {
    delay(50);
    debounce = 1;
    tombol = 1;
    flagTombol = true;
  }
  if (digitalRead(d) == 0 && debounce == 0) {
    delay(50);
    debounce = 1;
    tombol = 2;
    flagTombol = true;
  }
  if (digitalRead(c) == 0 && debounce == 0) {
    delay(50);
    debounce = 1;
    tombol = 3;
    flagTombol = true;
  }
  if (digitalRead(b) == 0 && debounce == 0) {
    delay(50);
    debounce = 1;
    tombol = 4;
    flagTombol = true;
  }
  if (digitalRead(a) == 0 && debounce == 0) {
    delay(50);
    debounce = 1;
    tombol = 5;
    flagTombol = true;
  }
}
void tick() { //Rutin waktu interupt sensor hall
  freq++;
}
void ekspor() { // rutin penyimpanan data ke memory
  lcd.clear();
  tampilan2();
  DateTime now = myRTC.now();
  myFile = SD.open("laporan.txt", FILE_WRITE);
  if (myFile) {
    myFile.print("Data Pembacaan. ");
    myFile.print("Tgl:");
    myFile.print(now.day(), DEC);
    myFile.print('/');
    myFile.print(now.month(), DEC);
    myFile.print('/');
    myFile.print(now.year(), DEC);
    myFile.print(". ");
    myFile.print("Jam:");
    if (now.hour() < 10) {
      myFile.print("0");
    }
    myFile.print(now.hour(), DEC);
    myFile.print(':');
    if (now.minute() < 10) {
      myFile.print("0");
    }
    myFile.print(now.minute(), DEC);
    myFile.print(':');
    if (now.second() < 10) {
      myFile.print("0");
    }
    myFile.println(now.second(), DEC);
    for (urutData = 1; urutData <= 10; urutData++) {
      myFile.print(urutData);
      myFile.print(". ");
      k = urutData;
      bacaFloat();
      myFile.print(nilaiAvg, 3);
      myFile.println(".");
      delay(100);
    }
    myFile.println(' ');
    // close the file:
    myFile.close();
  }
  urutData = 1;
}
void tulisFloat() { //rutin pemilihan menu
  switch (k) {
  case 1:
    satu = nilaiAvg;
    break;
  case 2:
    dua = nilaiAvg;
    break;
  case 3:
    tiga = nilaiAvg;
    break;
  case 4:
    empat = nilaiAvg;
    break;
  case 5:
    lima = nilaiAvg;
    break;
  case 6:
    enam = nilaiAvg;
    break;
  case 7:
    tujuh = nilaiAvg;
    break;
  case 8:
    delapan = nilaiAvg;
    break;
  case 9:
    sembilan = nilaiAvg;
    break;
  case 10:
    sepuluh = nilaiAvg;
    break;
  }
}
void bacaFloat() { //rutin pemilihan menu
  switch (k) {
  case 1:
    nilaiAvg = satu;
    break;
  case 2:
    nilaiAvg = dua;
    break;
  case 3:
    nilaiAvg = tiga;
    break;
  case 4:
    nilaiAvg = empat;
    break;
  case 5:
    nilaiAvg = lima;
    break;
  case 6:
    nilaiAvg = enam;
    break;
  case 7:
    nilaiAvg = tujuh;
    break;
  case 8:
    nilaiAvg = delapan;
    break;
  case 9:
    nilaiAvg = sembilan;
    break;
  case 10:
    nilaiAvg = sepuluh;
  break;
  }
}
void pilihMenu() { //rutin pemilihan menu
  switch (menu) {
  case 0:
    tampilan0();
    if (tombol == 1) {
      menu = 1;
      lcd.clear();
    }
    if (tombol == 2) {
      menu = 4;
      urutData = 1;
      lcd.clear();
    }
    if (tombol == 3) {
      menu = 5;
      lcd.clear();
    }
    if (tombol == 4) {
      menu = 7;
      lcd.clear();
    }
    if (tombol == 5) {
      //        menu = 0;
      lcd.clear();
    }
    flagTombol = false;
    detik = 0;
    r = 0;
    break;
  case 1:
    tampilan1();
    if (tombol == 1) {
      menu = 0;
      lcd.clear();
    }
    if (tombol == 2) {
      menu = 2;
      lcd.clear();
    }
    if (tombol == 5) {
      menu = 0;
      lcd.clear();
    }
    flagTombol = false;
    break;
  case 2:
    tampilan2();
    delay(1000);
    flagTombol = true;
//    hapusData();
    menu = 3;
    break;
  case 3:
    tampilan3();
    menu = 0;
    flagTombol = true; //non active
    break;
  case 4:
//    urutData = 1;
    tampilan4();
    if (urutData < 1) {
      urutData = 10;
      k = urutData;
      lcd.clear();
    }
    if (tombol == 1) {
      if (urutData > 1) {
        urutData--;
        k = urutData;
//        bacaFloat();
      }
    }
    if (tombol == 2) {
      if (urutData < 10) {
        urutData++;
        k = urutData;
//        bacaFloat();
      }
      if (urutData > 10) {
        urutData = 1;
        k = urutData;
//        bacaFloat();
      }
      lcd.clear();
    }
    if (tombol == 5) {
      menu = 0;
      urutData = 1;
      k = urutData;
//      bacaFloat();
    }
    flagTombol = false;
    break;
  case 5:
    if (flagDelay == LOW) {
      attachInterrupt(digitalPinToInterrupt(2), tick, FALLING);
      flagDelay = HIGH;
//      tampilan5();
    }
    if (flagDelay == HIGH) {
      currentMillis = millis();
      if ((currentMillis - previousMillis) < interval) {
        tampilan5();
      }
      if ((currentMillis - previousMillis) >= interval) {
        detachInterrupt(digitalPinToInterrupt(2));
        jeda = (currentMillis - previousMillis);
        freq = (freq / 4);// rumus untuk sensor 4 Magnet
        rpm = ((freq / jeda) * 60);
        rpm = rpm * konstanta;
        freq = 0;
//        tampilan5();
        lcd.clear();
        r++;
        detik++;
        flagDelay = LOW;
        previousMillis = currentMillis;
      }
      if (r != waktu) {
        if(r != 1){
          nilai = (nilai + rpm); //jumlahkan isi dataNilai untuk dicari rataratanya
          h++;
        }
      }
      if (r == waktu) {
        menu = 6;
//        k=0;
        r = 0;
        nilaiAvg = (nilai / h);
        nilai = 0; //setelah mendapatkan ratarata, nolkan nilai untuk hitungan selanjutnya
        h = 0;
        if (urutData <= 21 && flagData == LOW) {
          k = urutData;
          tulisFloat();
//          nilaiAvg = 0.00;
        }
        if (urutData > 11) {
          urutData = 11;
          flagData = HIGH;
        }
//        EEPROM.update(501, flagData);
      }
    }
    u = r;
    if (tombol == 5) {
      menu = 9;
      urutData = 1;
      k=0;
      lcd.clear();
    }
    flagTombol = true;
    break;
  case 6:
    lcd.clear();
    tampilan6();
    lcd.clear();
    if (urutData <= 11) {
      urutData++;
    }
    menu = 5;
    flagTombol = true;
    break;
  case 7:
    tampilan7();
    menu = 7;
    if (tombol == 1) {
      menu = 7;
      konstanta -= 1.0;
    }
    if (tombol == 2) {
      menu = 7;
      konstanta += 1.0;
    }
    if (tombol == 3) {
      menu = 7;
      konstanta -= 0.01;
    }
    if (tombol == 4) {
      menu = 7;
      konstanta += 0.01;
    }
    if (konstanta < 0.0) {
      konstanta = 0.000;
    }
    if (konstanta >= 10.00) {
      konstanta = 10;
    }
    if (tombol == 5) {
      menu = 8;
      EEPROM.put(450, konstanta);
      lcd.clear();
    }
    flagTombol = false;
    break;
  case 8:
    tampilan8();
    menu = 8;
    if (tombol == 1) {
      menu = 8;
      if (waktu > 10) {
        waktu = waktu - 10;
      }
    }
    if (tombol == 2) {
      menu = 8;
      if (waktu < 60) {
        waktu = waktu + 10;
      }
    }
    if (tombol == 5) {
      menu = 9;
      EEPROM.update(500, waktu);
      lcd.clear();
    }
    flagTombol = false;
    break;
  case 9:
    tampilan9();
    if (tombol == 1) {
      menu = 0;
      lcd.clear();
    }
    if (tombol == 2) {
      cekKartu();
      if (flagKartu == false) {
        menu = 0;
        lcd.clear();
      }
      if (flagKartu == true) {
        menu = 2;
        lcd.clear();
        ekspor();
      }
    }
    if (tombol == 5) {
      menu = 0;
      lcd.clear();
    }
    flagTombol = false;
    break;
  }
}
void cekKartu() { // rutin kesiapan kartu memory
  lcd.clear();
  lcd.print("Memeriksa Kartu");
  delay(1000);
  if (!SD.begin(chipSelect)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Kartu Rusak atau");
    lcd.setCursor(0, 1);
    lcd.print("Kartu Tidak Ada");
    flagKartu = false;
    delay(2000);
    lcd.clear();
  }
  else {
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("Kartu Siap");
    flagKartu = true;
    delay(2000);
    lcd.clear();
  }
}
void setup() {
  if(EEPROM.read(1023) != 0){
    for(int b = 0;b<1024;b++){
      EEPROM.write(b,0);
      delay(5);
    }

  }
  Wire.begin();
  pinMode(a, INPUT_PULLUP); // pin tombol 5 interupt esc & enter
  pinMode(b, INPUT_PULLUP);
  pinMode(c, INPUT_PULLUP);
  pinMode(d, INPUT_PULLUP);
  pinMode(e, INPUT_PULLUP);
  pinMode(f, INPUT_PULLUP); // pin hall effect sensor
  detik = 0;
  attachInterrupt(digitalPinToInterrupt(3), holder, FALLING);
  EEPROM.get(450, konstanta);
  DateTime now = myRTC.now();
  lcd.begin();
  lcd.backlight();
  delay(500);
  lcd.noBacklight();
  delay(500);
  lcd.backlight();
  lcd.setCursor(16, 0);
  lcd.print("Water Flow Meter");
  for(int i = 15; i>=0; i--){
    lcd.scrollDisplayLeft();
    delay(175);
  }
  delay(500);
  lcd.setCursor(20, 1);
  lcd.print("Ver:1(M4)");
  delay(1500);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Tgl: ");
  lcd.print(now.day(), DEC);
  lcd.print("/");
  lcd.print(now.month(), DEC);
  lcd.print("/");
  lcd.print(now.year(), DEC);
  lcd.setCursor(0, 1);
  lcd.print("Jam: ");
  if (now.hour() < 10) {
    myFile.print("0");
  }
  lcd.print(now.hour(), DEC);
  lcd.print(":");
  if (now.minute() < 10) {
    lcd.print("0");
  }
  lcd.print(now.minute(), DEC);
  lcd.print(":");
  if (now.second() < 10) {
    lcd.print("0");
  }
  lcd.print(now.second(), DEC);
  delay(6000);
  if(waktu > 100){
    waktu = 10;
  }
  cekKartu(); // cek kartu memory
}
void loop() {
  if (flagTombol == false) {
    bacaTombol();
    flagTombol = true;
  }
  pilihMenu();
  tombol = 0;
  if (digitalRead(a) == 1 && digitalRead(b) == 1 && digitalRead(c) == 1 && digitalRead(d) == 1 && digitalRead(e) == 1) {
    debounce = 0;
  }
  flagTombol = false;
}
