#ifndef BLE_COMMUNICATION_H
#define BLE_COMMUNICATION_H

#include <ArduinoBLE.h>


// Går att köra build, men vet ej om det funkar. project.cpp behöver uppdateras med exchangeWeights, där ena ardunio är initiator och den andra är responder.

BLEService weightService("180C");
BLECharacteristic weightCharacteristic("2A56", BLERead | BLEWrite, 512);
BLECharacteristic controlCharacteristic("2A57", BLERead | BLEWrite, 1);

void setupBLE() {
  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1);
  }

  BLE.setLocalName("Arduino");
  BLE.setAdvertisedService(weightService);
  weightService.addCharacteristic(weightCharacteristic);
  weightService.addCharacteristic(controlCharacteristic);
  BLE.addService(weightService);

  weightCharacteristic.writeValue((const uint8_t*)0, 0);
  controlCharacteristic.writeValue((const uint8_t*)0, 0);

  BLE.advertise();
  Serial.println("BLE device is now advertising");
}

void sendVector(float* WeightBiasPtr, unsigned int length) {
  Serial.println("Sending vector...");
  for (unsigned int i = 0; i < length; i++) {
    weightCharacteristic.writeValue((const uint8_t*)&WeightBiasPtr[i], sizeof(float));
    Serial.print("Sent value: ");
    Serial.println(WeightBiasPtr[i]);
  }
}

void receiveVector(float* WeightBiasPtr, unsigned int length) {
  Serial.println("Receiving vector...");
  for (unsigned int i = 0; i < length; i++) {
    while (!weightCharacteristic.written()) {
      // Wait for data to be written
    }
    weightCharacteristic.readValue((uint8_t*)&WeightBiasPtr[i], sizeof(float));
    Serial.print("Received value: ");
    Serial.println(WeightBiasPtr[i]);
  }
}

void exchangeWeights(float* WeightBiasPtr, unsigned int length, bool isInitiator) {
  uint8_t controlValue;
  if (isInitiator) {
    // Initiator sends first
    sendVector(WeightBiasPtr, length);
    controlValue = 1;
    controlCharacteristic.writeValue(&controlValue, 1); // Signal that sending is done
    do {
      controlCharacteristic.readValue(&controlValue, 1);
    } while (controlValue != 2); // Wait for responder to finish receiving and sending
    receiveVector(WeightBiasPtr, length);
  } else {
    // Responder receives first
    do {
      controlCharacteristic.readValue(&controlValue, 1);
    } while (controlValue != 1); // Wait for initiator to finish sending
    receiveVector(WeightBiasPtr, length);
    sendVector(WeightBiasPtr, length);
    controlValue = 2;
    controlCharacteristic.writeValue(&controlValue, 1); // Signal that sending is done
  }
}

#endif // BLE_COMMUNICATION_H