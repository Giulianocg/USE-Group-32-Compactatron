/////////////////////////////////////////////
//CHANGE THE VARIABLES BELOW FOR YOUR LIKING.
/////////////////////////////////////////////

//Distance sensors.
int amountOfDistanceChecks = 5; //if all distance checks are below minimumDistance, then close the trash can.
int durationBetweenChecks = 100; //milliseconds.
int minimumDistance = 10; //centimeters.
int height = 100; //centimeters.
int counter = 0; //sends distance measured every X measurements
//Compressing.
int durationForcompressingAndDecompressing = 6000; //milliseconds, time it takes for closing and time it takes to open, total time = 2*durationForcompressingAndDecompressing.
int durationBeforePressureAfterPressureAndAfterRelease = 1000; //milliseconds, time it waits before compressing, after the plastic was compressed, time it takes after releasing (after opening), total time = 2*durationBeforePressureAfterPressureAndAfterRelease.

//Locking.
int timeForLockingAndUnlocking = 1000; // Milliseconds, duration for rotation of the motors rotate when unlocking and locking (for example, if you put 1000, it takes 1000 milliseconds rotating to lock and 1000 milliseconds rotating to unlock).

///////////////////////////////////
//DON'T CHANGE THE VARIABLES BELOW.
///////////////////////////////////

//stuff for serial communication with ESP32
#include <SoftwareSerial.h>
SoftwareSerial ArdUNO_soft_ser (10, 11); //(RX, TX)
char c;
String dataIn;

//DISTANCE SENSORS VARIABLES.
int trig = 12;
int echo = 13;
int timeInMicro;
int distanceInCm;

//MOTOR VARIABLES.
int ena = 5;
int in1 = 6;
int in2 = 7;
int in3 = 3;
int in4 = 4;

//GREEN LIGHT VARIABLE.
int greenPin9 = 9;
int redPin8 = 8;

void setup() {
  //serial comms
  Serial.begin(19200); //UNO to monitor
  ArdUNO_soft_ser.begin(9600); //UNO to ESP32

  //DISTANCE SENSORS SETUPS.
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);

  //MOTOR SETUPS.
  pinMode(ena, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  //LIGHTS SETUP.
  pinMode(greenPin9, OUTPUT);
  pinMode(redPin8, OUTPUT);
  greenLightOn(); //Turns green light on.

}


void loop() {
  if(ArdUNO_soft_ser.available()>0){
    c=ArdUNO_soft_ser.read();
    if(c==0){ GreenFlicker(3,50);} //wifi disconnected
    else if(c==1){RedFlicker(3,50); } //wifi connected
    else if(c==2){ GreenFlicker(6, 10);} //connection stablished
    else { RedFlicker(6, 10); } //couldnt connect to website
    c=0;
  }

  //If the trash is supposed to lock and compress, shoudClose will evaluate to true and the if statement below will hold.
  boolean shouldClose = checkWithTimeElapsed(amountOfDistanceChecks, durationBetweenChecks, minimumDistance);
  Serial.println(shouldClose);
  
  if(shouldClose) {
    ArdUNO_soft_ser.print("&n3=1\n"); //display 'compressing' on website
    Serial.print("&n3=1\n"); //delete later
    greenLightOff(); //Turns green light off.
    delay(10);
    redLightOn(); //Turns red light on.
    locking(timeForLockingAndUnlocking); //Locks the trash.
    compressAndDecompress(durationForcompressingAndDecompressing, durationBeforePressureAfterPressureAndAfterRelease); //Compress and decompress.
    unlocking(timeForLockingAndUnlocking); //Unlocks trash.
    redLightOff(); //Turns green light off.
    delay(10);
    greenLightOn(); //Turns green light on.
    ArdUNO_soft_ser.print("&n3=0\n"); //undo 'compressing' on website
  }
}

//////////////////////////////////////////////////////////////////////////////
//DISTANCE SENSORS AUXILIARY METHODS.
//////////////////////////////////////////////////////////////////////////////

