#include <ArduinoBLE.h>

BLEService PeripheralService("19B10000-E8F2-537E-4F6C-D104768A1214");
BLECharacteristic PeripheralCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite, sizeof(float));

void sendData(BLEDevice peripheral, float *WeightBiasPtr);
void BLECentralSetup(float *WeightBiasPtr);
void CentralSearch(float *WeightBiasPtr);
void BLEPeripheralSetup(float *WeightBiasPtr);
void PeripheralLoop(float *WeightBiasPtr);

void sendData(BLEDevice peripheral, float *WeightBiasPtr) {
  // connect to the peripheral
  Serial.println("Connecting ...");

  if (peripheral.connect()) {

    if (peripheral.discoverAttributes()) {
      Serial.println("Attribute discovery succeded!");
    }
    else {
      Serial.println("Attribute discovery failed!");
      peripheral.disconnect();
      return;
    }

    if (peripheral.localName() == "Peripheral") {
      BLEService peripheralService = peripheral.service("19B10000-E8F2-537E-4F6C-D104768A1214");

      if (peripheralService) {
        Serial.println("Found peripheral service");
        BLECharacteristic peripheralCharacteristic = peripheralService.characteristic("19B10001-E8F2-537E-4F6C-D104768A1214");

        if (peripheralCharacteristic) {
          Serial.println("Found peripheral switch characteristic");
          for (int i = 0; i < 5792; i++) {
            uint8_t byteArray[sizeof(float)];
            memcpy(byteArray, &WeightBiasPtr[i], sizeof(float));

            Serial.print("Sending value: ");
            Serial.print(i + 1);
            Serial.print(" ");
            Serial.println(WeightBiasPtr[i]);

            peripheralCharacteristic.writeValue(byteArray, sizeof(float));
          }
        }
      }
    }
  }
  else {
    Serial.println("Failed to connect!");
    return;
  }

  Serial.println("Disconnecting ...");
  peripheral.disconnect();
  Serial.println("Disconnected");
}

void BLECentralSetup(float *WeightBiasPtr) {

  Serial.begin(9600);
  while (!Serial);

  if (!BLE.begin()) {
    Serial.println("Starting Bluetooth Low Energy failed!");

    while (1);
  }

  BLE.scan();
  CentralSearch(WeightBiasPtr);
}

void BLEPeripheralSetup(float *WeightBiasPtr) {
  Serial.begin(9600);
  while (!Serial);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth Low Energy failed!");

    while (1);
  }

  BLE.setLocalName("Peripheral");
  BLE.setAdvertisedService(PeripheralService);

  PeripheralService.addCharacteristic(PeripheralCharacteristic);
  BLE.addService(PeripheralService);
  BLE.advertise();

  Serial.println("BLE Peripheral");
  delay(2000);
  PeripheralLoop(WeightBiasPtr);
}

void PeripheralLoop(float *WeightBiasPtr) {
  Serial.println("Peripheral Loop");
  bool transferComplete = false;

  while (!transferComplete) {
    // listen for Bluetooth Low Energy peripherals to connect:
    BLEDevice central = BLE.central();

    // if a central is connected to peripheral:
    if (central) {
      Serial.print("Connected to central: ");
      Serial.println(central.address());

      // while the central is still connected to peripheral:
      int count = 0;
      while (central.connected()) {

        if (PeripheralCharacteristic.written()) {

          uint8_t byteArray[sizeof(float)];
          PeripheralCharacteristic.readValue(byteArray, sizeof(float));

          // Convert the byte array back to a float
          float receivedFloat;
          memcpy(&receivedFloat, byteArray, sizeof(float));

          // Print the received float value
          Serial.print("Receiving value: ");
          Serial.print(count + 1);
          Serial.print(" ");
          Serial.println(receivedFloat);
          WeightBiasPtr[count] = receivedFloat;
          count++;
        }
      }

      Serial.print(F("Disconnected from central: "));
      Serial.println(central.address());
      transferComplete = true;
    }
  }
}

void CentralSearch(float *WeightBiasPtr) {
  bool foundPeripheral = false;
  Serial.println("Searching...");
  while (!foundPeripheral) {

    // check if a peripheral has been discovered
    BLEDevice peripheral = BLE.available();

    if (peripheral) {
      if (peripheral.localName() == "Peripheral") {
        BLE.stopScan();
        Serial.println("Peripheral found. Connecting ...");

        sendData(peripheral, WeightBiasPtr);
        foundPeripheral = true;

      }
    }
  }
}
