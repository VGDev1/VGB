#include <PID_v1.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#define HEATER_RELAY_PIN 11
#define PROG_BTN_PIN 8

#define slaveAddress 9

#define ONE_WIRE_BUS 10

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

const int numOutputs = 4;

// Define Variables we'll be connecting to
double Setpoint, Input, Output;

// Specify the links and initial tuning parameters
double Kp = 80, Ki = 10, Kd = 450;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

int WindowSize = 2000;
unsigned long windowStartTime;

int maxSpeed = 3000;

boolean heating = false;

unsigned long previousMillis = 0;
unsigned long interval = 5000;  // Change word every 1 second

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
int nbrOfSequences = 3;
int long times[] = { 2, 3,4 };
int long temperatures[] = { 60, 70, 80 };
int long speeds[] = { 200, 500, 800 };

void setup() {

  Serial.begin(9600);
  pinMode(PROG_BTN_PIN, INPUT_PULLUP);  // Inpin till startknappen

  pinMode(HEATER_RELAY_PIN, OUTPUT);
  digitalWrite(HEATER_RELAY_PIN, LOW);

  // initialize the variables we're linked to
  Setpoint = 0;

  u8x8.begin();
  u8x8.setPowerSave(0);

  u8x8.setFont(u8x8_font_7x14B_1x2_f);
  u8x8.setCursor(0, 2);
  u8x8.print("Data mode");


  // tell the PID to range between 0 and the full window size
  myPID.SetOutputLimits(0, WindowSize);

  // turn the PID on
  myPID.SetMode(AUTOMATIC);

  sensors.begin();

  Wire.begin();
}

void printToOled() {
  static int currentOutput = 1;

  int remaining = times[nbrOfSequences-1] - timeSinceStartMinutes;
  // log setpoint, Input, setSpeed, remaning and print in serial monitor
  Serial.println("log");
  Serial.print("Output: ");
  Serial.print(Output);
  Serial.print("Setpoint: ");
  Serial.print(static_cast<int>(Setpoint));
  Serial.print(" Input: ");
  Serial.print(static_cast<int>(Input));
  Serial.print(" SetSpeed: ");
  Serial.print(setSpeed);
  Serial.print(" Remaining: ");
  Serial.println(remaining);
  Serial.println("currentOutput: ");
  Serial.println(currentOutput);

  
                         // Calculate the y-coordinate for centering the text

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
      outputString = (String) "Remaining:" + remaining + (String) "min";
      break;
    default:
      break;
  }

  Serial.println(outputString);
  // Display the output
  u8x8.clear();
  u8x8.setCursor(0, 2);
  if(heating) digitalWrite(HEATER_RELAY_PIN, LOW);
  u8x8.print(outputString.c_str());
  if(heating) digitalWrite(HEATER_RELAY_PIN, HIGH);
  Serial.print("did not lockup on " + outputString + "\n");

  // Increment current output index
  currentOutput = (currentOutput % numOutputs) + 1;
}

void heaterLoop() {
  if (millis() - windowStartTime > WindowSize) {
    // time to shift the Relay Window
    windowStartTime += WindowSize;
  }
  if (Output > millis() - windowStartTime) {
        digitalWrite(HEATER_RELAY_PIN, HIGH);
        heating = true;
  }
  else digitalWrite(HEATER_RELAY_PIN, LOW);
  heating = false;
}

void sendSpeed() {

  if (currentSpeed != setSpeed) {
    if(heating) digitalWrite(HEATER_RELAY_PIN, LOW);
    Wire.beginTransmission(slaveAddress);  // transmit to device #9
    // map the speed to a value between 0 and 255
    int speed = map(setSpeed, 0, maxSpeed, 0, 255);
    Wire.write(speed);       // sends x
    Wire.endTransmission();  // stop transmitting
    currentSpeed = setSpeed;
    if(heating) digitalWrite(HEATER_RELAY_PIN, HIGH);
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
      String outputString = "Finished";
      u8x8.clear();
      u8x8.setCursor(0, 2);
      u8x8.print(outputString.c_str());

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

  if(Input > 5) {
      if (timeSinceStartMillis - previousMillis >= interval) {
    previousMillis = timeSinceStartMillis;
    printToOled();
    Serial.println("second");
  }

  myPID.Compute();
  heaterLoop();
  Serial.println("running");
  }
}