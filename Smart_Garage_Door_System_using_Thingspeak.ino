#include <ESP8266WiFi.h>
#include <Servo.h>
#include <NewPing.h>
#include <ThingSpeak.h>

// ------ Wi-Fi Credentials ------
const char* ssid = "Oppo";
const char* password = "Gayu@1006";

// ------ ThingSpeak Channel Details ------
unsigned long channelID = 2850827;        // Replace with your channel ID (a number)
const char* apiKey = "OEA3M9GR3KTIE32H";          // Replace with your Write API Key

WiFiClient client;

// ------ Pin Definitions ------
#define SERVO_PIN D6          // Servo motor control
#define PIR_SENSOR D2         // PIR sensor output
#define MAGNETIC_SENSOR D5    // Magnetic reed switch (with pull-up)
#define RED_LED D4            // LED for door closed
#define GREEN_LED D3          // LED for door open
#define ULTRASONIC_TRIG D7    // Ultrasonic sensor trigger
#define ULTRASONIC_ECHO D8    // Ultrasonic sensor echo

// ------ Ultrasonic Sensor Settings ------
#define MAX_DISTANCE 200      // Maximum distance (cm)

Servo garageServo;              // Create a Servo object
NewPing sonar(ULTRASONIC_TRIG, ULTRASONIC_ECHO, MAX_DISTANCE);

unsigned long doorOpenDuration = 5000;  // Door open duration in ms
bool doorIsOpen = false;

void setup() {
  Serial.begin(115200);
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  // Initialize ThingSpeak with the Wi-Fi client
  ThingSpeak.begin(client);

  // Initialize Servo
  garageServo.attach(SERVO_PIN);
  garageServo.write(0);  // Start with door closed

  // Sensor and LED setups
  pinMode(PIR_SENSOR, INPUT);
  pinMode(MAGNETIC_SENSOR, INPUT_PULLUP);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  
  // Set initial LED status: Door closed
  digitalWrite(RED_LED, HIGH);
  digitalWrite(GREEN_LED, LOW);
  
  Serial.println("System Initialized");
}

void loop() {
  // Read sensor values
  int motion = digitalRead(PIR_SENSOR);
  int doorState = digitalRead(MAGNETIC_SENSOR); // LOW = closed, HIGH = open
  
  // Ultrasonic sensor: measure distance in cm
  unsigned int distance = sonar.ping_cm();
  if (distance == 0) {
    distance = MAX_DISTANCE; // Out-of-range fallback
  }

  // Print readings for debugging
  Serial.print("Motion: "); Serial.print(motion);
  Serial.print(" | DoorState: "); Serial.print(doorState);
  Serial.print(" | Distance: "); Serial.print(distance); Serial.println(" cm");

  // Logic to open door
  if (motion == HIGH && distance < 50 && doorState == LOW && !doorIsOpen) {
    openDoor();
  }
  
  // Logic to close door
  if ((motion == LOW || distance >= 50) && doorIsOpen) {
    delay(doorOpenDuration);
    closeDoor();
  }

  // Send sensor data to ThingSpeak
  // For example:
  // Field1: motion, Field2: distance, Field3: doorState, Field4: door open flag (1 or 0)
 ThingSpeak.setField(1, (float)motion);
 ThingSpeak.setField(2, (float)distance);
 ThingSpeak.setField(3, (float)doorState);
 ThingSpeak.setField(4, (float)(doorIsOpen ? 1 : 0));
  
  int response = ThingSpeak.writeFields(channelID, apiKey);
  if(response == 200) {
    Serial.println("Data sent to ThingSpeak successfully!");
  } else {
    Serial.print("Failed to send data. Response code: ");
    Serial.println(response);
  }
  
  delay(15000); // Update every 15 seconds
}

void openDoor() {
  Serial.println("Opening Door...");
  garageServo.write(90);  // Rotate servo to open position
  doorIsOpen = true;
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);
  Serial.println("Door is now OPEN");
}

void closeDoor() {
  Serial.println("Closing Door...");
  garageServo.write(0);   // Rotate servo to close position
  doorIsOpen = false;
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, HIGH);
  Serial.println("Door is now CLOSED");
}
