#pragma once

#include <Arduino.h>
#include <helpers/ESP32Board.h>
#include <driver/rtc_io.h>

#ifndef ADC_MULTIPLIER
  #define ADC_MULTIPLIER 2.110
#endif

class DIYS3E22PSuperMiniBoard : public ESP32Board {

protected:
  float adc_mult = ADC_MULTIPLIER;

private:
  bool radio_powered = false;

public:
  void begin();
  void enterDeepSleep(uint32_t secs, int pin_wake_btn = -1);
  void powerOff() override;
  uint16_t getBattMilliVolts() override;
  bool setAdcMultiplier(float multiplier) override {
    if (multiplier == 0.0f) {
      adc_mult = ADC_MULTIPLIER;
    } else {
      adc_mult = multiplier;
    }
    return true;
  }
  float getAdcMultiplier() const override { return adc_mult; }
  const char* getManufacturerName() const override;
};
