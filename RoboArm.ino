
#include <Servo.h>

#define servoXP 2
#define servoYP 3

#define JYP A5
#define JXP A4
int cPin = A3;
int cVal;
int serialprintfunc() {
cVal = analogRead(cPin);
}
const long MIN_PULSE = 800;
const long MAX_PULSE = 2000;
const long MIN_POT = 0;
const long MAX_POT = 1023;
const long POWER_OF_TWO_TO_AVERAGE = 2;

const long SERVO_PULSE_RANGE = MAX_PULSE - MIN_PULSE;
const long START_PULSE_X = MIN_PULSE + (SERVO_PULSE_RANGE / 2);
const long START_PULSE_Y = MIN_PULSE + (SERVO_PULSE_RANGE / 8);

const long POT_RANGE = MAX_POT - MIN_POT;

const int BUFFER_SIZE = 1 << POWER_OF_TWO_TO_AVERAGE;
const int BUFFER_LIMIT = BUFFER_SIZE - 1;

const unsigned long ANALOG_READ_PERIOD = 5000;
const unsigned long DEBUG_PERIOD = 100000;
const unsigned long SERVO_PERIOD = 20000;

const long LOW_CENTER_THRESHOLD = 510;
const long HIGH_CENTER_THRESHOLD = 512;
const long POT_TO_SPEED_CONSTANT = 12;

long averagingBufferX[BUFFER_SIZE];
long averagingBufferY[BUFFER_SIZE];
int bufferIndex = 0;
long servoPosition[] = {START_PULSE_X, START_PULSE_Y};

long bufferTotalX = 0;
long bufferTotalY = 0;

unsigned long lastDebug;
unsigned long lastServo;
unsigned long lastAnalogRead;

Servo myServo[2];
Servo servoC;

void setup()
{
  servoC.attach(6);
  Serial.begin(9600);
  myServo[0].attach(servoXP, MIN_PULSE, MAX_PULSE);
  myServo[1].attach(servoYP, MIN_PULSE, MAX_PULSE);

  myServo[0].writeMicroseconds(START_PULSE_X);
  myServo[1].writeMicroseconds(START_PULSE_Y);

  for (byte i = 0; i < BUFFER_SIZE; i++)
  {
    averagingBufferX[i] = (MAX_POT - MIN_POT) / 2;
    averagingBufferY[i] = (MAX_POT - MIN_POT) / 2;
    bufferTotalX += averagingBufferX[i];
    bufferTotalY += averagingBufferY[i];
  }

  lastDebug = micros();
  lastServo = lastDebug;
  lastAnalogRead = lastDebug;
}

void loop()
{
serialprintfunc();
cVal = map(cVal, 0, 1023, 0, 180);
servoC.write(cVal);
  checkAnalogReadTime();
}

void checkAnalogReadTime()
{
  if (micros() - lastAnalogRead > ANALOG_READ_PERIOD)
  {
    lastAnalogRead += ANALOG_READ_PERIOD;
    long joystickInputX = MAX_POT - analogRead(JXP);
    long joystickInputY = MAX_POT - analogRead(JYP);

    bufferIndex++;
    bufferIndex &= BUFFER_LIMIT;

    bufferTotalX -= averagingBufferX[bufferIndex];
    bufferTotalY -= averagingBufferY[bufferIndex];

    averagingBufferX[bufferIndex] = joystickInputX;
    averagingBufferY[bufferIndex] = joystickInputY;

    bufferTotalX += averagingBufferX[bufferIndex];
    bufferTotalY += averagingBufferY[bufferIndex];

    checkServoTime();
  }
}

void checkServoTime()
{
  if (micros() - lastServo > SERVO_PERIOD)
  {
    lastServo += SERVO_PERIOD;
    controlServo();
  }
}

void controlServo()
{
  long average[2];
  long servoSpeed[2];

  average[0] = bufferTotalX >> POWER_OF_TWO_TO_AVERAGE;
  average[1] = bufferTotalY >> POWER_OF_TWO_TO_AVERAGE;

  for (int i = 0; i < 2; i++)
  {
    if (average[i] < LOW_CENTER_THRESHOLD)
    {
      servoSpeed[i] = (average[i] - LOW_CENTER_THRESHOLD) / POT_TO_SPEED_CONSTANT;
    }
    else if (average[i] > HIGH_CENTER_THRESHOLD)
    {
      servoSpeed[i] = (average[i] - HIGH_CENTER_THRESHOLD) / POT_TO_SPEED_CONSTANT;
    }
    else
    {
      servoSpeed[i] = 0;
    }

    servoPosition[i] += servoSpeed[i];
    if (servoPosition[i] > MAX_PULSE)
      servoPosition[i] = MAX_PULSE;
    else if (servoPosition[i] < MIN_PULSE)
      servoPosition[i] = MIN_PULSE;

    myServo[i].writeMicroseconds(servoPosition[i]);
  }

  checkDebugTime(average[0], average[1], servoPosition[0], servoPosition[1], servoSpeed[0], servoSpeed[1]);
}

void checkDebugTime(long averageX, long averageY, long servoOutputX, long servoOutputY, long speedX, long speedY)
{
  if (micros() - lastDebug > DEBUG_PERIOD)
  {
    lastDebug += DEBUG_PERIOD;

    Serial.print(F("average = "));
    Serial.print(averageX); Serial.print(F(", "));
    Serial.print(averageY); Serial.print(F(", Servo = "));
    Serial.print(servoOutputX); Serial.print(F(", "));
    Serial.print(servoOutputY); Serial.print(F(", Speed = "));
    Serial.print(speedX); Serial.print(F(", "));
    Serial.println(speedY);
  }
}
