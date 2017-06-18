//define the Pin numbers and associate them with a name
#define PIN_SONAR_TRIG            4
#define PIN_SONAR_ECHO            13
#define PIN_MOTOR_LEFT_FORWARD    12
#define PIN_MOTOR_LEFT_BACKWARD   11
#define PIN_MOTOR_RIGHT_FORWARD   10
#define PIN_MOTOR_RIGHT_BACKWARD  9
#define PIN_LIGHT_SENSOR 0
#define PIN_INDICATOR_LEFT  8
#define PIN_INDICATOR_RIGHT 7
#define PIN_TAIL_LIGHTS     5
#define PIN_HEAD_LIGHTS     3
#define PIN_SPEAKER               2

//define the maximum and minimum sonar distance
#define MAX_SONAR_DISTANCE        60
#define MIN_SONAR_DISTANCE        3

//define the frequency at which the horn will sound
#define SPEAKER_FREQUENCY 100

//define some enumerations as constants
#define FORWARD 1
#define STOP 0
#define BACKWARD -1

#define TNONE 0
#define TLEFT 1
#define TRIGHT 2




void setup() {
  //configure modes
  pinMode(PIN_SONAR_TRIG, OUTPUT);
  pinMode(PIN_SONAR_ECHO, INPUT);
  pinMode(PIN_MOTOR_RIGHT_FORWARD, OUTPUT);
  pinMode(PIN_MOTOR_RIGHT_BACKWARD, OUTPUT);
  pinMode(PIN_MOTOR_LEFT_FORWARD, OUTPUT);
  pinMode(PIN_MOTOR_LEFT_BACKWARD, OUTPUT);
  pinMode(PIN_INDICATOR_LEFT, OUTPUT);
  pinMode(PIN_INDICATOR_RIGHT, OUTPUT);
  pinMode(PIN_TAIL_LIGHTS, OUTPUT);
  pinMode(PIN_HEAD_LIGHTS, OUTPUT);
  pinMode(PIN_SPEAKER, OUTPUT);
  pinMode(PIN_LIGHT_SENSOR, INPUT);
  pinMode(PIN_LIGHT_SENSOR, INPUT);

  
  digitalWrite(PIN_MOTOR_RIGHT_FORWARD, LOW);
  digitalWrite(PIN_MOTOR_RIGHT_BACKWARD, LOW);
  digitalWrite(PIN_MOTOR_LEFT_FORWARD, LOW);
  digitalWrite(PIN_MOTOR_LEFT_BACKWARD, LOW);

  //start serial communication
  Serial.begin(38400);
  Serial.setTimeout(100);
}


float distance = 10;
signed char turn = TNONE;
unsigned int timer = 0;
float lightLevel = 0;
float maxLightLevel = 0;
float minLightLevel = 1023;
float LEDPWM = 0;
char msg[3];
bool isAuton = false;


