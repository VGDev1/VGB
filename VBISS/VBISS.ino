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

U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/U8X8_PIN_NONE);

const int numOutputs = 4;

// Define Variables we'll be connecting to
double Setpoint, Input, Output;

// Specify the links and initial tuning parameters
double Kp = ParKp, Ki = ParKi, Kd = ParKd;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

int WindowSize = 2000;
unsigned long windowStartTime;

int maxSpeed = 3000;

boolean heating = false;

unsigned long previousMillis = 0;
unsigned long interval = 1000; // Change word every 1 second

// Calc variables
// Startbutton
long debounce = 200; // the debounce time, increase if the output flickers
// Time variables
unsigned long timeSinceStartMillis; // Veriable to use with millis to keep count on the sequences
unsigned long timeSinceStartMinutes;

unsigned int minuteInMillis = 60000;

// Stepper motor control
int setSpeed;
int currentSpeed = 0;

// Time values
int nbrOfSequences = ParNbrOfSequences;
int long times[] = ParTimes;
int long temperatures[] = ParTemps;
int long speeds[] = ParSpeeds;

unsigned long heatingStartTime = 0;
unsigned long heatingTime = 0;
float startTemp = 0.0;
bool isHeating = false;

unsigned long lastTempUpdate = 0;
unsigned long lastMinuteMark = 0;

void setup()
{

  Serial.begin(9600);
  pinMode(PROG_BTN_PIN, INPUT_PULLUP); // Inpin till startknappen

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

  Wire.beginTransmission(slaveAddress); // transmit to device #9
  // map the speed to a value between 0 and 255
  Wire.write(0);          // sends x
  Wire.endTransmission(); // stop transmitting

  startTemp = sensors.getTempCByIndex(0);

  delay(1000);
}

void printToOled()
{
  static int currentOutput = 1;

  int remaining = times[nbrOfSequences - 1] - timeSinceStartMinutes;

  String outputString = "";

  // Display the corresponding output based on the current value
  switch (currentOutput)
  {
  case 1:
    outputString = (String) "Speed: " + setSpeed;
    break;
  case 2:
    outputString = (String) "Temp: " + static_cast<int>(Input) + (String) "c";
    break;
  case 3:
    outputString = (String) "Set temp: " + static_cast<int>(Setpoint) + (String) "c";
    break;
  case 4:
    outputString = (String) "Remaining:" + remaining + (String) "min";
    break;
  default:
    break;
  }

  Serial.println(outputString);
  if (heating)
    digitalWrite(HEATER_RELAY_PIN, LOW);
  u8x8.clear();
  u8x8.setCursor(0, 2);
  u8x8.print(outputString.c_str());
  if (heating)
    digitalWrite(HEATER_RELAY_PIN, HIGH);
  Serial.print("did not lockup on " + outputString + "\n");

  // Increment current output index
  currentOutput = (currentOutput % numOutputs) + 1;
}

void heaterLoop()
{
  if (millis() - windowStartTime > WindowSize)
  {
    // time to shift the Relay Window
    windowStartTime += WindowSize;
  }

  if (Output - 500 > millis() - windowStartTime)
  {
    if (!isHeating)
    {
      heatingStartTime = millis();
      isHeating = true;
    }
    digitalWrite(HEATER_RELAY_PIN, HIGH);
  }
  else
  {
    Serial.print("heater off");
    if (isHeating)
    {
      Serial.println("here ");
      heatingTime += millis() - heatingStartTime;
      isHeating = false;
    }
    digitalWrite(HEATER_RELAY_PIN, LOW);
  }
}

