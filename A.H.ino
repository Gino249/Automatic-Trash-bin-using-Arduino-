#include <Servo.h>

#define TRIGGER_PIN  9 // Arduino pin connected to the trigger pin of ultrasonic sensor
#define ECHO_PIN     10 // Arduino pin connected to the echo pin of ultrasonic sensor
#define SERVO_PIN    11 // Arduino pin connected to the signal pin of servo motor
#define MAX_DISTANCE 20 // Maximum distance threshold for triggering servo (in centimeters)

Servo servo;

void setup() {
  Serial.begin(9600);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  servo.attach(SERVO_PIN);
}

void loop() {
  long duration, distance;
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = (duration / 2) / 29.1; // Calculate distance in centimeters
  
  if (distance <= MAX_DISTANCE) {
    // If object is within range, open the lid
    servo.write(90); // 90 degrees position (adjust as needed)
    delay(1000); // Wait for 1 second
  } else {
    // If no object is detected, close the lid
    servo.write(0); // 0 degrees position (adjust as needed)
  }
  
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  
  delay(1000); // Wait for 1 second before taking the next reading
}