//Parameter amountOfDistanceChecks: Number of times that the distances will be checked.
//Parameter durationBetweenChecks: Time that will be between each check in milliseconds (total duration is durationBetweenChecks * amountOfDistanceChecks).
//Parameter minimumDistance: Lower bound for distance to be checked.
//Output boolean: True if all distances checks were below the "minimumDistance" during "durationBetweenChecks" milliseconds, false if at least 1 distance is below the specified minimumDistance.
boolean checkWithTimeElapsed(int amountOfDistanceChecks, int durationBetweenChecks, int minimumDistance) {

  //Creates a list of distances of size amountOfDistanceChecks and all elements equals to 0.
  int distancesList[amountOfDistanceChecks];
  
  //Each element of distanceList will be filled with the current distance
  float sum = 0 ;
  for(int i = 0; i < amountOfDistanceChecks; i++) {
    distancesList[i] = calculateDistanceCentimeters();
    sum += distancesList[i];
    delay(durationBetweenChecks);
  }

  float percentage = (height - (sum/amountOfDistanceChecks))*100/height;
  String payload = "&n2=" + String(percentage) + "\n";
  counter +=1;

  if(counter >= 3){ 
    ArdUNO_soft_ser.print(payload); //send full percentage to website
    Serial.print(payload);
    counter=0;
  } 

  //If at least 1 distance is bigger than the minimum check, return false
  for(int i = 0; i < amountOfDistanceChecks; i++) {
    if (distancesList[i] >= minimumDistance) {
      return false;
    }
  }
  //Return true if no distance is bigger than the minimum check.
  return true;
  
}


// Calculates distance in cm from distance sensor
int calculateDistanceCentimeters() {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  timeInMicro = pulseIn(echo, HIGH);
  distanceInCm = timeInMicro / 29 / 2;
  Serial.println(String(distanceInCm) + "cm");
  delay(100);
  return distanceInCm;
}


//////////////////////////////////////////////////////////////////////////////
//MOTORS AUXILIARY METHODS.
//////////////////////////////////////////////////////////////////////////////

//LOCKING
void locking(int timeForLockingAndUnlocking) {
  //clockise max speed
  Serial.println("locking");
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(ena, 255);
  delay(timeForLockingAndUnlocking);

  //stop
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  Serial.println("stop locking");
}

//UNLOCKING
void unlocking(int timeForLockingAndUnlocking) {
  //clockise max speed
  Serial.println("unlocking");
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(ena, 255);
  delay(timeForLockingAndUnlocking);

  //stop
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW); 
  Serial.println("stop unlocking"); 
}

//COMPRESSING METHOD.
//Parameter durationForcompressingAndDecompressing: Time it takes for closing and time it takes to open, total time = 2*durationForcompressingAndDecompressing
//Parameter durationBeforePressureAfterPressureAndAfterRelease: Time it waits before compressing, after the plastic was compressed, and time it takes after releasing (after opening), total time = 3*durationBeforePressureAfterPressureAndAfterRelease
//Ouput void: Compresses and decompresses trash can.
void compressAndDecompress(int durationForcompressingAndDecompressing, int durationBeforePressureAfterPressureAndAfterRelease) {
  
  //stop
  Serial.println("stopping");
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  delay(durationBeforePressureAfterPressureAndAfterRelease);
  
  //clockise max speed
  Serial.println("compressing");
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  //the lower the integer in digitalWrite(ena, integer),
  //the slower it will turn
  digitalWrite(ena, 255);
  delay(durationForcompressingAndDecompressing);

  //stop
  Serial.println("stopping");
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  delay(durationBeforePressureAfterPressureAndAfterRelease);

  //counter clockwise max speed
  Serial.println("decompressing");
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  digitalWrite(ena, 255);
  delay(durationForcompressingAndDecompressing);

  //stop
  Serial.println("stopping");
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  delay(durationBeforePressureAfterPressureAndAfterRelease);
}

//////////////////////////////////////////////////////////////////////////////
//LIGHT AUXILIARY METHODS.
//////////////////////////////////////////////////////////////////////////////

//Turns green light on.
void greenLightOn() {
  Serial.println("green light on");
  digitalWrite(greenPin9, HIGH);
}

//Turns green light off.
void greenLightOff() {
  Serial.println("green light off");
  digitalWrite(greenPin9, LOW);
}

//Turns red light on.
void redLightOn() {
  Serial.println("red light on");
  digitalWrite(redPin8, HIGH);
}

//Turns red light off.
void redLightOff() {
  Serial.println("red light off");
  digitalWrite(redPin8, LOW);
}

void GreenFlicker(int n, int del){
  for(int i = 0; i < n; i++) {
    digitalWrite(greenPin9, HIGH);
    delay(del);
    digitalWrite(greenPin9, LOW);
    delay(del);
  }
}

void RedFlicker(int n, int del){
  for(int i = 0; i < n; i++) {
    digitalWrite(redPin8, HIGH);
    delay(del);
    digitalWrite(redPin8, LOW);
    delay(del);
  }
}