#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>
#define BLYNK_TEMPLATE_ID "TMPL3fPEk8irW"
#define BLYNK_DEVICE_NAME "GarageDoor"


// Blynk Credentials
char auth[] = "JRPi-rOLqwMYODjZ6wWfIzEKZS3SrnnA";
char ssid[] = "Oppo";
char pass[] = "Gayu@1006";

// Pin Definitions
#define ULTRASONIC_TRIG_PIN D2  // HC-SR04 Trigger
#define ULTRASONIC_ECHO_PIN D1  // HC-SR04 Echo
#define PIR_PIN             D3  // PIR sensor
#define REED_PIN            D5  // Magnetic reed switch for door state
#define SERVO_PIN           D6  // Servo motor control pin

// Create objects
Servo doorServo;           // Servo motor object
BlynkTimer timer;          // Blynk timer for periodic tasks

// Global sensor variables
int motionStatus = 0;
long duration;
float distance = 0;
int reedState = 0;         // 0 = Closed, 1 = Open

// Function to read sensors and send data to Blynk
void sendSensorData() {
  // --- Ultrasonic Sensor ---
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH);
  distance = (duration * 0.034) / 2;  // Distance in cm
  Blynk.virtualWrite(V0, distance);   // Send distance
  
  // --- PIR Sensor ---
  motionStatus = digitalRead(PIR_PIN);
  Blynk.virtualWrite(V1, motionStatus);  // Send PIR status
  
  // --- Magnetic Reed Switch ---
  // Assuming using internal pull-up: LOW means closed (magnet near => door closed)
  reedState = digitalRead(REED_PIN);
  if (reedState == HIGH) {
    Blynk.virtualWrite(V2, "ON");  // Door open
  } else {
    Blynk.virtualWrite(V2, "OFF"); // Door closed
  }
  
  // (Optional) You can send servo angle on V3 if desired.
  // Blynk.virtualWrite(V3, doorServo.read());
  
  // Debug prints
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm, Motion: ");
  Serial.print(motionStatus);
  Serial.print(", Door: ");
  Serial.println((reedState == HIGH ? "ON" : "OFF"));
}

// Blynk Button Widget on V5 for Manual Door Control
BLYNK_WRITE(V5) {
  int buttonState = param.asInt();  // 1: ON, 0: OFF
  if (buttonState == 1) {
    // Open the door: Rotate servo to 90 degrees
    doorServo.write(90);
    // Also update door state accordingly
    Blynk.virtualWrite(V2, "ON");
    Serial.println("Door Opened via Blynk Button");
  } else {
    // Close the door: Rotate servo to 0 degrees
    doorServo.write(0);
    Blynk.virtualWrite(V2, "OFF");
    Serial.println("Door Closed via Blynk Button");
  }
}

void setup() {
  Serial.begin(115200);
  
  // Initialize WiFi and Blynk connection
  Blynk.begin(auth, ssid, pass);
  
  // Configure sensor pins
  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(REED_PIN, INPUT_PULLUP);  // Enable internal pull-up for reed switch
  
  // Attach servo and set initial position (closed position)
  doorServo.attach(SERVO_PIN);
  doorServo.write(0);
  
  // Set a timer to send sensor data every 2 seconds
  timer.setInterval(2000L, sendSensorData);
  
  Serial.println("System Initialized");
}

void loop() {
  Blynk.run();
  timer.run();
}