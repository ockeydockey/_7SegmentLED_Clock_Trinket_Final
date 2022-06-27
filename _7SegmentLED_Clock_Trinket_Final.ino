/**
  * Designed to work with ATTinyCore (Version 1.3.3):
  * https://github.com/SpenceKonde/ATTinyCore
  *
  * Board: "ATtiny25/45/85"
  * millis()/micros(): "Enabled"
  * Timer 1 Clock: "CPU"
  * Clock: "8 MHz (internal)"
  * Chip: "ATtiny85"
  * Save EEPROM: "EEPROM retained"
  * LTO (1.6.11+ only): "Enabled"
  * B.O.D. Level: "B.O.D. Enables (4.3v)"
  *
  * Programmer: "USBTinyISP"
  *
  * NOTE: Don't forget to "Burn Bootloader" to set fuses on CPU
*/
#include <TinyWireM.h>   // Version 1.0.1
#include <TinyRTClib.h>  // Version 1.0.0
#include <EEPROM.h>      // Version 2.0.0

#include <Adafruit_LEDBackpack.h>  // Version 1.1.6
#include <Adafruit_GFX.h>          // Version 1.7.5

#include "BinaryIIRFilter.h"
#include "MillisTimer.h"

#define DST_PIN 3
#define LIGHTSENSOR A2  // This is really A2 ;)

Adafruit_7segment matrix = Adafruit_7segment();
Adafruit_24bargraph bar = Adafruit_24bargraph();
Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

/// @brief Low Pass Filter for light sensor ADC
/// @details Data for this filter needs to be sampled at a consistent rate
BinaryIIRFilter lightSensorLPF{ 3U };

MillisTimer adcSampleTimer{ 20UL, true };
MillisTimer displayUpdateTimer{ 100UL, true };
MillisTimer dotsToggleTimer{ 500UL, true };
MillisTimer dateTimeUpdateTimer{ 1000UL, true };
MillisTimer barFrameTimer{ 100UL, true };
MillisTimer barBlankingTimer{ 1700UL, false };

RTC_DS1307 rtc;
DateTime now;
uint8_t hour;
uint8_t minute;
uint8_t dow;
uint8_t day;
uint8_t dots = 0x02;
uint8_t b = 0;
uint8_t brightness;
char dateChar1 = ' ';
char dateChar2 = ' ';
//byte dateToggle = 0;

void setup() {
  TinyWireM.begin();
  rtc.begin();
  matrix.begin(0x70);
  bar.begin(0x71);
  alpha4.begin(0x72);
  pinMode(DST_PIN, INPUT_PULLUP);
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

  adcSampleTimer.start();
  dotsToggleTimer.start();
  dateTimeUpdateTimer.start();
  barFrameTimer.start();
  displayUpdateTimer.start();
}

