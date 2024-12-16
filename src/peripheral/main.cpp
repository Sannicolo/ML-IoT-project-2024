#include <ArduinoBLE.h>

// Define custom service and characteristic UUIDs
const char* SERVICE_UUID = "12345678-1234-5678-1234-56789abcdef0";
const char* CHARACTERISTIC_UUID = "abcdef01-1234-5678-1234-56789abcdef0";

// Create BLE Service
BLEService customService(SERVICE_UUID);

// Create BLE Characteristic for receiving data (Write)
BLECharacteristic receiveCharacteristic(CHARACTERISTIC_UUID, BLEWrite, 20);

void setup() {
  Serial.begin(9600);
  while (!Serial); // Wait for Serial to be ready

  // Initialize BLE
  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1);
  }

  // Set up the service and characteristic
  customService.addCharacteristic(receiveCharacteristic);
  BLE.addService(customService);

  // Set the device name and start advertising
  BLE.setLocalName("PeripheralArduino");
  BLE.advertise();

  Serial.println("Peripheral Arduino is now advertising...");
}

void loop() {
  // Wait for a central device to connect
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    while (central.connected()) {
      // Check if the characteristic has been written to
      if (receiveCharacteristic.written()) {
        // Retrieve the data as a byte array
        const uint8_t* data = receiveCharacteristic.value();
        size_t length = receiveCharacteristic.valueLength();
        
        // Ensure the data is null-terminated and within buffer size
        char buffer[21] = {0}; // 20 bytes + 1 null terminator
        if (length > 20) length = 20; // Prevent buffer overflow
        memcpy(buffer, data, length);
        
        // Convert to Arduino String
        String receivedData = String(buffer);
        Serial.print("Received: ");
        Serial.println(receivedData);
      }
    }

    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}