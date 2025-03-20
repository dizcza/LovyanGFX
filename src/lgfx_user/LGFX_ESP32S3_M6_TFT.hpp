#pragma once

#define LGFX_USE_V1

#include <LovyanGFX.hpp>

#include <lgfx/v1/panel/Panel_ILI9342.hpp>

// Example of settings when using LovyanGFX with your own settings on ESP32

/*
Make a copy of this file, give it a new name, and modify it to suit your environment.
The created file can be made available by including it in the user program.

You can use the copied file by placing it in the lgfx_user folder in the library,
In that case, please note that it may be deleted when the library is updated.

If you want to operate safely, please make a backup or place it in the user project folder.
//*/


/// Create a class that will perform your own settings by deriving it from LGFX_Device.
class LGFX : public lgfx::LGFX_Device
{
/*
 You may change the class name from "LGFX" to something else.
 If you use it in conjunction with AUTODETECT, "LGFX" will be used, so please change the name to something other than LGFX.
 Also, if you use multiple panels at the same time, please give each panel a different name.
 * If you change the class name, you must also change the constructor name to the same name.

 You can choose how to name them, but in case the number of settings increases,
 For example, if you configure the ILI9341 for SPI connection on the ESP32 DevKit-C,
  LGFX_DevKitC_SPI_ILI9341
 By naming the file like this and keeping the file name and class name consistent, you will be less likely to get confused when using the file.
//*/


// Prepare an instance that matches the type of panel you want to connect.
//lgfx::Panel_GC9A01      _panel_instance;
//lgfx::Panel_GDEW0154M09 _panel_instance;
//lgfx::Panel_HX8357B     _panel_instance;
//lgfx::Panel_HX8357D     _panel_instance;
//lgfx::Panel_ILI9163     _panel_instance;
//lgfx::Panel_ILI9341     _panel_instance;
lgfx::Panel_ILI9342     _panel_instance;
//lgfx::Panel_ILI9481     _panel_instance;
//lgfx::Panel_ILI9486     _panel_instance;
//lgfx::Panel_ILI9488     _panel_instance;
//lgfx::Panel_IT8951      _panel_instance;
//lgfx::Panel_LS044Q7DH01 _panel_instance;
//lgfx::Panel_LS032B7DD02 _panel_instance;
//lgfx::Panel_LS027B7DH01 _panel_instance;
//lgfx::Panel_LS018B7DH02 _panel_instance;
//lgfx::Panel_LS013B7DH05 _panel_instance;
//lgfx::Panel_LS013B7DH03 _panel_instance;
//lgfx::Panel_LS012B7DD01 _panel_instance;
//lgfx::Panel_LS011B7DH03 _panel_instance;
//lgfx::Panel_LS010B7DH04 _panel_instance;
//lgfx::Panel_RA8875      _panel_instance;
//lgfx::Panel_SH110x      _panel_instance; // SH1106, SH1107
//lgfx::Panel_SSD1306     _panel_instance;
//lgfx::Panel_SSD1327     _panel_instance;
//lgfx::Panel_SSD1331     _panel_instance;
//lgfx::Panel_SSD1351     _panel_instance; // SSD1351, SSD1357
//lgfx::Panel_SSD1963     _panel_instance;
//lgfx::Panel_ST7735      _panel_instance;
//lgfx::Panel_ST7735S     _panel_instance;
//lgfx::Panel_ST7789      _panel_instance;
//lgfx::Panel_ST7796      _panel_instance;

// Prepare an instance that matches the type of bus to which the panel will be connected.
  lgfx::Bus_SPI _bus_instance; // SPI bus instance
//lgfx::Bus_I2C _bus_instance; // I2C bus instance (ESP32 only)
//lgfx::Bus_Parallel8 _bus_instance; // 8-bit parallel bus instance (ESP32 only)

// If backlight control is possible, prepare an instance. (Delete if not required)
  lgfx::Light_PWM     _light_instance;

// Prepare an instance that matches the touch screen type. (Delete if not needed)
//lgfx::Touch_CST816S          _touch_instance;
//lgfx::Touch_FT5x06           _touch_instance; // FT5206, FT5306, FT5406, FT6206, FT6236, FT6336, FT6436
//lgfx::Touch_GSL1680E_800x480 _touch_instance; // GSL_1680E, 1688E, 2681B, 2682B
//lgfx::Touch_GSL1680F_800x480 _touch_instance;
//lgfx::Touch_GSL1680F_480x272 _touch_instance;
//lgfx::Touch_GSLx680_320x320  _touch_instance;
//lgfx::Touch_GT911            _touch_instance;
//lgfx::Touch_STMPE610         _touch_instance;
//lgfx::Touch_TT21xxx          _touch_instance; // TT21100
//lgfx::Touch_XPT2046          _touch_instance;

public:

