#include <ArduinoBLE.h>

// BLE Oriori Ball
BLEService pressureService("FFB0");
BLECharacteristic pressureChar("FFB1",  // standard 16-bit characteristic UUID
    BLERead | BLENotify, 16); // remote clients will be able to get notifications if this characteristic changes

int oldLevel = 0;  // last battery level reading from analog input
long previousMillis = 0;  // last time the battery level was checked, in ms

void setup() {
  Serial.begin(9600);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");
    while (1);
  }

  // Spoof the name of the Oriori Ball to integrate with Xtoys
  BLE.setLocalName("weixin-nini");
  BLE.setDeviceName("weixin-nini");

  // Register
  BLE.setAdvertisedService(pressureService); 
  pressureService.addCharacteristic(pressureChar);
  BLE.addService(pressureService); 
  pressureChar.writeValue(oldLevel);

  // start advertising
  BLE.advertise();

  Serial.println("Bluetooth device active, waiting for connections...");
}

void loop() {
  // wait for a BLE central
  BLEDevice central = BLE.central();

  // if a central is connected to the peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    
    // turn on the LED to indicate the connection:
    digitalWrite(LED_BUILTIN, HIGH);

    // meassure every 200ms while connected
    while (central.connected()) {
      long currentMillis = millis();
      if (currentMillis - previousMillis >= 200) {
        previousMillis = currentMillis;
        updateLevel();
      }
    }
    
    // when the central disconnects, turn off the LED:
    digitalWrite(LED_BUILTIN, LOW);
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}

void updateLevel() {
  int volt = analogRead(A1);
  Serial.println(volt);

  /*
   * 826 is the baseline pressure sensor value
   * 1023 is the maximum pressure sesnor baule
   * 0-60 is the range that Oriori Ball usee
   * 
   * so map the value for good spoofing
   */
  int level = map(volt, 826, 1023, 0, 60);

  // upate if needed
  if (level != oldLevel) {

    /*
     * Oriori Ball characteristics for pressure is like this
     * 
     * AABCCC.C|
     * 
     * AA is battery level (not used)
     * B is a boolean indiciating if pressure when up or down (not used)
     * CCC.C is the pressure reading 060.0 is max.
     */
    char info[8];
    sprintf(info,"9910%i0|", level);
    pressureChar.writeValue(info);
    oldLevel = level;
  }
}
