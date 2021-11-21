#ifndef __M5GFX_M5UNITOLED__
#define __M5GFX_M5UNITOLED__

// If you want to use a set of functions to handle SD/SPIFFS/HTTP,
//  please include <SD.h>,<SPIFFS.h>,<HTTPClient.h> before <M5GFX.h>
// #include <SD.h>
// #include <SPIFFS.h>
// #include <HTTPClient.h>
#include "M5GFX.h"
#include "lgfx/v1/panel/Panel_SSD1306.hpp"

#if defined ( ARDUINO )
 #include <Arduino.h>
 static constexpr std::uint8_t M5_UNIT_OLED_SDA = SDA;
 static constexpr std::uint8_t M5_UNIT_OLED_SCL = SCL;
#elif defined (ESP_PLATFORM)
 #include <sdkconfig.h>
 #if defined (CONFIG_IDF_TARGET_ESP32C3)
  static constexpr std::uint8_t M5_UNIT_OLED_SDA = 1;
  static constexpr std::uint8_t M5_UNIT_OLED_SCL = 0;
 #else
  static constexpr std::uint8_t M5_UNIT_OLED_SDA = 21;
  static constexpr std::uint8_t M5_UNIT_OLED_SCL = 22;
 #endif
#else
 static constexpr std::uint8_t M5_UNIT_OLED_SDA = 21;
 static constexpr std::uint8_t M5_UNIT_OLED_SCL = 22;
#endif
static constexpr std::uint8_t M5_UNIT_OLED_ADDR = 0x3C;
static constexpr std::uint32_t M5_UNIT_OLED_FREQ = 400000;

class M5UnitOLED : public lgfx::LGFX_Device
{
  lgfx::Bus_I2C _bus_instance;
  lgfx::Panel_SH110x _panel_instance;

public:

  m5gfx::board_t getBoard(void) const { return m5gfx::board_t::board_M5UnitOLED; }

  M5UnitOLED(std::uint8_t pin_sda = M5_UNIT_OLED_SDA, std::uint8_t pin_scl = M5_UNIT_OLED_SCL, std::uint32_t i2c_freq = M5_UNIT_OLED_FREQ, std::int8_t i2c_port = -1, std::uint8_t i2c_addr = M5_UNIT_OLED_ADDR)
  {
    setup(pin_sda, pin_scl, i2c_freq, i2c_port, i2c_addr);
  }

  using lgfx::LGFX_Device::init;
  bool init(std::uint8_t pin_sda, std::uint8_t pin_scl, std::uint32_t i2c_freq = M5_UNIT_OLED_FREQ, std::int8_t i2c_port = -1, std::uint8_t i2c_addr = M5_UNIT_OLED_ADDR)
  {
    setup(pin_sda, pin_scl, i2c_freq, i2c_port, i2c_addr);
    return init();
  }

  void setup(std::uint8_t pin_sda = M5_UNIT_OLED_SDA, std::uint8_t pin_scl = M5_UNIT_OLED_SCL, std::uint32_t i2c_freq = M5_UNIT_OLED_FREQ, std::int8_t i2c_port = -1, std::uint8_t i2c_addr = M5_UNIT_OLED_ADDR)
  {
    if (i2c_port < 0)
    {
      i2c_port = 0;
#ifdef _M5EPD_H_
      if ((pin_sda == 25 && pin_scl == 32)  /// M5Paper
      {
        i2c_port = 1
      }
#endif
    }

    {
      auto cfg = _bus_instance.config();
      cfg.freq_write = i2c_freq;
      cfg.freq_read = i2c_freq;
      cfg.pin_scl = pin_scl;
      cfg.pin_sda = pin_sda;
      cfg.i2c_port = i2c_port;
      cfg.i2c_addr = i2c_addr;
      cfg.prefix_cmd = 0x00;
      cfg.prefix_data = 0x40;
      cfg.prefix_len = 1;
      _bus_instance.config(cfg);
      _panel_instance.bus(&_bus_instance);
    }
    {
      auto cfg = _panel_instance.config();
      cfg.panel_width = 64;
      cfg.offset_x = 32;
      _panel_instance.config(cfg);
    }

    setPanel(&_panel_instance);
  }
};

#endif
