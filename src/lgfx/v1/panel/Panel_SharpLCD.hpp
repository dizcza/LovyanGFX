/*----------------------------------------------------------------------------/
  Lovyan GFX - Graphics library for embedded devices.

Original Source:
 https://github.com/lovyan03/LovyanGFX/

Licence:
 [FreeBSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)

Author:
 [lovyan03](https://twitter.com/lovyan03)

Contributors:
 [ciniml](https://github.com/ciniml)
 [mongonta0716](https://github.com/mongonta0716)
 [tobozo](https://github.com/tobozo)
 [UT2UH](https://github.com/UT2UH)
/----------------------------------------------------------------------------*/
#pragma once

#include "Panel_HasBuffer.hpp"
#include "../misc/range.hpp"
#include "../platforms/common.hpp"


#define SWAP8(a) ((((a) & 0x80) >> 7) | (((a) & 0x40) >> 5) | (((a) & 0x20) >> 3) | (((a) & 0x10) >> 1) | (((a) & 0x08) << 1) | (((a) & 0x04) << 3) | (((a) & 0x02) << 5) | (((a) & 0x01) << 7))
#define SWAP2(a) ((((a) & 0x10) >> 1) | (((a) & 0x01) << 1))

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  struct Panel_SharpLCD : public Panel_HasBuffer
  {
    color_depth_t setColorDepth(color_depth_t depth) override;

    bool init(bool use_reset) override;

    void waitDisplay(void) override {}
    bool displayBusy(void) override { return false; }

    void setRotation(uint_fast8_t) override { _rotation = r; }
    void setBrightness(uint8_t) override {}
    void setInvert(bool invert) override;
    void setSleep(bool) override {}
    void setPowerSave(bool) override {}

    void display(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h) override;

    void writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor) override;
    void writeImage(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param, bool use_dma) override;
    void writePixels(pixelcopy_t* param, uint32_t len, bool use_dma) override;

    uint32_t readCommand(uint_fast16_t, uint_fast8_t, uint_fast8_t) override { return 0; }
    uint32_t readData(uint_fast8_t, uint_fast8_t) override { return 0; }

    void readRect(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, void* dst, pixelcopy_t* param) override;

    void init_cs(void) override;
    void cs_control(bool level) override;

  protected:

    static constexpr uint8_t CMD_UPDATE             = 0x80;
    static constexpr uint8_t BIT_VCOM               = 0x40;
    static constexpr uint8_t CMD_CLEAR              = 0x20;
    static constexpr uint8_t VAL_TRAILER            = 0x00;
    static constexpr unsigned long _v_toggle_msec   = 990;

    range_rect_t _range_old;
    uint8_t _bayer_offset = 0;
    uint8_t _vcom = 0; 
    unsigned long _send_msec = 0;

    size_t _get_buffer_length(void) const override;
    bool _read_pixel(uint_fast16_t x, uint_fast16_t y);
    void _draw_pixel(uint_fast16_t x, uint_fast16_t y, uint32_t value);
    virtual void _update_transferred_rect(uint_fast16_t &xs, uint_fast16_t &ys, uint_fast16_t &xe, uint_fast16_t &ye);
    void _exec_transfer(const range_rect_t& range, bool invert = false);    
  };


  struct Panel_LS032B7DD02 : public Panel_SharpLCD
  {
    Panel_LS032B7DD02(void) : Panel_SharpLCD()
    {
      /// Actual width of the display in pixels.
      _cfg.panel_width = 336;
      /// Actual height of the display in rows/
      /// Note that this one has more than 255 rows
      _cfg.memory_height = _cfg.panel_height = 536;
      /// For this LCD each row in memory includes:
      /// 1) 2 bytes containing update command and the row number to update.
      /// Note that the row number spans across both bytes, sent in LSB order.
      _cfg.offset_x = 2;
      /// 2) 336/8=42 bytes of image data, one bit per pixel.
      /// Note that SPI bus sends MSB first so bits are stored in reverse order
      /// 3) Trailer
      _cfg.memory_width = _cfg.offset_x + _cfg.panel_width / 8 + 1;
      /// Actual number of LSBs moved to the command byte in reverse order
      _cfg.offset_y = 2;
      _auto_display = true;
    }
  };

  struct Panel_LS044Q7DH01 : public Panel_SharpLCD
  {
    Panel_LS044Q7DH01(void) : Panel_SharpLCD()
    {
      _cfg.panel_width = 320;
      _cfg.memory_height = _cfg.panel_height = 240;
      /// For LCDs having less than 255 rows each row in memory includes:
      /// 1) 1 byte containing the row number to update.
      _cfg.offset_x = 1;
      /// Actual number of bytes per row, including row number and trailer
      _cfg.memory_width = _cfg.offset_x + _cfg.panel_width / 8 + 1;
      _cfg.offset_y = 0;
      _auto_display = true;
    }
  };


  struct Panel_LS027B7DH01 : public Panel_SharpLCD
  {
    Panel_LS027B7DH01(void) : Panel_SharpLCD()
    {
      _cfg.panel_width = 400;
      _cfg.memory_height = _cfg.panel_height = 240;
      _cfg.offset_x = 1;
      _cfg.memory_width = _cfg.offset_x + _cfg.panel_width / 8 + 1;
      _cfg.offset_y = 0;
      _auto_display = true;
    }
  };

  // Special case to work out
  struct Panel_LS018B7DH02 : public Panel_SharpLCD
  {
    Panel_LS018B7DH02(void) : Panel_SharpLCD()
    {
      _cfg.panel_width = 230;
      _cfg.memory_height = _cfg.panel_height = 303;
      /// For this LCD each row in memory has:
      /// 1) 2 bytes containing command and the row number to update.
      _cfg.offset_x = 2;
      /// 2) 30 bytes of image data including 10 bits of padding
      /// 3) One Trailer byte
      _cfg.memory_width = _cfg.offset_x + 30 + 1;
      _cfg.offset_y = 1;
      _auto_display = true;
    }
  };

  struct Panel_LS013B7DH05 : public Panel_SharpLCD
  {
    Panel_LS013B7DH05(void) : Panel_SharpLCD()
    {
      _cfg.panel_width = 144;
      _cfg.memory_height = _cfg.panel_height = 168;
      _cfg.offset_x = 1;
      _cfg.memory_width = _cfg.offset_x + _cfg.panel_width / 8 + 1;
      _cfg.offset_y = 0;
      _auto_display = true;
    }
  };

  struct Panel_LS013B7DH03 : public Panel_SharpLCD
  {
    Panel_LS013B7DH03(void) : Panel_SharpLCD()
    {
      _cfg.panel_width = 128;
      _cfg.memory_height = _cfg.panel_height = 128;
      _cfg.offset_x = 1;
      _cfg.memory_width = _cfg.offset_x + _cfg.panel_width / 8 + 1;
      _cfg.offset_y = 0;
      _auto_display = true;
    }
  };
  
  struct Panel_LS012B7DD01 : public Panel_SharpLCD
  {
    Panel_LS012B7DD01(void) : Panel_SharpLCD()
    {
      _cfg.panel_width = 184;
      _cfg.memory_height = _cfg.panel_height = 38;
      _cfg.offset_x = 1;
      _cfg.memory_width = _cfg.offset_x + _cfg.panel_width / 8 + 1;
      _cfg.offset_y = 0;
      _auto_display = true;
    }
  };
  
  struct Panel_LS011B7DH03 : public Panel_SharpLCD
  {
    Panel_LS011B7DH03(void) : Panel_SharpLCD()
    {
      _cfg.panel_width = 160;
      _cfg.memory_height = _cfg.panel_height = 68;
      _cfg.offset_x = 1;
      _cfg.memory_width = _cfg.offset_x + _cfg.panel_width / 8 + 1;
      _cfg.offset_y = 0;
      _auto_display = true;
    }
  };
  
  struct Panel_LS010B7DH04 : public Panel_SharpLCD
  {
    Panel_LS010B7DH04(void) : Panel_SharpLCD()
    {
      _cfg.panel_width = 128;
      _cfg.memory_height = _cfg.panel_height = 128;
      _cfg.offset_x = 1;
      _cfg.memory_width = _cfg.offset_x + _cfg.panel_width / 8 + 1;
      _cfg.offset_y = 0;
      _auto_display = true;
    }
  };

//----------------------------------------------------------------------------
 }
}
