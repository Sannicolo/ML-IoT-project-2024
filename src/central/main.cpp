#include <ArduinoBLE.h>

void exploreService(BLEService service);
void explorerPeripheral(BLEDevice peripheral);
void exploreCharacteristic(BLECharacteristic characteristic);
void exploreDescriptor(BLEDescriptor descriptor);
void printData(const unsigned char data[], int length);

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

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("Starting Bluetooth Low Energy failed!");

    while (1);
  }

  // start scanning for peripherals
  BLE.scan();
}

void loop() {
  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();
  Serial.println("Searching...");

  if (peripheral) {
    // see if peripheral is device with local name "Peripheral"
    if (peripheral.localName() == "Peripheral") {
      // stop scanning
      BLE.stopScan();

      sendData(peripheral);

      // peripheral disconnected, we are done
      while (1);
    }
  }
}
