#include <ArduinoBLE.h>

BLEService PeripheralService("19B10000-E8F2-537E-4F6C-D104768A1214");
BLECharacteristic PeripheralCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite, sizeof(float));

void sendData(BLEDevice peripheral, float *WeightBiasPtr);
void BLECentralSetup(float *WeightBiasPtr);
void CentralSearch(float *WeightBiasPtr);
void BLEPeripheralSetup( float *WeightBiasPtr);
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
          for (int i = 0; i < 5972; i++) {
            Serial.println(WeightBiasPtr[i]);
            uint8_t byteArray[sizeof(float)];
            memcpy(byteArray, &WeightBiasPtr[i], sizeof(float));
            delay(10);

            // Debug: Print byte array content before transmission
            /* Serial.print("Sending byte array: ");
            for (int j = 0; j < sizeof(float); j++) {
              uint8_t byteValue = byteArray[j];
              if (byteValue < 0x10) {
                Serial.print("0");
              }
              Serial.print(byteValue, HEX);
              Serial.print(" ");
            }
            Serial.println(); */

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
  if (!BLE.begin()){
    Serial.println("starting Bluetooth Low Energy failed!");

    while (1);
  }

  BLE.setLocalName("Peripheral");
  BLE.setAdvertisedService(PeripheralService);

  PeripheralService.addCharacteristic(PeripheralCharacteristic);
  BLE.addService(PeripheralService);
  //PeripheralCharacteristic.writeValue(0);
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
          // Serial.println(PeripheralCharacteristic.value());
          //  Assuming you have the byte array received from the peripheralCharacteristic
          //  Assuming you have a method to get the value as a byte array
          // Assuming you have a method to get the value as a byte array
          uint8_t byteArray[sizeof(float)];
          PeripheralCharacteristic.readValue(byteArray, sizeof(float));

          // Debug: Print byte array content after reception
          //Serial.print("Received byte array: ");
          /* for (int i = 0; i < sizeof(float); i++) {
            uint8_t byteValue = byteArray[i];
            if (byteValue < 0x10) {
              Serial.print("0");
            }
            Serial.print(byteValue, HEX);
            Serial.print(" ");
          } */
          // Serial.println();

          // Convert the byte array back to a float
          float receivedFloat;
          memcpy(&receivedFloat, byteArray, sizeof(float));

          // Print the received float value

          Serial.println(count);
          Serial.print(" ");
          Serial.print(receivedFloat);
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
  while (!foundPeripheral) {
    Serial.println("Searching...");

    // check if a peripheral has been discovered
    BLEDevice peripheral = BLE.available();

    if (peripheral) {
      Serial.print(peripheral.localName());
      if (peripheral.localName() == "Peripheral") {
        BLE.stopScan();
        Serial.println("Peripheral found. Connecting ...");

        sendData(peripheral, WeightBiasPtr);
        // foundPeripheral = true;
        //  peripheral disconnected
        while (1);
      }
    }
  }
}
