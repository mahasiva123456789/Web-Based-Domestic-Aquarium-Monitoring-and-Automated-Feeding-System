#include <ESP8266WiFi.h>
#include <NewPing.h>
#include <DHT.h>
#include <Servo.h>

// Replace with your network credentials
const char* ssid = "Redmi 9i";       // Replace with your Wi-Fi SSID
const char* password = "Mirna@2005.."; // Replace with your Wi-Fi Password

// Ultrasonic Sensor settings
#define TRIGGER_PIN  5  // GPIO5 (D1 on NodeMCU)
#define ECHO_PIN     4  // GPIO4 (D2 on NodeMCU)
#define MAX_DISTANCE 200 // Maximum distance to measure (in centimeters)

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// DHT Sensor settings
#define DHTPIN 2  // GPIO2 (D4 on NodeMCU)
#define DHTTYPE DHT11 // DHT 11

DHT dht(DHTPIN, DHTTYPE);

// Define LED pins
#define LED1_PIN 0  // GPIO0 (D3 on NodeMCU)
#define LED2_PIN 14  // GPIO14 (D5 on NodeMCU)
#define LED_PIN 12   // GPIO12 (D6 on NodeMCU) for the Ultrasonic sensor LED

Servo servo;

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Initialize LED pins
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  // Initialize Servo
  servo.attach(13); // D7 (GPIO 13)
  servo.write(0);
  delay(1000);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  server.begin();
  Serial.println("Server started");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); // This line prints the IP address
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New Client.");
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if (c == '\n') {
          if (currentLine.length() == 0) {
            // Read distance from the ultrasonic sensor
            unsigned int distance = sonar.ping_cm(); // Read distance in centimeters

            // Read humidity and temperature from the DHT sensor
            float humidity = dht.readHumidity();
            float temperature = dht.readTemperature();

            if (isnan(humidity) || isnan(temperature)) {
              Serial.println("Failed to read from DHT sensor!");
              client.stop();
              return;
            }

            String alertMessage = "";
            String ledMessage = "";

            if (temperature < 28.5) {
              alertMessage = "TEMPERATURE IS LOW";
              digitalWrite(LED1_PIN, HIGH);  // Turn on LED1
            } else if (temperature > 29) {
              alertMessage = "TEMPERATURE IS HIGH";
              digitalWrite(LED2_PIN, HIGH);  // Turn on LED2
            } else {
              // Turn off LEDs if temperature is within the normal range
              digitalWrite(LED1_PIN, LOW);
              digitalWrite(LED2_PIN, LOW);
            }

            if (distance > 3) {
              alertMessage += (alertMessage.length() > 0 ? ", " : "") + String("WATER LEVEL IS LOW");
              digitalWrite(LED_PIN, HIGH); // Turn on the LED if water level is above 3cm
            } else {
              digitalWrite(LED_PIN, LOW); // Turn off the LED if water level is below 3cm
            }

            // Print the response to the Serial Monitor
            Serial.print("Temperature: ");
            Serial.println(temperature);
            Serial.print("Humidity: ");
            Serial.println(humidity);
            Serial.print("Distance: ");
            Serial.println(distance);
            Serial.print("Alert Message: ");
            Serial.println(alertMessage);

            // Respond to the client
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: application/json");
            client.println("Access-Control-Allow-Origin: *"); // Allow cross-origin requests
            client.println("Connection: close");
            client.println();
            client.print("{\"temperature\":");
            client.print(temperature);
            client.print(", \"humidity\":");
            client.print(humidity);
            client.print(", \"distance\":");
            client.print(distance);
            if (alertMessage.length() > 0) {
                client.print(", \"alert\":\"");
                client.print(alertMessage);
                client.print("\"");
            }
            client.println("}");
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    client.stop();
    Serial.println("Client Disconnected.");
  }

  // Servo control in the main loop
  servo.write(180);
   delay(1000);
  servo.write(0);
  delay(10000); // Delay for 20 seconds (20000 milliseconds)
}
