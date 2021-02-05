//----------------------------------------//
//  esp32-ttnmapper-gps.ino
//
//  created 01/06/2019
//  by Luiz H. Cassettari
//----------------------------------------//
// Designed to work with esp32 lora 915MHz
// Create a device with ABP on ttn
// Create a integration with ttnmapper
//----------------------------------------//

//----------------------------------------//

#define LMIC_DEBUG_LEVEL 0

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

#include "keys.h"

#ifndef COUNTDOWN_LOG_INTERVAL
#define COUNTDOWN_LOG_INTERVAL 10000
#endif

#if LMIC_ENABLE_event_logging
extern "C" {
    void LMICOS_logEvent(const char *pMessage);
    void LMICOS_logEventUint32(const char *pMessage, uint32_t datum);
}

void LMICOS_logEvent(const char *pMessage)
    {
      Serial.printf("LMIC EVENT %s\n", pMessage);
    }

void LMICOS_logEventUint32(const char *pMessage, uint32_t datum)
    {
      Serial.printf("LMIC EVENT %s 0x%08lx\n", pMessage, datum);
    }
#endif // LMIC_ENABLE_event_logging

//
// Try to send every 10 seconds.
// But, if the LoraWAN band-plan limits the transmit duty-cycle, honour that limit
//
#define SEND_TIMER 10

// Different flash rates for the built in LED
// Fast flash - sending
// Slow flash - no GPS lock
#define LED_OFF 0
#define LED_SENDING 100
#define LED_NOGPS 1000

#define LORA_HTOI(c) ((c<='9')?(c-'0'):((c<='F')?(c-'A'+10):((c<='f')?(c-'a'+10):(0))))
#define LORA_TWO_HTOI(h, l) ((LORA_HTOI(h) << 4) + LORA_HTOI(l))
#define LORA_HEX_TO_BYTE(a, h, n) { for (int i = 0; i < n; i++) (a)[i] = LORA_TWO_HTOI(h[2*i], h[2*i + 1]); }
#define LORA_DEVADDR(a) (uint32_t) ((uint32_t) (a)[3] | (uint32_t) (a)[2] << 8 | (uint32_t) (a)[1] << 16 | (uint32_t) (a)[0] << 24)

// These are initialised from keys.h, which you should copy from keys-example.h
static uint8_t DEVADDR[4];
static uint8_t NWKSKEY[16];
static uint8_t APPSKEY[16];

#include "pins.h"
#ifndef PIN_LED
#define PIN_LED LED_BUILTIN
#endif

void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

static osjob_t sendjob;

int led = LED_NOGPS;

void set_led(int ms)
{
  //Serial.print("set_led ");
  //Serial.println(ms);
  led = ms;
}


void onEvent (ev_t ev) {
  Serial.print(os_getTime());
  Serial.print(": ");
  switch (ev) {
  case EV_SCAN_TIMEOUT:
    oled_status(" - SCAN_TIMEOUT - ");
    break;
  case EV_SCAN_FOUND:
    oled_status(" - SCAN_FOUND - ");
    break;
  case EV_BEACON_FOUND:
    oled_status(" - BEACON_FOUND - ");
    break;
  case EV_BEACON_MISSED:
    oled_status(" - BEACON_MISSED - ");
    break;
  case EV_BEACON_TRACKED:
    oled_status(" - BEACON_TRACKED - ");
    break;
  case EV_JOINING:
    oled_status(" --- JOINING --- ");
    break;
  case EV_JOINED:
    oled_status(" --- JOINED --- ");
    break;
  case EV_JOIN_FAILED:
    oled_status(" --- JOIN_FAIL --- ");
    break;
  case EV_REJOIN_FAILED:
    oled_status(" -- REJOIN_FAIL -- ");
    break;
  case EV_LOST_TSYNC:
    oled_status(" -- LOST_TSYNC -- ");
    break;
  case EV_RESET:
    oled_status(" --- RESET --- ");
    break;
  case EV_RXCOMPLETE:
    oled_status(" --- RXCOMPLETE --- ");
    break;
  case EV_LINK_DEAD:
    oled_status(" --- LINK_DEAD --- ");
    break;
  case EV_LINK_ALIVE:
    oled_status(" --- LINK_ALIVE --- ");
    break;
  case EV_TXCOMPLETE:
    oled_status(" --- TXCOMPLETE --- ");
    Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
    set_led(LED_OFF);
    if (LMIC.txrxFlags & TXRX_ACK)
    {
      Serial.println(F("Received ack"));
    }
    if (LMIC.dataLen != 0 || LMIC.dataBeg != 0) {
      uint8_t port = 0;
      if (LMIC.txrxFlags & TXRX_PORT)
      {
	port = LMIC.frame[LMIC.dataBeg - 1];
      }
      message(LMIC.frame + LMIC.dataBeg, LMIC.dataLen , port);
    }
#if defined(CFG_as923) && defined(OVERRIDE_BANDPLAN_DUTY_CYCLE)
    //
    // AS923 defines strict 1% duty cycle, so we ought to wait about
    // 140 seconds between sends, else we will queue a packet with a stale location
    // that may go on the air many minutes later
    //
    // If you set OVERRIDE_BANDPLAN_DUTY_CYCLE, this code will, well,
    // override the bandplan duty cycle, and let you transmit more frequently.
    //
    // WARNING: this is naughty hack, and should ONLY be used for bona-fide
    // vehicle-based network mapping.
    //
    // Here, we reach into the guts of the LMIC engine and defeat the band plan rate
    // limiting, to allow closer samples from fast-moving vehicles.
    //
    {
      ostime_t next_send = os_getTime();
      LMIC.opmode &= ~OP_NEXTCHNL;
      LMICbandplan_updateTx(next_send);
      LMIC.globalDutyAvail = next_send;
    }
#else
    os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(SEND_TIMER), do_send);
#endif

    break;
  case EV_TXSTART:
    oled_status(" --- TXSTART --- ");
    Serial.println(F("EV_TXSTART"));
    break;
  case EV_TXCANCELED:
    oled_status(" -- TXCANCELED -- ");
    break;
  case EV_RXSTART:
    oled_status(" --- RXSTART --- ");
    break;
  case EV_JOIN_TXCOMPLETE:
    oled_status("- JOIN_TXCOMPLETE -");
    break;
  }
}



