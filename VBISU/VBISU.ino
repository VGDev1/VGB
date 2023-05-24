#include <PID_v1.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

#define HEATER_RELAY_PIN 11
#define PROG_BTN_PIN 8

#define slaveAddress 9

#define ONE_WIRE_BUS 10

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int numOutputs = 4;

// Define Variables we'll be connecting to
double Setpoint, Input, Output;

// Specify the links and initial tuning parameters
double Kp = 80, Ki = ParKi, Kd = 450;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

int WindowSize = 2000;
unsigned long windowStartTime;

int maxSpeed = 3000;

unsigned long previousMillis = 0;
unsigned long interval = 1000;  // Change word every 1 second

// Calc variables
// Startbutton
long debounce = 200;  // the debounce time, increase if the output flickers
// Time variables
unsigned long timeSinceStartMillis;  // Veriable to use with millis to keep count on the sequences
unsigned long timeSinceStartMinutes;

unsigned int minuteInMillis = 60000;

// Stepper motor control
int setSpeed;
int currentSpeed = 0;

// Time values
int nbrOfSequences = 8;
int long times[] = {20, 30, 40, 50, 80, 120, 180, 300};
int long temperatures[] = {95, 95, 95, 90, 90, 85, 80, 75};
int long speeds[] = {0, 200, 300, 400, 400, 510, 510, 510};

void setup() {

  Serial.begin(9600);
  pinMode(PROG_BTN_PIN, INPUT_PULLUP);  // Inpin till startknappen

  pinMode(HEATER_RELAY_PIN, OUTPUT);
  digitalWrite(HEATER_RELAY_PIN, LOW);

  // initialize the variables we're linked to
  Setpoint = 0;

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // Initialize with the I2C address of your display
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);  // Increase text size
  display.setCursor(0, 0);
  display.println("Data mode");
  display.display();
  delay(1000);

  // tell the PID to range between 0 and the full window size
  myPID.SetOutputLimits(0, WindowSize);

  // turn the PID on
  myPID.SetMode(AUTOMATIC);

  sensors.begin();

  Wire.begin();
}

void setCursor(int wordLength) {
  int16_t x = (SCREEN_WIDTH - (wordLength * 12)) / 2;  // Calculate the x-coordinate for centering the text
  int16_t y = (SCREEN_HEIGHT - 16) / 2;                // Calculate the y-coordinate for centering the text

  display.setCursor(0, 0);
}

void printToOled() {
  static int currentOutput = 1;
  display.clearDisplay();                           // Calculate the y-coordinate for centering the text

  String outputString = "";

  // Display the corresponding output based on the current value
  switch (currentOutput) {
    case 1:
      outputString = (String) "Speed: " + setSpeed;
      break;
    case 2:
      outputString = (String) "Temp: " + static_cast<int>(Input) + (String) "c";
      break;
    case 3:
      outputString =  (String) "Set temp: " + static_cast<int>(Setpoint) + (String) "c";
      break;
    case 4:
      int remaining = times[nbrOfSequences-1] - timeSinceStartMinutes;
      outputString = (String) "Remaining:" + remaining + (String) "min";
      break;
    default:
      break;
  }

  // Display the output
  display.setCursor(0, 0);
  display.println(outputString);
  display.display();

  // Increment current output index
  currentOutput = (currentOutput % numOutputs) + 1;
}

void heaterLoop() {
  if (millis() - windowStartTime > WindowSize) {
    // time to shift the Relay Window
    windowStartTime += WindowSize;
  }
  if (Output > millis() - windowStartTime) digitalWrite(HEATER_RELAY_PIN, HIGH);
  else digitalWrite(HEATER_RELAY_PIN, LOW);
}

void sendSpeed() {

  if (currentSpeed != setSpeed) {
    Wire.beginTransmission(slaveAddress);  // transmit to device #9
    // map the speed to a value between 0 and 255
    int speed = map(setSpeed, 0, maxSpeed, 0, 255);
    Wire.write(speed);       // sends x
    Wire.endTransmission();  // stop transmitting
    currentSpeed = setSpeed;
  }
}

int sequenceNumber(int time) {
  for (int i = 0; i < nbrOfSequences; i++) {
    if (timeSinceStartMinutes < times[i]) {
      return i;
    }
  }
  return -1;
}

void software_Reset()  // Restarts program from beginning but does not reset the peripherals and registers
{
  asm volatile("  jmp 0");
}

void readTemp() {
  sensors.requestTemperatures();
  Input = sensors.getTempCByIndex(0);
}

void finished() {
      Serial.print("done");
      Setpoint = 0;
      setSpeed = 0;
      sendSpeed();
      digitalWrite(HEATER_RELAY_PIN, LOW);
      display.clearDisplay();
        String outputString = "Finished";
        setCursor(outputString.length());
        display.println(outputString);
        display.display();

      delay(1000);
}


void loop() {
  timeSinceStartMillis = millis();  // Counting millis to minuteInMilliss

  Serial.println("timeSinceStartMillis");
  Serial.print(timeSinceStartMillis);

  timeSinceStartMinutes = timeSinceStartMillis / minuteInMillis;


  if (digitalRead(PROG_BTN_PIN) == LOW) {
    software_Reset();
  }

  readTemp();

  int currentSequence = sequenceNumber(timeSinceStartMinutes);

  if (currentSequence == -1) {
    while (true) {
      finished();
    }
  }

  Setpoint = temperatures[currentSequence];
  setSpeed = speeds[currentSequence];
  sendSpeed();

  Serial.println(setSpeed);
  Serial.println(timeSinceStartMinutes);

  Serial.println("sequence");
  Serial.println(currentSequence);


  Serial.println("Input");
  Serial.println(Input);

  Serial.println("Output");
  Serial.println(Output);

  if (timeSinceStartMillis - previousMillis >= interval) {
    previousMillis = timeSinceStartMillis;
    printToOled();
    Serial.println("second");
  }

  myPID.Compute();
  heaterLoop();
  Serial.println("running");
}