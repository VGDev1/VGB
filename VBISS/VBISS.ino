#include <Adafruit_MAX31865.h> //Library för tempgivare
#include <LiquidCrystal.h>     // inkludera programmet för lcd displayn
#include <PID_v1.h>
#include <Wire.h>

LiquidCrystal lcd(9, 8, 7, 6, 5, 4);
Adafruit_MAX31865 max = Adafruit_MAX31865(10, 11, 12, 13); // Defines what pin the pt100 board uses
#define RREF 430.0                                         // The value of the Rref resistor. Use 430.0 for PT100 and 4300.0 for PT1000
#define RNOMINAL 100.0                                     // The 'nominal' 0-degrees-C resistance of the sensor, it is 100 for pt100

#define HEATER_RELAY_PIN 14
#define PROG_BTN_PIN 16
#define TRANSMIT_PIN 15

#define slaveAddress 9

// Relay
int heater = 14; // Relay ch1 onA1 but acting like digital output

// Define Variables we'll be connecting to
double Setpoint, Input, Output;

// Specify the links and initial tuning parameters
double Kp = 5, Ki = 3, Kd = 3;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

int WindowSize = 10000;
unsigned long windowStartTime;

// Calc variables
// Startbutton
long debounce = 200; // the debounce time, increase if the output flickers
// Time variables
float timeSinceStartMillis; // Veriable to use with millis to keep count on the sequences
float timeSinceStartMinutes;

float minuteInMillis = 60000;

// Stepper motor control
int setSpeed;
int currentSpeed = 0;

// Time values
int nbrOfSequences = seq;
int long times[] = tms;
int long temperatures[] = tmps;
int long speeds[] = spds;

void setup()
{

    pinMode(PROG_BTN_PIN, INPUT_PULLUP); // Inpin till startknappen

    pinMode(HEATER_RELAY_PIN, OUTPUT);
    digitalWrite(HEATER_RELAY_PIN, LOW);

    // initialize the variables we're linked to
    Setpoint = 0;

    // tell the PID to range between 0 and the full window size
    myPID.SetOutputLimits(0, WindowSize);

    // turn the PID on
    myPID.SetMode(AUTOMATIC);

    lcd.begin(16, 2); 

    pinMode(TRANSMIT_PIN, OUTPUT);

    max.begin(MAX31865_3WIRE);

    Wire.begin(); 
}

void heaterLoop()
{
    if (millis() - windowStartTime > WindowSize)
    {
        // time to shift the Relay Window
        windowStartTime += WindowSize;
    }
    if (Output < millis() - windowStartTime)
        digitalWrite(HEATER_RELAY_PIN, HIGH);
    else
        digitalWrite(HEATER_RELAY_PIN, LOW);
}

void sendSpeed()
{

    if (currentSpeed != setSpeed)
    {
        digitalWrite(TRANSMIT_PIN, HIGH);
        Wire.beginTransmission(slaveAddress); // transmit to device #9
        // map the speed to a value between 0 and 255
        speed = map(setSpeed, 0, 3000, 0, 255);
        Wire.write(speed);                 // sends x
        Wire.endTransmission();               // stop transmitting
        digitalWrite(TRANSMIT_PIN, LOW);
        currentSpeed = setSpeed;
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

void readTemp()
{
    uint16_t rtd = max.readRTD();
    float ratio = rtd;
    ratio /= 32768;
    Input = max.temperature(RNOMINAL, RREF);
}

void printInformation(minutesElapsed)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("S");
    lcd.print((int) setSpeed);
    lcd.print("C");
    lcd.print((int) Input);
    lcd.setCursor(0, 1);
    lcd.print("T");
    lcd.print((int) Setpoint);
    lcd.print("P");
    lcd.print((int) (minutesElapsed / times[nbrOfSequences]) * 100);
}

void loop()
{

    timeSinceStartMillis = millis(); // Counting millis to minuteInMilliss
    timeSinceStartMinutes = timeSinceStartMillis / minuteInMillis;

    int programmingMode = digitalRead(PROG_BTN_PIN); // progBtnReading the value of the button and setting progBtnReading

    if (programmingMode == HIGH)
    {
        software_Reset();
    }

    readTemp();

    int currentSequence = sequenceNumber(timeSinceStartMinutes);

    if (currentSequence == -1)
    {
        while (true)
        {
            digitalWrite(heater, 0);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Finished");
            delay(1000);
        }
    }

    Setpoint = temperatures[currentSequence];
    setSpeed = speeds[currentSequence];

    sendSpeed();

    if (timeSinceStartMillis % 1000 == 0)
    {
        printInformation(timeSinceStartMinutes);
    }

    myPID.Compute();
    heaterLoop();
}