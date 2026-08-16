#pragma once

// I2C for OLED display
#define I2C_SDA 6
#define I2C_SCL 7

// Buttons
#define BUTTON_PIN 0         // BUTTON 1 (boot)

// SPI (shared by LoRa and SD)
#define SPI_MOSI 10
#define SPI_SCK 9
#define SPI_MISO 11
#define SPI_CS 8

// LoRa Radio - SX1262 with 1W PA
#define USE_SX1262
#define LORA_SCK SPI_SCK
#define LORA_MISO SPI_MISO
#define LORA_MOSI SPI_MOSI
#define LORA_CS SPI_CS
#define LORA_RESET 12
#define LORA_DIO1 1
#define LORA_BUSY 13

// CRITICAL: Radio power enable - MUST be HIGH before lora.begin()!
// GPIO 5 connected to E22P RF_EN powers the SX1262 + LNA + PA module
#define LORA_EN 5

#if not defined(P_LORA_EN)
#define P_LORA_EN LORA_EN
#endif

#if defined(USE_SX1262)
#define SX126X_CS LORA_CS
#define SX126X_DIO1 LORA_DIO1
#define SX126X_BUSY LORA_BUSY
#define SX126X_RESET LORA_RESET

// RF switching configuration for E22P module
// DIO2 controls PA and LNA (via SX126X_DIO2_AS_RF_SWITCH)
// Truth table: DIO2=1,CTRL=1 -> TX (PA on, LNA off, RADIO on)
//              DIO2=0,CTRL=1 -> RX (PA off, LNA on, RADIO on)
#define SX126X_DIO2_AS_RF_SWITCH
#define SX126X_RXEN RADIOLIB_NC  // No need to control RX_EN via MCU
#define SX126X_TXEN RADIOLIB_NC  // No need to control TX_EN via MCU

// TCXO voltage - required for radio init
#define SX126X_DIO3_TCXO_VOLTAGE 1.8

#define SX126X_MAX_POWER 22
#endif

// LED
#define LED_TX 48
#if not defined(P_LORA_TX_LED)
#define P_LORA_TX_LED LED_TX
#endif

// PA Ramp Time - E22P requires >800us stabilization (default is 200us)
// Value 0x05 = RADIOLIB_SX126X_PA_RAMP_800U
#define SX126X_PA_RAMP_US 0x05

// Display - SSD1306 OLED (128x64)
#define USE_SSD1306
#define OLED_WIDTH 128
#define OLED_HEIGHT 64