void do_send(osjob_t* j) {
  // Check if there is not a current TX/RX job running
#if LMIC_DEBUG_LEVEL
  Serial.println(F("do_send"));
#endif
  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println(F("OP_TXRXPEND, not sending"));
  } else {
    if (gps_read()) {
      set_led(LED_SENDING);
    }
    else {
      set_led(LED_NOGPS);
    }
    bool sent = PayloadNow();
    if (!sent) {
      set_led(LED_NOGPS);
    }
  }
#if LMIC_DEBUG_LEVEL
  Serial.println(F("do_send done"));
#endif
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("Starting"));
#if PIN_LED != LMIC_UNUSED_PIN
  pinMode(PIN_LED, OUTPUT);
#endif

  oled_setup();
  gps_setup();
  button_setup();

  LORA_HEX_TO_BYTE(DEVADDR, devAddr, 4);
  LORA_HEX_TO_BYTE(NWKSKEY, nwkSKey, 16);
  LORA_HEX_TO_BYTE(APPSKEY, appSKey, 16);

  if (LORA_DEVADDR(DEVADDR) == 0) while(true);

#if defined(CFG_au915)
  Serial.println(F("LoRa init band au915"));
#elif defined(CFG_as923)
  Serial.println(F("LoRa init band as923"));
#else
#error Unsupported band
#endif

  os_init();

  LMIC_reset();
  LMIC_setSession (0x13, LORA_DEVADDR(DEVADDR), NWKSKEY, APPSKEY);
  LMIC_setAdrMode(0);
  LMIC_setClockError(MAX_CLOCK_ERROR * 10 / 100);
#if defined(CFG_au915) || defined(CFG_us915)
  LMIC_selectSubBand(1);
#endif
  LMIC_setLinkCheckMode(0);

  Serial.println(F("First send"));
  do_send(&sendjob);
}

boolean runEvery_countdown()
{
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= COUNTDOWN_LOG_INTERVAL)
  {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}

void countdown_loop()
{
  // The AS923 (and AFAIK EU858) band has a 1% duty cycle enforcement.
  //
  // We cant just schedule a packet and expect it to be sent immediately,
  // there may be a delay of several minutes (which means the GPS
  // coordinates could be wrong).
  //
  // Instead, we ask the engine if we are clear to send now, and only
  // perform a send if so.
#if defined(CFG_as923)
  ostime_t now = os_getTime();
  ostime_t then = LMICbandplan_nextTx(now);
  ostime_t remain = then - now;
  if (now > then) {
    do_send(NULL);
  }
  else if (runEvery_countdown()) {
      Serial.printf("Bandplan enforces wait of %lu.%03us until next send.\n",
		    (uint32_t)osticks2ms(remain)/1000, (unsigned int)(osticks2ms(remain)%1000),
		    (uint32_t)osticks2ms(now)/1000, (int)(osticks2ms(now)%1000),
		    (uint32_t)osticks2ms(then)/1000, (int)(osticks2ms(then)%1000));
  }
#endif
}

void loop() {

#if PIN_LED != LMIC_UNUSED_PIN
  if (led == LED_OFF) {
    digitalWrite(PIN_LED, 0);
  }
  else {
    unsigned long cycle = millis()/led;
    digitalWrite(PIN_LED, cycle%2);
  }
#endif

  os_runloop_once();
  gps_loop();
  oled_loop();

  if (button_loop())
  {
    oled_mode(button_mode());

    if (button_count() == 0)
      do_send(&sendjob);
    else if (button_count() == 2)
    {
      os_clearCallback(&sendjob);
      os_radio(RADIO_RST);
    }
  }

  countdown_loop();
}

void message(const uint8_t *payload, size_t size, uint8_t port)
{
  Serial.println("-- MESSAGE");
  Serial.println("Received " + String(size) + " bytes on port " + String(port) + ":");
  if (port == 0)
  {
    oled_status(" --- TX_CONFIRMED --- ");
    return;
  }
  if (size == 0) return;
  switch (port) {
    case 1:
      break;
  }
}
