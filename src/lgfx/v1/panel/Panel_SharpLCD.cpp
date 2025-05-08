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
#include "Panel_SharpLCD.hpp"
#include "../Bus.hpp"
#include "../platforms/common.hpp"
#include "../misc/pixelcopy.hpp"
#include "pgmspace.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  static const char* TAG = "SharpLCD";

  static constexpr uint8_t Bayer[] = { 8, 136, 40, 168, 200, 72, 232, 104, 56, 184, 24, 152, 248, 120, 216, 88, 8, 136, 40, 168, 200, 72, 232, 104, 56, 184, 24, 152, 248, 120, 216, 88 };

  // 1<<n is a costly operation on AVR -- table usu. smaller & faster
  static const uint8_t PROGMEM set[] = {1, 2, 4, 8, 16, 32, 64, 128},
  clr[] = {(uint8_t)~1,  (uint8_t)~2,  (uint8_t)~4,
          (uint8_t)~8,  (uint8_t)~16, (uint8_t)~32,
          (uint8_t)~64, (uint8_t)~128};

  inline static uint32_t to_gray(uint8_t r, uint8_t g, uint8_t b)
  {
    return (uint32_t)          // gamma2.0 convert and ITU-R BT.601 RGB to Y convert
          ( (r * r * 19749)    // R 0.299
          + (g * g * 38771)    // G 0.587
          + (b * b *  7530)    // B 0.114
          ) >> 24;
  }

  Panel_SharpLCD::Panel_SharpLCD() {
    _epd_mode = epd_quality;
  }

  void Panel_SharpLCD::setInvert(bool invert)
  {
    ESP_LOGI(TAG, "Set Invert %d -> %d", _invert, invert);
    _invert = invert;
  }

  color_depth_t Panel_SharpLCD::setColorDepth(color_depth_t depth)
  {
    _write_depth = color_depth_t::grayscale_1bit;
    _read_depth = color_depth_t::grayscale_1bit;
    depth = color_depth_t::grayscale_1bit;
    return depth;
  }

  size_t Panel_SharpLCD::_get_buffer_length(void) const
  {
    return (_cfg.memory_width * _cfg.memory_height / 8);
  }

  bool Panel_SharpLCD::init(bool use_reset)
  {
    ESP_LOGD(TAG, "Panel_SharpLCD::init");
    if (!Panel_HasBuffer::init(false))
    {
      return false;
    }
    ESP_LOGD(TAG, "Panel_SharpLCD::init done");
    _vcom = BIT_VCOM;
    setColorDepth(color_depth_t::grayscale_1bit);
    // clearDisplay();
    ESP_LOGD(TAG, "Panel_SharpLCD::screen init");
    return true;
  }

  void Panel_SharpLCD::writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor)
  {
    uint_fast16_t xs = x, xe = x + w - 1;
    uint_fast16_t ys = y, ye = y + h - 1;
    _xs = xs;
    _ys = ys;
    _xe = xe;
    _ye = ye;
    //_update_transferred_rect(xs, ys, xe, ye); //TODO Do we need this with no flip?

    swap565_t color;
    color.raw = rawcolor;
    uint32_t value = to_gray(color.R8(), color.G8(), color.B8());

    y = ys;
    do
    {
      auto btbl = &Bayer[((y + (_bayer_offset >> 2)) & 3) << 2];  //TODO
      x = xs;
      uint32_t ms = y * _cfg.memory_width + _cfg.offset_x;
      do
      {
        uint32_t idx = ms + (x >> 3);
        uint32_t mask = 1 << (7- (x & 7)); //LSB go to MSB
        bool flg = 256 <= value + btbl[(x + _bayer_offset) & 3];
        if (flg) _buf[idx] |=   mask;
        else     _buf[idx] &= ~ mask;
      } while (++x <= xe);
    } while (++y <= ye);
  }

  void Panel_SharpLCD::writeImage(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param, bool use_dma)
  {
    uint_fast16_t xs = x, xe = x + w - 1;
    uint_fast16_t ys = y, ye = y + h - 1;
    //_update_transferred_rect(xs, ys, xe, ye); //TODO Do we need this with no flip?

    auto readbuf = (swap565_t*)alloca(w * sizeof(swap565_t));
    auto sx = param->src_x32;
    h += y;
    do
    {
      uint32_t prev_pos = 0, new_pos = 0;
      do
      {
        new_pos = param->fp_copy(readbuf, prev_pos, w, param);
        if (new_pos != prev_pos)
        {
          do
          {
            auto color = readbuf[prev_pos];
            _draw_pixel(x + prev_pos, y, to_gray(color.R8(), color.G8(), color.B8()));
          } while (new_pos != ++prev_pos);
        }
      } while (w != new_pos && w != (prev_pos = param->fp_skip(new_pos, w, param)));
      param->src_x32 = sx;
      param->src_y++;
    } while (++y < h);
  }

  void Panel_SharpLCD::writePixels(pixelcopy_t* param, uint32_t length, bool use_dma)
  {
    {
      uint_fast16_t xs = _xs;
      uint_fast16_t xe = _xe;
      uint_fast16_t ys = _ys;
      uint_fast16_t ye = _ye;
      //_update_transferred_rect(xs, ys, xe, ye); //TODO Do we need this with no flip?
    }
    uint_fast16_t xs   = _xs  ;
    uint_fast16_t ys   = _ys  ;
    uint_fast16_t xe   = _xe  ;
    uint_fast16_t ye   = _ye  ;
    uint_fast16_t xpos = _xpos;
    uint_fast16_t ypos = _ypos;

    static constexpr uint32_t buflen = 16;
    swap565_t colors[buflen];
    int bufpos = buflen;
    do
    {
      if (bufpos == buflen)
      {
        param->fp_copy(colors, 0, std::min(length, buflen), param);
        bufpos = 0;
      }
      auto color = colors[bufpos++];
      _draw_pixel(xpos, ypos, to_gray(color.R8(), color.G8(), color.B8()));
      if (++xpos > xe)
      {
        xpos = xs;
        if (++ypos > ye)
        {
          ypos = ys;
        }
      }
    } while (--length);
    _xpos = xpos;
    _ypos = ypos;
  }

  void Panel_SharpLCD::readRect(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, void* dst, pixelcopy_t* param)
  {
    auto readbuf = (swap565_t*)alloca(w * sizeof(swap565_t));
    param->src_data = readbuf;
    int32_t readpos = 0;
    h += y;
    do
    {
      uint32_t idx = 0;
      do
      {
        readbuf[idx] = _read_pixel(x + idx, y) ? -1 : 0;
      } while (++idx != w);
      param->src_x32 = 0;
      readpos = param->fp_copy(dst, readpos, readpos + w, param);
    } while (++y < h);
  }

  void Panel_SharpLCD::_draw_pixel(uint_fast16_t x, uint_fast16_t y, uint32_t value)
  {
    if (value) {
      _buf[(y * _cfg.memory_width + x) / 8] |= pgm_read_byte(&set[x & 7]);
    } else {
      _buf[(y * _cfg.memory_width + x) / 8] &= pgm_read_byte(&clr[x & 7]);
    }
  }

  bool Panel_SharpLCD::_read_pixel(uint_fast16_t x, uint_fast16_t y)
  {
    return _buf[(y * _cfg.memory_width + x) / 8] & pgm_read_byte(&set[x & 7]) ? 1 : 0;
  }

  void Panel_SharpLCD::_update_transferred_rect(uint_fast16_t &xs, uint_fast16_t &ys, uint_fast16_t &xe, uint_fast16_t &ye)
  {
    _range_mod.left   = std::min<int32_t>(xs, _range_mod.left);
    _range_mod.right  = std::max<int32_t>(xe, _range_mod.right);
    _range_mod.top    = std::min<int32_t>(ys, _range_mod.top);
    _range_mod.bottom = std::max<int32_t>(ye, _range_mod.bottom);
  }

  void Panel_SharpLCD::_exec_transfer(const range_rect_t& range, bool invert)
  {
    int32_t y = range.top;
    int32_t add = _cfg.memory_width;
    

    auto b = &_buf[y * _cfg.memory_width];
    if (invert)
    {
      //b += y * add;
      do
      {
        _bus->writeData(b[0], 8);
        if (_cfg.offset_x == 2) 
          _bus->writeData(b[1], 8);
        int32_t i = _cfg.offset_x; 
        do
        {
          _bus->writeData(~b[i], 8); //inverted pixels
        } while (++i != _cfg.memory_width - 1);
        _bus->writeData(b[_cfg.memory_width - 1], 8); //trailer
        b += add;
      } while (++y <= range.bottom);
    }
    else
    {
      int32_t w = _cfg.memory_width * (range.bottom - range.top);
      _bus->writeBytes(&b[y * _cfg.memory_width], w, true, true);
    }
  }

  void Panel_SharpLCD::display(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h)
  {
    // refresh();
    clearDisplay();
    return;
    if (0 < w && 0 < h)
    {
      _range_mod.left   = std::min<int16_t>(_range_mod.left  , x        );
      _range_mod.right  = std::max<int16_t>(_range_mod.right , x + w - 1);
      _range_mod.top    = std::min<int16_t>(_range_mod.top   , y        );
      _range_mod.bottom = std::max<int16_t>(_range_mod.bottom, y + h - 1);
    }
    if (_range_mod.empty())
    {
//      if (millis() - _send_msec >= _v_toggle_msec) 
        _vcom ^= BIT_VCOM;
      beginTransaction();
      _bus->writeData(_vcom, 8);
      _bus->writeData(VAL_TRAILER, 8);
      auto send_msec = millis();
      endTransaction();
      return;
    }

    _range_old = _range_mod;

    // if (millis() - _send_msec >= _v_toggle_msec)
    // {
      _vcom ^= BIT_VCOM; //toggle VCOM
      if (_cfg.offset_x == 1)
      {
        //LCD with less than 255 rows 
        beginTransaction();
        _bus->writeData(CMD_UPDATE | _vcom, 8);
      }
      else
      {
        // over 255 rows LCD
        // update VCOM bit in command byte of every row
        uint16_t idxL = 0;
        for (uint16_t y = 0; y < _cfg.panel_height; y++)
        {
          _buf[idxL] &= ~BIT_VCOM;
          _buf[idxL] |= _vcom;
          idxL += _cfg.memory_width;
        }
        beginTransaction();
      }
    //}  
    _exec_transfer(_range_mod, true);
    _bus->writeData(VAL_TRAILER, 8);
    auto send_msec = millis();
    endTransaction();

    _range_mod.top    = INT16_MAX;
    _range_mod.left   = INT16_MAX;
    _range_mod.right  = 0;
    _range_mod.bottom = 0;
  }

  void Panel_SharpLCD::init_cs(void)
  {
    auto pin = _cfg.pin_cs;
    if (pin < 0) return;
    ESP_LOGI(TAG, "Init CS %d", pin);
    lgfx::gpio_lo(pin);
    lgfx::pinMode(pin , pin_mode_t::output);
  }

  void Panel_SharpLCD::cs_control(bool level)
  {
    auto pin = _cfg.pin_cs;
    if (pin < 0) return;
    // ESP_LOGI(TAG, "CS %d level %d", pin, level);
    if (level)
    {
      lgfx::gpio_lo(pin);
    }
    else
    {
      lgfx::gpio_hi(pin);
    }
  }

  void Panel_SharpLCD::clearDisplay() {
    memset(_buf, 0xff, _get_buffer_length());

    ESP_LOGI(TAG, "clearDisplay");

    // beginTransaction();
    // log_d("beginTransaction");

    uint8_t clear_data[2] = {(uint8_t)(_vcom | CMD_CLEAR), 0x00};
    _bus->writeBytes(clear_data, 2, false, false);
    // _bus->writeData(_vcom | CMD_CLEAR);
    // _bus->transfer(0x00);

    // _bus->writeData(_vcom | CMD_CLEAR, 8);
    // _bus->writeData(VAL_TRAILER, 8);
    _vcom = _vcom == 0 ? BIT_VCOM : 0;

    // endTransaction();
    // log_d("endTransaction");
  }

  void Panel_SharpLCD::refresh(void) {
    uint16_t i, currentline;
  
    beginTransaction();

    _bus->writeData(_vcom | CMD_UPDATE, 8);
    _vcom = _vcom == 0 ? BIT_VCOM : 0;
  
    uint8_t bytes_per_line = _cfg.panel_width / 8;
    uint8_t line[bytes_per_line + 2];
  
    for (i = 0; i < _get_buffer_length(); i += bytes_per_line) {
      // Send address byte
      currentline = ((i + 1) / bytes_per_line) + 1;
      line[0] = currentline;
      // copy over this line
      memcpy(line + 1, _buf + i, bytes_per_line);
      // Send end of line
      line[bytes_per_line + 1] = 0x00;
      // send it!
      // _bus->writeBytes(line, bytes_per_line + 2, false, false);
      _bus->writeBytes(line, bytes_per_line + 2);
    }
    _bus->writeData(VAL_TRAILER, 8);

    endTransaction();
  }

//----------------------------------------------------------------------------
}
}
