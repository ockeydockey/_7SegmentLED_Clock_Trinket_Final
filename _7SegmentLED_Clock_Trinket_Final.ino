#include <TinyWireM.h>
#include <TinyRTClib.h>
#include <EEPROM.h>

#include <Adafruit_LEDBackpack.h>
#include <Adafruit_GFX.h>

#define DST_PIN 4
#define LIGHTSENSOR 3

Adafruit_7segment matrix = Adafruit_7segment();
Adafruit_24bargraph bar = Adafruit_24bargraph();
Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

RTC_DS1307 rtc;
DateTime now;
uint8_t hour;
byte dots = 0x02;
byte b = 0;
unsigned int metranome = 0;
char dateChar1;
char dateChar2;

void setup() {
  TinyWireM.begin();
  rtc.begin();
  matrix.begin(0x70);
  bar.begin(0x71);
  alpha4.begin(0x72);
  pinMode(DST_PIN, INPUT);
  pinMode(LIGHTSENSOR, INPUT);

  // following line sets the RTC to the date & time this sketch was compiled
  now = DateTime(__DATE__, __TIME__);
  int32_t build = 0;
  EEPROM.get(0, build);
  if (now.unixtime() != build) {
    if (digitalRead(DST_PIN) == HIGH) {
      rtc.adjust(now.unixtime() - 3600);
    } else {
      rtc.adjust(now.unixtime());
    }
    EEPROM.put(0, now.unixtime());
  }
}

void loop() {
  bar.setBrightness(analogRead(LIGHTSENSOR) / 64);
  if ((metranome % 20) < 12) {
    bar.setBar(23 - b, LED_OFF);
    bar.setBar(b, LED_OFF);
    bar.writeDisplay();

    b = (b + 1) % 12;

    bar.setBar(b, LED_YELLOW);
    bar.setBar(23 - b, LED_YELLOW);
    bar.writeDisplay();
  } else {
    b = 11;
    bar.setBar(11, LED_OFF);
    bar.setBar(12, LED_OFF);
    bar.writeDisplay();
  }


  if (metranome % 5 == 0) {
    // Inverts (flashes) the colon in the middle of the display
    dots ^= 0x02;

    matrix.writeDigitNum(0, (int)(hour / 10), false);
    matrix.writeDigitNum(1, (hour % 10), false);
    matrix.writeDigitRaw(2, dots);
    matrix.writeDigitNum(3, (int)(now.minute() / 10), false);
    matrix.writeDigitNum(4, (now.minute() % 10), false);
    matrix.writeDisplay();

    matrix.setBrightness(analogRead(LIGHTSENSOR) / 64);
    delayMicroseconds(100);
  }

  if (metranome % 10 == 0) {
    now = rtc.now();
    //    hour = (now.hour() + 18) % 24;
    hour = (now.hour()) % 24;

    if (digitalRead(DST_PIN) == HIGH) {
      hour++;
    }

    if (hour >= 12) {
      dots |= 0x08;
    } else {
      if (hour == 0) {
        hour = 12;
      }
      dots &= ~(0x08);
    }

    // Make sure that you do this math AFTER the AM/PM calculations
    if (hour > 12) {
      hour -= 12;
    }

    matrix.writeDigitNum(0, (int)(hour / 10), false);
    matrix.writeDigitNum(1, (hour % 10), false);
    matrix.writeDigitRaw(2, dots);
    matrix.writeDigitNum(3, (int)(now.minute() / 10), false);
    matrix.writeDigitNum(4, (now.minute() % 10), false);

    matrix.writeDisplay();

    switch (now.month()) {
      case 1:
        // JAnuary
        dateChar1 = 'J';
        dateChar2 = 'A';
        break;
      case 2:
        // FeBruary
        dateChar1 = 'F';
        dateChar2 = 'B';
        break;
      case 3:
        // MaRch
        dateChar1 = 'M';
        dateChar2 = 'R';
        break;
      case 4:
        // APril
        dateChar1 = 'A';
        dateChar2 = 'P';
        break;
      case 5:
        // MaY
        dateChar1 = 'M';
        dateChar2 = 'Y';
        break;
      case 6:
        // JunE
        dateChar1 = 'J';
        dateChar2 = 'N';
        break;
      case 7:
        // JuLy
        dateChar1 = 'J';
        dateChar2 = 'L';
        break;
      case 8:
        // AUgust
        dateChar1 = 'A';
        dateChar2 = 'U';
        break;
      case 9:
        // SePtember
        dateChar1 = 'S';
        dateChar2 = 'P';
        break;
      case 10:
        // OCtober
        dateChar1 = 'O';
        dateChar2 = 'C';
        break;
      case 11:
        // NOvember
        dateChar1 = 'N';
        dateChar2 = 'O';
        break;
      case 12:
        // DeCember
        dateChar1 = 'D';
        dateChar2 = 'C';
        break;
      default:
        dateChar1 = '-';
        dateChar2 = '-';
        break;
    }
    alpha4.writeDigitAscii(0, dateChar1);
    alpha4.writeDigitAscii(1, dateChar2);
    
    if (now.day() >= 10) {
      alpha4.writeDigitAscii(2, ((char)(now.day() / 10)) | 0x30);
    } else {
      alpha4.writeDigitRaw(2, 0x20);
    }
    alpha4.writeDigitAscii(3, (now.day() % 10) | 0x30);
    alpha4.writeDisplay();
  }

  metranome++;
  metranome = metranome % 60;
  delay(100);
}
