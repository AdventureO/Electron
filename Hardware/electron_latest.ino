/* This example shows how to use continuous mode to take
range measurements with the VL53L0X. It is based on
vl53l0x_ContinuousRanging_Example.c from the VL53L0X API.

The range readings are in units of mm. */

#include <Wire.h>
#include <VL53L0X.h>


VL53L0X sensor;
const int trigPin = 9;
const int echoPin = 10;
int distance = 0;
int infSensorDist = 0;
const int analogInPin = A0;

int readUltrasonicSensor() 
{
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  int duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  int distance= duration*0.034/2;
  // Prints the distance on the Serial Monitor
  return distance;
}


void sendToStm(int dist, char sChar, char eChar) {
  char toSend[16];
  itoa(dist, toSend, 10);
  if (dist / 100) {
    toSend[3] = eChar;
  }
  if (dist / 10) {
    toSend[2] = eChar;  
  } else {
    toSend[1] = eChar;  
  }
  int i = 0;
  Serial.print(sChar);  
  delay(50);
  do {
    Serial.print(toSend[i]);  
    delay(50);
    i++;
  }
  while (toSend[i - 1] != eChar);
}



void setup()
{
  Serial.begin(9600);
  Wire.begin();
 
  sensor.init();
//   Serial.println("defrgt");
  sensor.setTimeout(500);

  // Start continuous back-to-back mode (take readings as
  // fast as possible).  To use continuous timed mode
  // instead, provide a desired inter-measurement period in
  // ms (e.g. sensor.startContinuous(100)).
  sensor.startContinuous(100);
}

void loop()
{
  distance = sensor.readRangeContinuousMillimeters() / 10;
  //Serial.println(distance);
  if (sensor.timeoutOccurred()) { Serial.print("TIMEOUT"); }
  sendToStm(distance, '(', ')');

   float volts = analogRead(analogInPin)*0.0048828125;  // value from sensor * (5/1024)
   int infSensorDist = 13*pow(volts, -1);
 
  sendToStm(infSensorDist, '[', ']');
  sendToStm(readUltrasonicSensor(), '{', '}');
  Serial.print('<');
  Serial.print(millis());
  Serial.print('>');  
//  Serial.println();
}
