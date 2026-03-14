/*

MIT License

Copyright (c) 2026 Oliver Schmidt (https://a2retro.de/)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <pico/time.h>
#include <a2pico.h>

#include "board.h"

static const uint8_t __not_in_flash("mult_table") mult_table[] = {
    1,
    7,
    20,
    20};

static uint32_t        t1_val;
static bool            t1_run;
static uint32_t        t2_val;
static bool            t2_run;
static uint8_t         t2_freq;
static bool            t2_disp;

static uint32_t        t1_start;
static uint32_t        t2_start;
static absolute_time_t t1_begin;
static absolute_time_t t2_begin;
static uint8_t         t2_mult = mult_table[0];

static void __time_critical_func(handler)(bool asserted) {
    if (asserted) {
        t1_start = 0;
        t1_val   = 0;
        t1_run   = false;
        t2_start = 0;
        t2_val   = 0;
        t2_run   = false;
        t2_freq  = 0;
        t2_mult  = mult_table[0];
        t2_disp  = false;
        sio_hw->fifo_wr = 0xffffffff;
    }
}

static void __always_inline t2_display(void) {
    if (!t2_disp) {
        return;
    }
    sio_hw->fifo_wr = t2_val;
}

static void __always_inline t1_update(absolute_time_t now) {
    if (!t1_run) {
        return;
    }
    int64_t diff = absolute_time_diff_us(t1_begin, now);
    t1_val = t1_start + (uint32_t)(diff * 20);
}

static void __always_inline t2_update(absolute_time_t now) {
    if (!t2_run) {
        return;
    }
    int64_t diff = absolute_time_diff_us(t2_begin, now);
    t2_val = t2_start + (uint32_t)(diff * t2_mult);
    t2_display();
}

static void __time_critical_func(nop_get)(void) {
}

static void __time_critical_func(status_get)(void) {
    a2pico_putdata((t1_run ? 0x80 : 0x00) |
                   (t2_run ? 0x40 : 0x00));
}

static void __time_critical_func(freq_get)(void) {
    a2pico_putdata(t2_freq << 6);
}

static void __time_critical_func(t1_0_get)(void) {
    t1_update(get_absolute_time());
    a2pico_putdata((t1_val >> 0x00) & 0xFF);
}

static void __time_critical_func(t1_1_get)(void) {
    a2pico_putdata((t1_val >> 0x08) & 0xFF);
}

static void __time_critical_func(t1_2_get)(void) {
    a2pico_putdata((t1_val >> 0x10) & 0xFF);
}

static void __time_critical_func(t1_3_get)(void) {
    a2pico_putdata((t1_val >> 0x18) & 0xFF);
}

static void __time_critical_func(t2_0_get)(void) {
    t2_update(get_absolute_time());
    a2pico_putdata((t2_val >> 0x00) & 0xFF);
}

static void __time_critical_func(t2_1_get)(void) {
    a2pico_putdata((t2_val >> 0x08) & 0xFF);
}

static void __time_critical_func(t2_2_get)(void) {
    a2pico_putdata((t2_val >> 0x10) & 0xFF);
}

static void __time_critical_func(t2_3_get)(void) {
    a2pico_putdata((t2_val >> 0x18) & 0xFF);
}

static const void __not_in_flash("devsel_get") (*devsel_get[])(void) = {
    nop_get,  status_get, nop_get,  nop_get,
    freq_get, nop_get,    nop_get,  nop_get,
    t1_0_get, t1_1_get,   t1_2_get, t1_3_get,
    t2_0_get, t2_1_get,   t2_2_get, t2_3_get,
};

static void __time_critical_func(nop_put)(uint32_t data) {
}

static void __time_critical_func(reset_put)(uint32_t data) {
    if (!data) {
        data = 3;
    }
    absolute_time_t now = get_absolute_time();
    if (data & 1) {
        t1_start = 0;
        t1_begin = now;  // if running
        t1_val   = 0;
    }
    if (data & 2) {
        t2_start = 0;
        t2_begin = now;  // if running
        t2_val   = 0;
        t2_display();
    }
}

static void __time_critical_func(start_put)(uint32_t data) {
    if (!data) {
        data = 3;
    }
    absolute_time_t now = get_absolute_time();
    if (data & 1) {
        t1_start = t1_val;
        t1_begin = now;
        t1_run   = true;
    }
    if (data & 2) {
        t2_start = t2_val;
        t2_begin = now;
        t2_run   = true;
    }
}

static void __time_critical_func(stop_put)(uint32_t data) {
    if (!data) {
        data = 3;
    }
    absolute_time_t now = get_absolute_time();
    if (data & 1) {
        t1_update(now);
        t1_run = false;
    }
    if (data & 2) {
        t2_update(now);
        t2_display();
        t2_run = false;
    }
}

static void __time_critical_func(freq_put)(uint32_t data) {
    t2_freq = data;
    t2_mult = mult_table[t2_freq];
}

static void __time_critical_func(disp_put)(uint32_t data) {
    t2_disp = !!data;
    if (t2_disp) {
        t2_update(get_absolute_time());
        t2_display();
    } else {
        sio_hw->fifo_wr = 0xffffffff;
    }
}

static const void __not_in_flash("devsel_put") (*devsel_put[])(uint32_t) = {
    reset_put, start_put, stop_put, nop_put,
    freq_put,  disp_put,  nop_put,  nop_put,
    nop_put,   nop_put,   nop_put,  nop_put,
    nop_put,   nop_put,   nop_put,  nop_put
};

void __time_critical_func(board)(void) {

    a2pico_init();

    a2pico_resethandler(&handler);

    while (true) {
        uint32_t pico = a2pico_getaddr();
        uint32_t addr = pico & 0x0FFF;
        uint32_t io   = pico & 0x0F00;  // IOSTRB or IOSEL
        uint32_t strb = pico & 0x0800;  // IOSTRB
        uint32_t read = pico & RW_BIT;  // R/W

        if (read) {
            if (!io) {  // DEVSEL
                devsel_get[addr & 0xF]();
            }
        } else {
            uint32_t data = a2pico_getdata();
            if (!io) {  // DEVSEL
                devsel_put[addr & 0xF](data);
            }
        }
    }
}