void loop() {
  // Sample light sensor and set screen brightness
  if (adcSampleTimer.process()) {
    // Check the light levels from the light sensor,
    // then set the screen birghtness to match
    brightness = map(constrain(lightSensorLPF.addSample(analogRead(LIGHTSENSOR)), 0, 780), 0, 780, 0, 16);
    bar.setBrightness(brightness);
  }

  // Larson scanner animation
  if (barFrameTimer.process()) {
    barBlankingTimer.process();
    if (barBlankingTimer.isRunning()) {
      bar.setBar(11, LED_OFF);
      bar.setBar(12, LED_OFF);
    } else {
      bar.setBar(23 - b, LED_OFF);
      bar.setBar(b, LED_OFF);

      b = (b + 1);
      if (b >= 12) {
        barBlankingTimer.start();
        b = 0;
      } else {
        bar.setBar(b, LED_YELLOW);
        bar.setBar(23 - b, LED_YELLOW);
      }
    }
  }

  // Inverts (flashes) the colon in the middle of the display
  if (dotsToggleTimer.process()) {
    dots ^= 0x02;

    matrix.writeDigitNum(0, (uint8_t)(hour / 10), false);
    matrix.writeDigitNum(1, (hour % 10), false);
    matrix.writeDigitRaw(2, dots);
    matrix.writeDigitNum(3, (uint8_t)(minute / 10), false);
    matrix.writeDigitNum(4, (minute % 10), false);
  }

  // Update date/time display
  if (dateTimeUpdateTimer.process()) {
    // Check the DST switch for "Daylight Saveing Time" == true
    if (digitalRead(DST_PIN) == HIGH) {
      // Add one hour to the current Date/Time
      now = DateTime(rtc.now().unixtime() + 3600);
      // Illuminate the "Daylight Savings Time" (DST) dot
      dots |= 0x0f;
    } else {
      now = rtc.now();
      // Deluminate the "Daylight Savings Time" (DST) dot
      dots &= ~(0x0f);
    }
    hour = now.hour();
    minute = now.minute();
    day = now.day();
    dow = now.dayOfWeek();

    if (hour >= 12) {
      // Illuminate the PM dot
      dots |= 0x08;
    } else {
      if (hour == 0) {
        hour = 12;
      }
      // Deluminate the PM dot
      dots &= ~(0x08);
    }

    // Make sure that you do this math AFTER the AM/PM calculations
    if (hour > 12) {
      hour -= 12;
    }

    matrix.writeDigitNum(0, (uint8_t)(hour / 10), false);
    matrix.writeDigitNum(1, (hour % 10), false);
    matrix.writeDigitRaw(2, dots);
    matrix.writeDigitNum(3, (uint8_t)(minute / 10), false);
    matrix.writeDigitNum(4, (minute % 10), false);


    //    dateToggle = ~ dateToggle;
    //    if (dateToggle == 0) {
    switch (dow) {
      // Sunday
      case 0:
        dateChar1 = 'S';
        dateChar2 = 'u';
        break;
      // Monday
      case 1:
        dateChar1 = 'M';
        dateChar2 = 'o';
        break;
      // Tuesday
      case 2:
        dateChar1 = 'T';
        dateChar2 = 'u';
        break;
      //Wednesday
      case 3:
        dateChar1 = 'W';
        dateChar2 = 'e';
        break;
      // Thursday
      case 4:
        dateChar1 = 'T';
        dateChar2 = 'h';
        break;
      // Friday
      case 5:
        dateChar1 = 'F';
        dateChar2 = 'r';
        break;
      // Saturday
      case 6:
        dateChar1 = 'S';
        dateChar2 = 'a';
        break;
      default:
        dateChar1 = '-';
        dateChar2 = '-';
        break;
    }

    // Possibly just show the numeric month with a "." in the middle

    //    } else {
    //      switch (now.month()) {
    //        case 1:
    //          // JAnuary
    //          dateChar1 = 'J';
    //          dateChar2 = 'A';
    //          break;
    //        case 2:
    //          // FeBruary
    //          dateChar1 = 'F';
    //          dateChar2 = 'B';
    //          break;
    //        case 3:
    //          // MaRch
    //          dateChar1 = 'M';
    //          dateChar2 = 'R';
    //          break;
    //        case 4:
    //          // APril
    //          dateChar1 = 'A';
    //          dateChar2 = 'P';
    //          break;
    //        case 5:
    //          // MaY
    //          dateChar1 = 'M';
    //          dateChar2 = 'Y';
    //          break;
    //        case 6:
    //          // JunE
    //          dateChar1 = 'J';
    //          dateChar2 = 'N';
    //          break;
    //        case 7:
    //          // JuLy
    //          dateChar1 = 'J';
    //          dateChar2 = 'L';
    //          break;
    //        case 8:
    //          // AUgust
    //          dateChar1 = 'A';
    //          dateChar2 = 'U';
    //          break;
    //        case 9:
    //          // SePtember
    //          dateChar1 = 'S';
    //          dateChar2 = 'P';
    //          break;
    //        case 10:
    //          // OCtober
    //          dateChar1 = 'O';
    //          dateChar2 = 'C';
    //          break;
    //        case 11:
    //          // NOvember
    //          dateChar1 = 'N';
    //          dateChar2 = 'O';
    //          break;
    //        case 12:
    //          // DeCember
    //          dateChar1 = 'D';
    //          dateChar2 = 'C';
    //          break;
    //        default:
    //          dateChar1 = '-';
    //          dateChar2 = '-';
    //          break;
    //      }
    //    }
  }

  // Only push data to the displays once every interval to prevent glitching
  if (displayUpdateTimer.process()) {
    alpha4.setBrightness(brightness / 3);
    matrix.setBrightness(brightness);

    // Only display date if the brightness is above a certain level
    if (brightness > 1) {
      alpha4.writeDigitAscii(0, dateChar1);
      alpha4.writeDigitAscii(1, dateChar2);

      if (day >= 10) {
        alpha4.writeDigitAscii(2, ((char)(day / 10)) | 0x30);
      } else {
        alpha4.writeDigitRaw(2, 0x00);
      }
      alpha4.writeDigitAscii(3, (day % 10) | 0x30);
    } else {
      alpha4.clear();
    }

    alpha4.writeDisplay();
    bar.writeDisplay();
    matrix.writeDisplay();
  }
}