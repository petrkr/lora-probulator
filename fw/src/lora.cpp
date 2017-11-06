#include "lora.h"

#include "hal.h"

#include "Arduino.h"
#include "core.h"

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

#include "lora_keys.h"

void do_send(osjob_t* j);

//////////////////////////////////////////////////
// APPLICATION CALLBACKS
//////////////////////////////////////////////////

// provide application router ID (8 bytes, LSBF)
void os_getArtEui (u1_t* buf) {
    memcpy(buf, APPEUI, 8);
}

// provide device ID (8 bytes, LSBF)
void os_getDevEui (u1_t* buf) {
    memcpy(buf, DEVEUI, 8);
}

static const u1_t PROGMEM APPKEY[16] = { 0xB4, 0xD0, 0x4F, 0x2B, 0x8D, 0x2A, 0xA6, 0xEB, 0x53, 0xAF, 0x5A, 0x19, 0x3C, 0x06, 0xF9, 0x33 };


// provide device key (16 bytes)
void os_getDevKey (u1_t* buf) {
    memcpy(buf, APPKEY, 16);
}

//uint8_t mydata[] = "Hello ESP8266 world!";
static osjob_t sendjob;

// Pin mapping
// XXX We have to see whether all these pins are really used
// if not, we can use them for real sensor work.
lmic_pinmap pins = {
    .nss = PIN_RFM95_NSS,      // Make D8/GPIO15, is nSS on ESP8266
    .rxtx = 0xff,      // D4/GPIO2. For placeholder only,
              // Do not connected on RFM92/RFM95
    .rst = 0xff,       // Make D0/GPIO16, Needed on RFM92/RFM95? (probably not)
    .dio = {PIN_RFM95_DIO0, PIN_RFM95_DIO1, 0xff},    // Specify pin numbers for DIO0, 1, 2
              // D1/GPIO5,D2/GPIO4,D3/GPIO3 are usable pins on ESP8266
              // NOTE: D3 not really usable when UART not connected
              // As it is used during bootup and will probably not boot.
              // Leave D3 Pin unconnected for sensor to work
  };


void onEvent(ev_t ev) {

  switch (ev) {
  case EV_SCAN_TIMEOUT:
    Serial.println(F("EV_SCAN_TIMEOUT"));
    break;
  case EV_BEACON_FOUND:
    Serial.println(F("EV_BEACON_FOUND"));
    break;
  case EV_BEACON_MISSED:
    Serial.println(F("EV_BEACON_MISSED"));
    break;
  case EV_BEACON_TRACKED:
    Serial.println(F("EV_BEACON_TRACKED"));
    break;
  case EV_JOINING:
    Serial.println(F("EV_JOINING"));
    break;
  case EV_JOINED:
    Serial.println(F("EV_JOINED"));

    // Disable link check validation (automatically enabled
    // during join, but not supported by TTN at this time).
  //  LMIC_setLinkCheckMode(0);
    break;
  case EV_RFU1:
    Serial.println(F("EV_RFU1"));
    break;
  case EV_JOIN_FAILED:
    Serial.println(F("EV_JOIN_FAILED"));
    break;
  case EV_REJOIN_FAILED:
    Serial.println(F("EV_REJOIN_FAILED"));
    break;
    break;
  case EV_TXCOMPLETE:
    Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));

      screenMgr.change(&rxs);

    /*
    if (LMIC.txrxFlags & TXRX_ACK) {
      
      Serial.println(F("Received ack"));
      Serial.println();
      printf("RSSI: %d \n", LMIC.rssi);
      printf("SNR: %d \n", LMIC.snr);
      printf("Freq: %d \n", LMIC.freq);
      printf("TXpow: %d dBm\n", LMIC.txpow);
      printf("globalDutyRate: %d \n", LMIC.globalDutyRate);
      printf("globalDutyAvail: %d \n", LMIC.globalDutyAvail);
      printf("datarate: %d \n", LMIC.datarate);
      ostime_t airtime = calcAirTime(LMIC.rps, LMIC.dataLen);
      printf("Air time: %d.", airtime / 100);
      printf("%d ms\n", airtime % 100);

    }

    if (LMIC.dataLen) {
      // data received in rx slot after tx
      Serial.print(F("Received "));
      Serial.print(LMIC.dataLen);
      Serial.print(F(" bytes of payload: 0x"));
      for (int i = 0; i < 50; i++)
      {
        digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
        delay(20);                       
        digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
        delay(20);

      }
      for (int i = 0; i < LMIC.dataLen; i++) 
      {
        if (LMIC.frame[LMIC.dataBeg + i] < 0x10) 
        {
          Serial.print(F("0"));
        }
        Serial.print(LMIC.frame[LMIC.dataBeg + i], HEX);
      }
      

      Serial.println();
    }
    else
    {
     Serial.print(F("No data received "));
    }
    */
     // Schedule next transmission
    //mydata[0] = 170 + os_getRndU1() % 80;
    //mydata[1] = 50 + os_getRndU1() % 50;
    //os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(0), do_send);

    break;
  case EV_LOST_TSYNC:
    Serial.println(F("EV_LOST_TSYNC"));
    break;
  case EV_RESET:
    Serial.println(F("EV_RESET"));
    break;
  case EV_RXCOMPLETE:
    // data received in ping slot
    Serial.println(F("EV_RXCOMPLETE"));
    break;
  case EV_LINK_DEAD:
    Serial.println(F("EV_LINK_DEAD"));
    break;
  case EV_LINK_ALIVE:
    Serial.println(F("EV_LINK_ALIVE"));
    break;
  default:
    Serial.println(F("Unknown event"));
    break;
  }
}

