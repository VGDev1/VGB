#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int numWords = 5;
const String words[numWords] = {"Hello", "World", "Arduino", "Display", "Example"};
int currentWord = 0;

unsigned long previousMillis = 0;
unsigned long interval = 1000; // Change word every 1 second

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2); // Increase text size
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    display.clearDisplay();

    int16_t x = (SCREEN_WIDTH - (words[currentWord].length() * 12)) / 2; // Calculate the x-coordinate for centering the text
    int16_t y = (SCREEN_HEIGHT - 16) / 2; // Calculate the y-coordinate for centering the text

    display.setCursor(x, y);
    display.println(words[currentWord]);
    display.display();

    currentWord = (currentWord + 1) % numWords;
  }
}