  // Create a constructor and set various settings here.
  // If you change the class name, specify the same name for the constructor.
  LGFX(void)
  {
    { // Set up the bus control.
      auto cfg = _bus_instance.config(); // Get the structure for the bus configuration.

// Configure the SPI bus
      cfg.spi_host = SPI2_HOST; // Use SPI to select ESP32-S2,C3: SPI2_HOST or SPI3_HOST / ESP32: VSPI_HOST or HSPI_HOST
      // * Due to the ESP-IDF version upgrade, the VSPI_HOST and HSPI_HOST descriptions are deprecated, so if an error occurs, please use SPI2_HOST and SPI3_HOST instead.
      cfg.spi_mode = 0; // Set SPI communication mode (0 to 3)
      cfg.freq_write = 16000000; // SPI clock when transmitting (maximum 80MHz, rounded to an integer division of 80MHz)
      cfg.freq_read = 16000000; // SPI clock when receiving
      cfg.spi_3wire = true; // Set true if you want to receive via the MOSI pin
      cfg.use_lock = true; // Set true if you want to use transaction locking
      cfg.dma_channel = SPI_DMA_CH_AUTO; // Set the DMA channel to use (0=DMA not used / 1=1ch / 2=ch / SPI_DMA_CH_AUTO=automatic setting)
      // * Due to the ESP-IDF version upgrade, SPI_DMA_CH_AUTO (automatic setting) is now recommended for the DMA channel. Specifying 1ch or 2ch is no longer recommended.
      cfg.pin_sclk = 40; // Set the SPI SCLK pin number
      cfg.pin_mosi = 39; // Set the SPI MOSI pin number
      cfg.pin_miso = -1; // Set the MISO pin number for SPI (-1 = disable)
      cfg.pin_dc = 38; // Set SPI D/C pin number (-1 = disable)
     // If you use a common SPI bus with the SD card, be sure to set MISO and do not omit it.
      _bus_instance.config(cfg); // Reflect the setting value to the bus.
      _panel_instance.setBus(&_bus_instance); // Set the bus to the panel.
    }

    { // Set up the display panel control.
      auto cfg = _panel_instance.config(); // Get the structure for the display panel settings.

      cfg.pin_cs = 41; // Pin number to which CS is connected (-1 = disable)
      cfg.pin_rst = 5; // Pin number to which RST is connected (-1 = disable)
      cfg.pin_busy = -1; // Pin number to which BUSY is connected (-1 = disable)

      // * The following settings are set to general initial values ​​for each panel, so try commenting out any items you are unsure of.

      // cfg.panel_width = 320; // Actual displayable width
      // cfg.panel_height = 240; // Actual displayable height
      // cfg.offset_x = 0; // Panel X-direction offset
      // cfg.offset_y = 0; // Panel offset in the Y direction
      cfg.offset_rotation = 0; // Rotation offset value 0~7 (4~7 are upside down)
      cfg.dummy_read_pixel = 8; // Number of bits to dummy read before pixel read
      cfg.dummy_read_bits = 1; // Number of dummy read bits before reading non-pixel data
      cfg.readable = true; // Set to true if data can be read
      cfg.invert = false; // Set to true if the panel's light and dark colors are inverted
      cfg.rgb_order = false; // Set to true if the red and blue of the panel are swapped
      cfg.dlen_16bit = false; // Set to true for panels that transmit data length in 16-bit units via 16-bit parallel or SPI
      cfg.bus_shared = false; // Set to true if the bus is shared with the SD card (bus control is performed using drawJpgFile, etc.)

// Set the following only if the display is misaligned when using a driver with a variable pixel count, such as ST7735 or ILI9163.
      // cfg.memory_width = 320; // Maximum width supported by the driver IC
      // cfg.memory_height = 240; // Maximum height supported by the driver IC

      _panel_instance.config(cfg);
    }

    { // Set the backlight control. (Delete if not required)
      auto cfg = _light_instance.config(); // Get the structure for the backlight settings.

      cfg.pin_bl = 4; // Pin number to which the backlight is connected
      cfg.invert = false; // true to invert the backlight brightness
      cfg.freq = 44100; // Backlight PWM frequency
      cfg.pwm_channel = 7; // PWM channel number to use

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance); // Set the backlight on the panel.
    }
//

    setPanel(&_panel_instance); // Set the panel to use.
  }
};
