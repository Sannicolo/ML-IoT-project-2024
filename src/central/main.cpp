#include <ArduinoBLE.h>

// Define the same service and characteristic UUIDs as the Peripheral
const char* SERVICE_UUID = "12345678-1234-5678-1234-56789abcdef0";
const char* CHARACTERISTIC_UUID = "abcdef01-1234-5678-1234-56789abcdef0";

BLEDevice peripheralDevice;
BLECharacteristic sendCharacteristic;

void setup() {
  Serial.begin(9600);
  while (!Serial); // Wait for Serial to be ready

  // Initialize BLE
  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1);
  }

  Serial.println("Central Arduino initializing...");
  delay(2000); // Wait for 2 seconds before scanning

  Serial.println("Central Arduino scanning for Peripheral...");
  BLE.scanForUuid(SERVICE_UUID);
}

void loop() {
  // Check if a peripheral is available
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    Serial.print("Found peripheral: ");
    Serial.println(peripheral.address());

    // Verify if the peripheral has the desired service
    if (peripheral.hasService(SERVICE_UUID)) {
      Serial.println("Peripheral has the desired service. Attempting to connect...");

      // Connect to the peripheral
      if (peripheral.connect()) {
        Serial.println("Connected to peripheral");

        // Discover the service and characteristic
        if (peripheral.discoverService(SERVICE_UUID)) {
          Serial.println("Discovered service");

          sendCharacteristic = peripheral.characteristic(CHARACTERISTIC_UUID);

          if (sendCharacteristic) {
            Serial.println("Found characteristic. Preparing to send data...");

            // Prepare the message to send
            String message = "Hello from Central!";
            // Send the message as a C-style string
            if (sendCharacteristic.writeValue(message.c_str())) {
              Serial.print("Sent: ");
              Serial.println(message);
            } else {
              Serial.println("Failed to send data");
            }
          } else {
            Serial.println("Failed to find the characteristic");
          }
        } else {
          Serial.println("Failed to discover the service");
        }

        // Disconnect after sending
        peripheral.disconnect();
        Serial.println("Disconnected from peripheral");

        // Restart scanning after a short delay
        delay(2000);
        Serial.println("Restarting scan for Peripheral...");
        BLE.scanForUuid(SERVICE_UUID);
      } else {
        Serial.println("Failed to connect to peripheral");
      }
    } else {
      Serial.println("Peripheral does not have the desired service");
    }
  }
}