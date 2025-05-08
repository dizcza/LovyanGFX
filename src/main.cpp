#include "Arduino.h"
#include "lgfx_user/LGFX_ESP32S3_M6_LCD.hpp"
// #include "lgfx_user/LGFX_ESP32S3_M6_TFT.hpp"

LGFX lcd;

void setup()
{
    Serial.begin(115200);
    delay(200);
    log_d("Starting...");
    if (lcd.begin()) {
        log_i("Success!");
    } else {
        log_e("begin FAILED");
    }
    // lcd.clear();
    // delay(1000);
}

void loop() {
    lcd.display();
    delay(100);
}

void loop2()
{
    static int r = 10;
    lcd.setColor(0xFF0000U); // Specify red as the drawing color
    for (int i = 10; i < 290; i++) {
        lcd.drawPixel(100, i);
    }
    lcd.drawCircle(100, 100, r);
    r += 10;
    if (r > 150) {
        r = 1;
    }
    delay(50);
    lcd.display();
}