void loop() {

  //find the distance between the sonar and the first thing in front of it
  /*digitalWrite(PIN_SONAR_TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(PIN_SONAR_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_SONAR_TRIG, LOW);

  distance = pulseIn(PIN_SONAR_ECHO, HIGH) * 0.034 / 2;*/

  //get the light level
  lightLevel = analogRead(PIN_LIGHT_SENSOR);
  //Serial.println(distance);

  //update the minimum and maximum light levels
  if (lightLevel > maxLightLevel) maxLightLevel = lightLevel;
  if (lightLevel < minLightLevel) minLightLevel = lightLevel;

  //formula to assign PWM to headlights
  LEDPWM = 255.0 - ( ( (lightLevel - minLightLevel) / (maxLightLevel - minLightLevel)) * 255.0);


  //only give the headlights power if they are above this threshold, else turn them off
  if (LEDPWM > 8) analogWrite(PIN_HEAD_LIGHTS, LEDPWM);
  else analogWrite(PIN_HEAD_LIGHTS, 0);

  //if the car is taking a left turn blink the left indicator lights
  if (turn == TLEFT)
  {
    digitalWrite(PIN_INDICATOR_RIGHT, LOW);
    if (timer % 10 == 0) digitalWrite(PIN_INDICATOR_LEFT, HIGH);
    if (timer % 20 == 0) digitalWrite(PIN_INDICATOR_LEFT, LOW);
  }
  //if the car is taking a right turn, blink the right indicator lights
  else if (turn == TRIGHT)
  {
    digitalWrite(PIN_INDICATOR_LEFT, LOW);
    if (timer % 10 == 0) digitalWrite(PIN_INDICATOR_RIGHT, HIGH);
    if (timer % 20 == 0) digitalWrite(PIN_INDICATOR_RIGHT, LOW);
  }
  //if the car is not turning, keep all the indicators off.
  else
  {
    digitalWrite(PIN_INDICATOR_LEFT, LOW);
    digitalWrite(PIN_INDICATOR_RIGHT, LOW);
  }


  //get information from the C# program which tells the car what to do
  //Read everything up to 4 bytes until a newline character is hit
  Serial.readBytesUntil('\n', msg, 4);

  //if there is an object less than 5 cm in front of the car and the user is trying to go forward, stop the car
  if (!isAuton && distance < 5 && msg[0] == 'U') setMotorPower(STOP, STOP);
  else if (isAuton && distance < 5);
  {
    //if the car is within 5 cm of an object, reverse, turn, and go forward for a bit
    setMotorPower(BACKWARD, BACKWARD);
    delay(2000);
    setMotorPower(BACKWARD, FORWARD);
    delay(1000);
    setMotorPower(FORWARD, FORWARD);
    delay(1000);
  }
  else
  {
    if (msg[0] == 'U') setMotorPower(FORWARD, FORWARD);
    else if (msg[0] == 'D') setMotorPower(BACKWARD, BACKWARD);
    else if (msg[0] == 'L') setMotorPower(BACKWARD, FORWARD);
    else if (msg[0] == 'R') setMotorPower(FORWARD, BACKWARD);
    else if (msg[0] == 'N') setMotorPower(STOP, STOP);
  }

  
  //if the C# program tells the car to horn, play a tone at the specified frequency on the speaker, else turn it off.
  if (msg[1] == 'H') tone(PIN_SPEAKER, SPEAKER_FREQUENCY);
  else noTone(PIN_SPEAKER);

  //if the C# program sends "A", the program is in auton mode, so allow reversing and remanouvering when in front of an object
  if (msg[1] == 'A') isAuton = !isAuton; 
  

  //increment the timer and avoid incrementing past the integer limit
  if (timer == 4294967295) timer = 0;
  timer++;
}



void setMotorPower(signed char motorLeft, signed char motorRight)
{
  //if the motors aren't supposed to stop, provide power to the right pins based on what the car is supposed to do
  if (motorLeft != STOP && motorRight != STOP)
  {
      digitalWrite(PIN_MOTOR_LEFT_FORWARD, (motorLeft < 0) ? LOW : HIGH);
      digitalWrite(PIN_MOTOR_LEFT_BACKWARD, (motorLeft < 0) ? HIGH : LOW);

      digitalWrite(PIN_MOTOR_RIGHT_FORWARD, (motorRight < 0) ? LOW : HIGH);
      digitalWrite(PIN_MOTOR_RIGHT_BACKWARD, (motorRight < 0) ? HIGH : LOW);
  }
  else
  {
      //dont give any power, if the car is supposed to stop
      digitalWrite(PIN_MOTOR_LEFT_FORWARD, LOW);
      digitalWrite(PIN_MOTOR_LEFT_BACKWARD, LOW);

      digitalWrite(PIN_MOTOR_RIGHT_FORWARD, LOW);
      digitalWrite(PIN_MOTOR_RIGHT_BACKWARD, LOW);
  }

  //if the motors are going opposite directions, the car is turning, so update the turn variable accordingly
  if (motorLeft == BACKWARD && motorRight == FORWARD) turn = TLEFT;
  else if (motorLeft == FORWARD && motorRight == BACKWARD) turn = TRIGHT;
  else turn = TNONE;

  //if both motors are being told to go backwards, turn on the backlights
  if (motorLeft == BACKWARD && motorRight == BACKWARD) digitalWrite(PIN_TAIL_LIGHTS, HIGH);
  else digitalWrite(PIN_TAIL_LIGHTS, LOW);
     
}