void do_send(osjob_t* j){
      Serial.print("Time: ");
      Serial.println(millis() / 1000);
      // Show TX channel (channel numbers are local to LMIC)
      Serial.print("Send, txCnhl: ");
      Serial.println(LMIC.txChnl);
      Serial.print("Opmode check: ");
      // Check if there is not a current TX/RX job running
    if (LMIC.opmode & (1 << 7)) 
    {
      Serial.println("OP_TXRXPEND, not sending");
    } 
    else 
    {
      Serial.println("ok");
      // Prepare upstream data transmission at the next possible time.
      //LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 1);
    }

    /*
    // Schedule a timed job to run at the given timestamp (absolute system time)
    os_setTimedCallback(j, os_getTime()+sec2osticks(10), do_send);
    */     
}



void lora_setup() {
    
    os_init();

    lora_reset();

    Serial.println("os_init() finished");
    
    


          //do_send(&sendjob);
}

void lora_reset() {
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();
    Serial.println("LMIC_reet() finished");

    // Set static session parameters. Instead of dynamically establishing a session 
    // by joining the network, precomputed session parameters are be provided.
    LMIC_setSession (0x1, DEVADDR, (uint8_t*)NWKSKEY, (uint8_t*)APPSKEY);
    Serial.println("LMIC_setSession() finished");

    // Disable data rate adaptation
    // LMIC_setAdrMode(0);
    Serial.println("LMICsetAddrMode() finished");

    // Disable link check validation
    LMIC_setLinkCheckMode(0);
    // Disable beacon tracking
    //LMIC_disableTracking ();
    // Stop listening for downstream data (periodical reception)
    //LMIC_stopPingable();
      // Disable link check validation
    // TTN uses SF12 for its RX2 window.
    LMIC.dn2Dr = DR_SF12;

    // Set data rate and transmit power (note: txpow seems to be ignored by the library)
    LMIC_setDrTxpow(DR_SF7, 14);
    //
    //Serial.flush();
    Serial.println("Init done");
}

void saveParam(u4_t value, char *name)
{
//   if(strcmp(name, "seqnoUp") == 0){
//       Serial.println("updating seq");
//       EEPROM.put(0, value);
//       EEPROM.commit();
//   }
}


void lora_loop() {
    os_runloop_once();
}

void lora_off() {
    LMIC_shutdown();
}