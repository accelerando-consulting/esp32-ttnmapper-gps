// Edit this file if the LoRa pins for your board aren't automatically
// detected

// If you want to use a different LED than LED_BUILTIN, define PIN_LED here
// #define PIN_LED ??

// If you want to use different GPS pins than defaults, define here
// #define GPS_PIN_RX ??
// #define GPS_PIN_TX ??

#if defined(ARDUINO_TTGO_LoRa32_V1)
// If you have the Lora32 then arduino-lmic already defines a correct pinmap

#elif defined(ARDUINO_HELTEC_WIFI_LORA32)
// If you have the Heltec Wifi Lora32 then arduino-lmic already defines a correct pinmap

#else
//#error Your board is not automatically supported, delete this line and define your custom pins below

// LoRa module pin mapping, replace LMIC_UNUSED_PIN with your pin, where relevant
// (you can leave rxtx and dio2 unused, but must set everything else)
const lmic_pinmap lmic_pins = {
  .nss = LMIC_UNUSED_PIN,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = LMIC_UNUSED_PIN,
  .dio = {/*dio0*/ LMIC_UNUSED_PIN, /*dio1*/ LMIC_UNUSED_PIN, /*dio2*/ LMIC_UNUSED_PIN}
};

// If you have TTGO T-beam, uncomment the below:
//
// #define GPS_PIN_RX 12
// #define GPS_PIN_TX 15
//
// const lmic_pinmap lmic_pins = {
//   .nss = 18,
//   .rxtx = LMIC_UNUSED_PIN,
//   .rst = 23,
//   .dio = {/*dio0*/ 26, /*dio1*/ 33, /*dio2*/ 32}
// };

#endif
