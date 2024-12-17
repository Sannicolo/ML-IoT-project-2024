#include <ArduinoBLE.h>

BLEService PeripheralService("19B10000-E8F2-537E-4F6C-D104768A1214"); 
BLEByteCharacteristic PeripheralCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

void sendData(BLEDevice peripheral);
void BLECentralSetup();
void CentralSearch();
void BLEPeripheralSetup();
void PeripherialLoop();

void sendData(BLEDevice peripheral) {
  // connect to the peripheral
  Serial.println("Connecting ...");

  if (peripheral.connect()) {
    Serial.println("Connected");

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
          peripheralCharacteristic.writeValue((uint8_t)1);
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

void BLECentralSetup() {
  Serial.begin(9600);
  while (!Serial);

  if (!BLE.begin()) {
    Serial.println("Starting Bluetooth Low Energy failed!");

    while (1);
  }

  BLE.scan();
  CentralSearch();
}

void BLEPeripheralSetup(){
  BLE.setLocalName("Peripheral");
  BLE.setAdvertisedService(PeripheralService);

  PeripheralService.addCharacteristic(PeripheralCharacteristic);
  BLE.addService(PeripheralService);
  PeripheralCharacteristic.writeValue(0);
  BLE.advertise();

  Serial.println("BLE Peripheral");
  PeripherialLoop();
}

void PeripherialLoop(){
  bool peripheralConnected = false;
  Serial.println("Peripheral Loop");

  while(!peripheralConnected){
    // listen for Bluetooth Low Energy peripherals to connect:
    BLEDevice central = BLE.central();

    // if a central is connected to peripheral:
    if (central) {
      Serial.print("Connected to central: ");
      Serial.println(central.address());

      // while the central is still connected to peripheral:
      while (central.connected()) {
        Serial.println("Connected");
        if (PeripheralCharacteristic.written()) {
          Serial.println(PeripheralCharacteristic.value());
        }
      }

      Serial.print(F("Disconnected from central: "));
      Serial.println(central.address());
      peripheralConnected = true;
    } else {
      Serial.println("No central connected!!!");
    }
  }
}

void CentralSearch() {
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

        sendData(peripheral);
        //foundPeripheral = true;
        // peripheral disconnected
        while (1);
      }
    }
  }
}
