#include <ArduinoBLE.h>

BLEService PeripheralService("19B10000-E8F2-537E-4F6C-D104768A1214"); // Bluetooth® Low Energy LED Service

// Bluetooth® Low Energy LED Switch Characteristic - custom 128-bit UUID, read and writable by central
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
      // get the peripheral service
      BLEService peripheralService = peripheral.service("19B10000-E8F2-537E-4F6C-D104768A1214");

      if (peripheralService) {
        Serial.println("Found peripheral service");
        // get the peripheral switch characteristic
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

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("Starting Bluetooth Low Energy failed!");

    while (1);
  }

  // start scanning for peripherals
  BLE.scan();
  CentralSearch();
}

void BLEPeripheralSetup(){
  // set advertised local name and service UUID:
  BLE.setLocalName("Peripheral");
  BLE.setAdvertisedService(PeripheralService);

  // add the characteristic to the service
  PeripheralService.addCharacteristic(PeripheralCharacteristic);

  // add service
  BLE.addService(PeripheralService);

  // set the initial value for the characeristic:
  PeripheralCharacteristic.writeValue(0);

  // start advertising
  BLE.advertise();

  Serial.println("BLE Peripheral");
  PeripherialLoop();
}

void PeripherialLoop(){
  bool peripheralConnected = false;

  while(!peripheralConnected){
    // listen for Bluetooth® Low Energy peripherals to connect:
    BLEDevice central = BLE.central();

    // if a central is connected to peripheral:
    if (central) {
      Serial.print("Connected to central: ");
      // print the central's MAC address:
      Serial.println(central.address());

      // while the central is still connected to peripheral:
      while (central.connected()) {
        // if the remote device wrote to the characteristic,
        // use the value to control the LED:
        if (PeripheralCharacteristic.written()) {
        Serial.println(PeripheralCharacteristic.value());
        }
      }

      // when the central disconnects, print it out:
      Serial.print(F("Disconnected from central: "));
      Serial.println(central.address());
    }
  }
}

void CentralSearch() {
  bool foundPeripheral = false;
  while (!foundPeripheral) {
    // check if a peripheral has been discovered
    BLEDevice peripheral = BLE.available();
    Serial.println("Searching...");

    if (peripheral) {
      // see if peripheral is device with local name "Peripheral"
      if (peripheral.localName() == "Peripheral") {
        // stop scanning
        BLE.stopScan();

        sendData(peripheral);
        foundPeripheral = true;
        // peripheral disconnected, we are done
        while (1);
      }
    }
  }
}
