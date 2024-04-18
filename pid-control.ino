#include <Arduino.h>

// Pin definitions
#define ENCA 2
#define ENCB 3
#define PWM_PIN 5
#define IN1_PIN 6
#define IN2_PIN 7

// Encoder and timing variables
volatile int position = 0;
volatile float velocity = 0;
volatile long lastInterruptTime = 0;

long previousTime = 0;
int previousPosition = 0;

// Filter variables
float velocityFilter1 = 0;
float previousVelocity1 = 0;
float velocityFilter2 = 0;
float previousVelocity2 = 0;

// PID control variables
float errorIntegral = 0;
float previousError = 0;
float controlOutput = 0;

// Control parameters
float kp = 5.0;  // Proportional gain
float ki = 0.1;  // Integral gain
float kd = 0.01; // Derivative gain

// Control mode selector
enum ControlType { P, PI, PID };
ControlType controlMode = PID; // Set default control mode here

// Setup function
void setup() {
  Serial.begin(115200);
  pinMode(ENCA, INPUT);
  pinMode(ENCB, INPUT);
  pinMode(PWM_PIN, OUTPUT);
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(ENCA), readEncoder, RISING);
}

// Main control loop
void loop() {
  long currentTime = micros();
  float deltaTime = (currentTime - previousTime) / 1.0e6;

  int currentPosition = 0;
  float currentVelocity = 0;

  noInterrupts(); // Temporarily disable interrupts
  currentPosition = position;
  currentVelocity = velocity;
  interrupts(); // Re-enable interrupts

  float measuredVelocity = calculateVelocity(currentPosition, previousPosition, deltaTime);
  previousPosition = currentPosition;
  previousTime = currentTime;

  velocityFilter1 = lowPassFilter(measuredVelocity, velocityFilter1, previousVelocity1);
  previousVelocity1 = measuredVelocity;

  float targetVelocity = 100 * (sin(currentTime / 1e6) > 0);

  controlOutput = computeControl(targetVelocity, velocityFilter1, deltaTime);

  setMotorDirectionAndSpeed(controlOutput);

  Serial.print(targetVelocity);
  Serial.print(" ");
  Serial.println(velocityFilter1);

  delay(1);
}

// Supporting functions
float calculateVelocity(int currentPosition, int previousPosition, float deltaTime) {
  return (currentPosition - previousPosition) / deltaTime * 60.0 / 600.0;
}

float lowPassFilter(float currentVelocity, float filterValue, float previousVelocity) {
  return 0.854 * filterValue + 0.0728 * currentVelocity + 0.0728 * previousVelocity;
}

float computeControl(float targetVelocity, float measuredVelocity, float deltaTime) {
  float error = targetVelocity - measuredVelocity;
  float derivative = 0;
  switch (controlMode) {
    case P:
      // P controller
      return kp * error;
    case PI:
      // PI controller
      errorIntegral += error * deltaTime;
      return kp * error + ki * errorIntegral;
    case PID:
      // PID controller
      errorIntegral += error * deltaTime;
      derivative = (error - previousError) / deltaTime;
      previousError = error;
      return kp * error + ki * errorIntegral + kd * derivative;
    default:
      return 0;  // Default to no control output
  }
}

void setMotorDirectionAndSpeed(float controlSignal) {
  int direction = (controlSignal < 0) ? -1 : 1;
  int power = constrain(abs(controlSignal), 0, 255);

  analogWrite(PWM_PIN, power);
  digitalWrite(IN1_PIN, direction == 1);
  digitalWrite(IN2_PIN, direction != 1);
}

void readEncoder() {
  int b = digitalRead(ENCB);
  int increment = (b > 0) ? 1 : -1;
  position += increment;

  long currentTime = micros();
  float deltaTime = (currentTime - lastInterruptTime) / 1.0e6;
  velocity = increment / deltaTime;
  lastInterruptTime = currentTime;
}
