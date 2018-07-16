#include "SX1231.h"

SX1231::SX1231(Module* mod) : RF69(mod) {

}

uint8_t SX1231::begin(float freq = 434.0, float br = 48.0, float rxBw = 125.0, float freqDev = 50.0, int8_t power = 13) {
  // set module properties
  _mod->init(USE_SPI, INT_BOTH);
  
  // try to find the SX1231 chip
  uint8_t i = 0;
  bool flagFound = false;
  while((i < 10) && !flagFound) {
    uint8_t version = _mod->SPIreadRegister(RF69_REG_VERSION);
    if((version == 0x21) || (version == 0x22) || (version == 0x23)) {
      flagFound = true;
      _chipRevision = version;
    } else {
      #ifdef KITELIB_DEBUG
        Serial.print("SX1231 not found! (");
        Serial.print(i + 1);
        Serial.print(" of 10 tries) RF69_REG_VERSION == ");
        
        char buffHex[5];
        sprintf(buffHex, "0x%02X", version);
        Serial.print(buffHex);
        Serial.println();
      #endif
      delay(1000);
      i++;
    }
  }
  
  if(!flagFound) {
    DEBUG_PRINTLN_STR("No SX1231 found!");
    SPI.end();
    return(ERR_CHIP_NOT_FOUND);
  } else {
    DEBUG_PRINTLN_STR("Found SX1231!");
  }
  
  // configure settings not accessible by API
  uint8_t state = config();
  if(state != ERR_NONE) {
    return(state);
  }
  
  // configure publicly accessible settings
  state = setFrequency(freq);
  if(state != ERR_NONE) {
    return(state);
  }
  
  _rxBw = 125.0;
  state = setBitRate(br);
  if(state != ERR_NONE) {
    return(state);
  }
  
  state = setRxBandwidth(rxBw);
  if(state != ERR_NONE) {
    return(state);
  }
  
  state = setFrequencyDeviation(freqDev);
  if(state != ERR_NONE) {
    return(state);
  }
  
  state = setOutputPower(power);
  if(state != ERR_NONE) {
    return(state);
  }
  
  // default sync word values 0x2D01 is the same as the default in LowPowerLab RFM69 library
  uint8_t syncWord[] = {0x2D, 0x01};
  state = setSyncWord(syncWord, 2);
  if(state != ERR_NONE) {
    return(state);
  }
  
  // SX1231 V2a only
  if(_chipRevision == SX1231_CHIP_REVISION_2_A) {
    // modify default OOK threshold value
    state = _mod->SPIsetRegValue(SX1231_REG_TEST_OOK, SX1231_OOK_DELTA_THRESHOLD);
    if(state != ERR_NONE) {
      return(state);
    }
    
    // enable OCP with 95 mA limit
    state = _mod->SPIsetRegValue(RF69_REG_OCP, RF69_OCP_ON | RF69_OCP_TRIM, 4, 0);
    if(state != ERR_NONE) {
      return(state);
    }
  }
  
  return(ERR_NONE);
}
