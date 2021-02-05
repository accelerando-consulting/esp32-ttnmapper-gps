//----------------------------------------//
//  lmic_payload.ino
//
//  created 03/06/2019
//  by Luiz Henrique Cassettari
//----------------------------------------//

uint8_t txBuffer[9];
uint32_t LatitudeBinary, LongitudeBinary;
uint16_t altitudeGps;
uint8_t hdopGps;

bool PayloadNow()
{
#if LMIC_DEBUG_LEVEL > 0
  Serial.println(F("PayloadNow"));
#endif
  boolean confirmed = false;
  lmic_tx_error_t err;
  bool result = false;
  static const char *tx_error_name[] = {LMIC_ERROR_NAME__INIT};
  
  if (button_count() == 1) confirmed = true;
  
  if (gps_read()) {
    float lat = gps_latitude();
    float lon = gps_longitude();
    float alt = gps_meters();
    //Serial.printf("GPS: %f,%f alt=%f\n", lat, lon, alt);

    LatitudeBinary = ((lat + 90) / 180) * 16777215;
    LongitudeBinary = ((lon + 180) / 360) * 16777215;

    txBuffer[0] = ( LatitudeBinary >> 16 ) & 0xFF;
    txBuffer[1] = ( LatitudeBinary >> 8 ) & 0xFF;
    txBuffer[2] = LatitudeBinary & 0xFF;

    txBuffer[3] = ( LongitudeBinary >> 16 ) & 0xFF;
    txBuffer[4] = ( LongitudeBinary >> 8 ) & 0xFF;
    txBuffer[5] = LongitudeBinary & 0xFF;

    altitudeGps = alt;
    txBuffer[6] = ( altitudeGps >> 8 ) & 0xFF;
    txBuffer[7] = altitudeGps & 0xFF;

    hdopGps = gps_HDOP() * 10;
    txBuffer[8] = hdopGps & 0xFF;

    err = LMIC_setTxData2(1, txBuffer, sizeof(txBuffer), confirmed);
    result = true;
  }
  else
  {
    err = LMIC_setTxData2(1, txBuffer, 0, confirmed);
  }

  if (err != 0) {
    Serial.printf("LMIC transmit error %d (%s)\n", err, (err>=LMIC_ERROR_TX_FAILED)?tx_error_name[-err]:"unknown");
    return false;
  }
  
  if (result) {
#if LMIC_DEBUG_LEVEL > 0
    Serial.println(F("PayloadNow: sent location"));
#endif
  }
  else {
    Serial.println(F("PayloadNow: did not send (no GPS lock)"));
  }

  return result;
}
