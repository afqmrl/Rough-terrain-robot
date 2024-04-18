#include <Arduino.h>

// Pin definitions for easier changes and understanding
#define ENCA 2
#define ENCB 3
#define PWM_PIN 5
#define IN1_PIN 6
#define IN2_PIN 7

// Encoder and timing variables for tracking motor position and speed
volatile int position = 0;
volatile float velocity = 0;
volatile long lastInterruptTime = 0;

long previousTime = 0;
int previousPosition = 0;

// Filter variables for smoothing the velocity readings
float velocityFilter1 = 0;
float previousVelocity1 = 0;

// PID control variables for tuning the control response
float errorIntegral = 0;
float previousError = 0;
float controlOutput = 0;

// Control parameters with initial gains for PID
float kp = 5.0;  // Proportional gain
float ki = 0.1;  // Integral gain
float kd = 0.01; // Derivative gain

// Control mode selector for switching between P, PI, and PID modes
enum ControlType { P, PI, PID };
ControlType controlMode = PID; // Set default control mode to PID

// Initial setup function, runs once on microcontroller reset
void setup() {
  Serial.begin(115200);
  pinMode(ENCA, INPUT);
  pinMode(ENCB, INPUT);
  pinMode(PWM_PIN, OUTPUT);
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);

  // Attach encoder read function to interrupt for real-time speed tracking
  attachInterrupt(digitalPinToInterrupt(ENCA), readEncoder, RISING);
}

// Main loop, runs repeatedly
void loop() {
  long currentTime = micros();
  float deltaTime = (currentTime - previousTime) / 1.0e6;

  // Temporary variables for current position and velocity
  int currentPosition = 0;
  float currentVelocity = 0;

  // Disable interrupts while reading shared variables to avoid corruption
  noInterrupts();
  currentPosition = position;
  currentVelocity = velocity;
  interrupts(); // Re-enable interrupts

  // Calculate velocity and apply low-pass filter for smoothing
  float measuredVelocity = calculateVelocity(currentPosition, previousPosition, deltaTime);
  previousPosition = currentPosition;
  previousTime = currentTime;

  // Convert revolutions per second to RPM
  float rpm = measuredVelocity * 60.0;

  velocityFilter1 = lowPassFilter(measuredVelocity, velocityFilter1, previousVelocity1);
  previousVelocity1 = measuredVelocity;

  // Set target velocity based on a sinusoidal function for testing
  float targetVelocity = 100 * (sin(currentTime / 1e6) > 0);

  // Compute control output based on selected mode
  controlOutput = computeControl(targetVelocity, velocityFilter1, deltaTime);

  // Set motor direction and speed based on control signal
  setMotorDirectionAndSpeed(controlOutput);

  // Print target and actual RPM to the serial monitor
  Serial.print("Target RPM: ");
  Serial.print(targetVelocity);
  Serial.print(", Actual RPM: ");
  Serial.println(rpm);

  delay(1);
}

// Function to calculate velocity based on encoder position changes
float calculateVelocity(int currentPosition, int previousPosition, float deltaTime) {
  return (currentPosition - previousPosition) / deltaTime; // Revolutions per second
}

// Low-pass filter function to smooth out velocity fluctuations
float lowPassFilter(float currentVelocity, float filterValue, float previousVelocity) {
  return 0.854 * filterValue + 0.0728 * currentVelocity + 0.0728 * previousVelocity;
}

// Compute control output based on PID calculations
float computeControl(float targetVelocity, float measuredVelocity, float deltaTime) {
  float error = targetVelocity - measuredVelocity;
  float derivative = 0;
  switch (controlMode) {
    case P:
      return kp * error;
    case PI:
      errorIntegral += error * deltaTime;
      return kp * error + ki * errorIntegral;
    case PID:
      errorIntegral += error * deltaTime;
      derivative = (error - previousError) / deltaTime;
      previousError = error;
      return kp * error + ki * errorIntegral + kd * derivative;
    default:
      return 0;  // Default to no control output
  }
}

// Function to set motor direction and speed based on control signal
void setMotorDirectionAndSpeed(float controlSignal) {
  int direction = (controlSignal < 0) ? -1 : 1;
  int power = constrain(abs(controlSignal), 0, 255);

  analogWrite(PWM_PIN, power); // Set PWM signal for speed control
  digitalWrite(IN1_PIN, direction == 1); // Set motor direction
  digitalWrite(IN2_PIN, direction != 1); // Set motor direction
}

// Interrupt service routine to read encoder signals
void readEncoder() {
  int b = digitalRead(ENCB);
  int increment = (b > 0) ? 1 : -1;
  position += increment;

  long currentTime = micros();
  float deltaTime = (currentTime - lastInterruptTime) / 1.0e6;
  velocity = increment / deltaTime;
  lastInterruptTime = currentTime;
}
