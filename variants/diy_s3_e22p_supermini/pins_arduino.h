#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>

#define USB_VID 0x303a
#define USB_PID 0x1001

// Serial (USB CDC)
static const uint8_t TX = 43;
static const uint8_t RX = 44;

// I2C for OLED and sensors
static const uint8_t SDA = 6;
static const uint8_t SCL = 7;

// Default SPI mapped to Radio/SD
static const uint8_t SS = 8;   // LoRa CS
static const uint8_t MOSI = 10;
static const uint8_t MISO = 11;
static const uint8_t SCK = 9;

#endif /* Pins_Arduino_h */