void checkHeaterIssue()
{
  Serial.println("Checking heater issue");
  Serial.println(heatingTime);
  Serial.println("Start temp: ");
  Serial.println(startTemp);
  if (heatingTime > 0.5 * minuteInMillis)
  {
    if (Input <= startTemp && Input > 5 && Input < 115)
    {
      while (true)
      {
        u8x8.clear();
        u8x8.setCursor(0, 2);
        u8x8.print("Heater Issue");
        digitalWrite(HEATER_RELAY_PIN, LOW); // Turn off the heater
        setSpeed = 0;
        sendSpeed();
        delay(1000);
      }
    }
  }
}

void sendSpeed()
{

  if (currentSpeed != setSpeed)
  {
    if (heating)
      digitalWrite(HEATER_RELAY_PIN, LOW);
    Wire.beginTransmission(slaveAddress); // transmit to device #9
    // map the speed to a value between 0 and 255
    int speed = map(setSpeed, 0, maxSpeed, 0, 255);
    Wire.write(speed);      // sends x
    Wire.endTransmission(); // stop transmitting
    currentSpeed = setSpeed;
    if (heating)
      digitalWrite(HEATER_RELAY_PIN, HIGH);
  }
}

int sequenceNumber(int time)
{
  for (int i = 0; i < nbrOfSequences; i++)
  {
    if (timeSinceStartMinutes < times[i])
    {
      return i;
    }
  }
  return -1;
}

void software_Reset() // Restarts program from beginning but does not reset the peripherals and registers
{
  asm volatile("  jmp 0");
}

void readTemp() {
  static float tempReadings[30];
  static int currentIndex = 0;
  static unsigned long lastTempUpdate = 0;

  unsigned long currentTime = millis();

  if (currentTime - lastTempUpdate >= 1000) {
    sensors.requestTemperatures();
    float currentTemp = sensors.getTempCByIndex(0);

    tempReadings[currentIndex] = currentTemp;
    currentIndex = (currentIndex + 1) % 30;

    bool isOutOfRange = true;
    for (int i = 0; i < 30; i++) {
      if (tempReadings[i] >= 10 && tempReadings[i] <= 115) {
        isOutOfRange = false;
        break;
      }
    }

    if (isOutOfRange) {
      // Temperature consistently out of range for 30 seconds
      while (true) {
        u8x8.clear();
        u8x8.setCursor(0, 2);
        u8x8.print("Temp Sensor Fault");
        digitalWrite(HEATER_RELAY_PIN, LOW); // Turn off the heater
        setSpeed = 0;
        sendSpeed();
        delay(1000);
      }
    }

    lastTempUpdate = currentTime;
  }

  sensors.requestTemperatures();
  Input = sensors.getTempCByIndex(0);

  if (currentTime - lastMinuteMark >= minuteInMillis)
  {
    startTemp = Input;
    heatingTime = 0;

    lastMinuteMark = currentTime;
  }

  checkHeaterIssue();
}

void finished()
{
  Setpoint = 0;
  setSpeed = 0;
  sendSpeed();
  digitalWrite(HEATER_RELAY_PIN, LOW);
  u8x8.clear();
  u8x8.setCursor(0, 2);
  u8x8.print("Finished");
  delay(1000);
}

void loop()
{
  timeSinceStartMillis = millis(); // Counting millis to minuteInMilliss

  Serial.println("timeSinceStartMillis");
  Serial.print(timeSinceStartMillis);

  timeSinceStartMinutes = timeSinceStartMillis / minuteInMillis;

  if (digitalRead(PROG_BTN_PIN) == LOW)
  {
    software_Reset();
  }

  readTemp();

  int currentSequence = sequenceNumber(timeSinceStartMinutes);

  if (currentSequence == -1)
  {
    while (true)
    {
      finished();
    }
  }

  Setpoint = temperatures[currentSequence];
  setSpeed = speeds[currentSequence];
  sendSpeed();

  if (Input > 5 && Input < 115)
  { // check if temp sensor is connected and working
    if (timeSinceStartMillis - previousMillis >= interval)
    {
      previousMillis = timeSinceStartMillis;
      printToOled();
    }
    myPID.Compute();
    heaterLoop();
  }
}