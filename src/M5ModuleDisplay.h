#ifndef __M5GFX_M5MODULEDISPLAY__
#define __M5GFX_M5MODULEDISPLAY__

// If you want to use a set of functions to handle SD/SPIFFS/HTTP,
//  please include <SD.h>,<SPIFFS.h>,<HTTPClient.h> before <M5GFX.h>
// #include <SD.h>
// #include <SPIFFS.h>
// #include <HTTPClient.h>

#include "lgfx/v1/panel/Panel_M5HDMI.hpp"
#include "M5GFX.h"

#include <sdkconfig.h>
#include <soc/efuse_reg.h>

#ifndef M5MODULEDISPLAY_LOGICAL_WIDTH
#define M5MODULEDISPLAY_LOGICAL_WIDTH 1280
#endif
#ifndef M5MODULEDISPLAY_LOGICAL_HEIGHT
#define M5MODULEDISPLAY_LOGICAL_HEIGHT 720
#endif
#ifndef M5MODULEDISPLAY_REFRESH_RATE
#define M5MODULEDISPLAY_REFRESH_RATE 0.0f
#endif
#ifndef M5MODULEDISPLAY_OUTPUT_WIDTH
#define M5MODULEDISPLAY_OUTPUT_WIDTH 0
#endif
#ifndef M5MODULEDISPLAY_OUTPUT_HEIGHT
#define M5MODULEDISPLAY_OUTPUT_HEIGHT 0
#endif
#ifndef M5MODULEDISPLAY_SCALE_W
#define M5MODULEDISPLAY_SCALE_W 0
#endif
#ifndef M5MODULEDISPLAY_SCALE_H
#define M5MODULEDISPLAY_SCALE_H 0
#endif

class M5ModuleDisplay : public lgfx::LGFX_Device
{
  lgfx::Panel_M5HDMI _panel_instance;
  lgfx::Bus_SPI      _bus_instance;

public:

  M5ModuleDisplay( uint16_t logical_width  = M5MODULEDISPLAY_LOGICAL_WIDTH
                 , uint16_t logical_height = M5MODULEDISPLAY_LOGICAL_HEIGHT
                 , float refresh_rate      = M5MODULEDISPLAY_REFRESH_RATE
                 , uint16_t output_width   = M5MODULEDISPLAY_OUTPUT_WIDTH
                 , uint16_t output_height  = M5MODULEDISPLAY_OUTPUT_HEIGHT
                 , uint_fast8_t scale_w    = M5MODULEDISPLAY_SCALE_W
                 , uint_fast8_t scale_h    = M5MODULEDISPLAY_SCALE_H
                 )
  {
    lgfx::Panel_M5HDMI::config_resolution_t cfg_reso;
    cfg_reso.logical_width  = logical_width;
    cfg_reso.logical_height = logical_height;
    cfg_reso.refresh_rate   = refresh_rate;
    cfg_reso.output_width   = output_width;
    cfg_reso.output_height  = output_height;
    cfg_reso.scale_w        = scale_w;
    cfg_reso.scale_h        = scale_h;
    _panel_instance.config_resolution(cfg_reso);

    setPanel(&_panel_instance);
    _board = lgfx::board_t::board_M5ModuleDisplay;
  }

