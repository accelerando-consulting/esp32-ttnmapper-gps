// copy this file to keys.h and edit it

// Get the key information from your TTN console's device record and insert
// it here, conforming to this example:
//
const char *devAddr = "2******F";
const char *nwkSKey = "2F****************************66";
const char *appSKey = "AB****************************8F";

/*
 * If you want to wardrive with multiple mappers on multiple bands
 * (for example, in Brisbane, Australia, TTN uses au915 and as923)
 * you can use a different device ID for each, as below:
 *
 *  #if defined(CFG_au915)
 *  //my-mapper-01 on AU915 band
 *  const char *devAddr = "2******F";
 *  const char *nwkSKey = "2F****************************66";
 *  const char *appSKey = "AB****************************8F";
 *
 *  #elif defined(CFG_as923)
 *  //my-mapper-02 on AS923 band
 *  const char *devAddr = "2******C";
 *  const char *nwkSKey = "73****************************82";
 *  const char *appSKey = "8E****************************EA";
 *  #else
 *  #error No key for this band
 *  #endif
 **/