  bool init_impl(bool use_reset, bool use_clear) override
  {
#if defined (CONFIG_IDF_TARGET_ESP32S3)

    // for CoreS3
    int i2c_port = 1;
    int i2c_sda  = GPIO_NUM_12;
    int i2c_scl  = GPIO_NUM_11;
    int spi_cs   = GPIO_NUM_7;
    int spi_mosi = GPIO_NUM_37;
    int spi_miso = GPIO_NUM_35;
    int spi_sclk = GPIO_NUM_36;
    spi_host_device_t spi_host = SPI2_HOST;

    m5gfx::pinMode(GPIO_NUM_0, m5gfx::pin_mode_t::output);
    m5gfx::gpio_lo(GPIO_NUM_0);

#elif !defined (CONFIG_IDF_TARGET) || defined (CONFIG_IDF_TARGET_ESP32)

    int i2c_port = 1;
    int i2c_sda  = GPIO_NUM_25;
    int i2c_scl  = GPIO_NUM_21;
    int spi_cs   = GPIO_NUM_33;
    int spi_mosi = GPIO_NUM_19;
    int spi_miso = GPIO_NUM_22;
    int spi_sclk = GPIO_NUM_5;
    spi_host_device_t spi_host = VSPI_HOST;

    std::uint32_t pkg_ver = lgfx::get_pkg_ver();
    ESP_LOGD("LGFX","pkg:%d", pkg_ver);

    switch (pkg_ver)
    {
    case EFUSE_RD_CHIP_VER_PKG_ESP32PICOD4: // for MODULE Lite / Matrix
      ESP_LOGD("LGFX","AtomDisplay Lite");
      spi_sclk = GPIO_NUM_23;
      break;

    case 6: // EFUSE_RD_CHIP_VER_PKG_ESP32PICOV3_02: // MODULE PSRAM
      ESP_LOGD("LGFX","AtomDisplay PSRAM");
      spi_sclk = GPIO_NUM_5;
      break;

    default:

      m5gfx::pinMode(GPIO_NUM_0, m5gfx::pin_mode_t::output);
      m5gfx::gpio_lo(GPIO_NUM_0);

      { // ModuleDisplay
        spi_mosi = GPIO_NUM_23;
        spi_sclk = GPIO_NUM_18;
        if (0x03 == m5gfx::i2c::readRegister8(1, 0x34, 0x03, 400000))
        { // M5Stack Core2 / Tough
          ESP_LOGD("LGFX","DisplayModule with Core2/Tough");
          i2c_port =  1;
          i2c_sda  = GPIO_NUM_21;
          i2c_scl  = GPIO_NUM_22;
          spi_cs   = GPIO_NUM_19;
          spi_miso = GPIO_NUM_38;
        }
        else
        { // M5Stack BASIC / FIRE / GO
          ESP_LOGD("LGFX","DisplayModule with Core Basic/Fire/Go");
          i2c_port =  0;
          i2c_sda  = GPIO_NUM_21;
          i2c_scl  = GPIO_NUM_22;
          spi_cs   = GPIO_NUM_13;
          spi_miso = GPIO_NUM_19;
        }
      }
      break;
    }
#endif

    {
      auto cfg = _bus_instance.config();
      cfg.freq_write = 80000000;
      cfg.freq_read  = 20000000;
      cfg.spi_host = spi_host;
      cfg.spi_mode = 3;
      cfg.dma_channel = 1;
      cfg.use_lock = true;
      cfg.pin_mosi = spi_mosi;
      cfg.pin_miso = spi_miso;
      cfg.pin_sclk = spi_sclk;
      cfg.spi_3wire = false;

      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }

    {
      auto cfg = _panel_instance.config_transmitter();
      cfg.freq_read = 400000;
      cfg.freq_write = 400000;
      cfg.pin_scl = i2c_scl;
      cfg.pin_sda = i2c_sda;
      cfg.i2c_port = i2c_port;
      cfg.i2c_addr = 0x39;
      cfg.prefix_cmd = 0x00;
      cfg.prefix_data = 0x00;
      cfg.prefix_len = 0;
      _panel_instance.config_transmitter(cfg);
    }

    {
      auto cfg = _panel_instance.config();
      cfg.offset_rotation = 3;
      cfg.pin_cs     = spi_cs;
      cfg.readable   = false;
      cfg.bus_shared = false;
      _panel_instance.config(cfg);
      _panel_instance.setRotation(1);
    }

    return LGFX_Device::init_impl(use_reset, use_clear);
  }

  bool setResolution( uint16_t logical_width  = M5MODULEDISPLAY_LOGICAL_WIDTH
                    , uint16_t logical_height = M5MODULEDISPLAY_LOGICAL_HEIGHT
                    , float refresh_rate      = M5MODULEDISPLAY_REFRESH_RATE
                    , uint16_t output_width   = M5MODULEDISPLAY_OUTPUT_WIDTH
                    , uint16_t output_height  = M5MODULEDISPLAY_OUTPUT_HEIGHT
                    , uint_fast8_t scale_w    = M5MODULEDISPLAY_SCALE_W
                    , uint_fast8_t scale_h    = M5MODULEDISPLAY_SCALE_H
                    )
  {
    bool res = _panel_instance.setResolution
      ( logical_width
      , logical_height
      , refresh_rate
      , output_width
      , output_height
      , scale_w
      , scale_h
      );
    setRotation(getRotation());
    return res;
  }
};

#endif
