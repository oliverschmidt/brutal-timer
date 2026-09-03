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

#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <hardware/clocks.h>
#include <hardware/structs/busctrl.h>

#include <a2pico.h>

#include "board.h"

void main(void) {
    busctrl_hw->priority = BUSCTRL_BUS_PRIORITY_PROC1_BITS;
    multicore_launch_core1(board);

    set_sys_clock_khz(200000, false);

    if (a2pico_led() >= 0) {
        gpio_init(a2pico_led());
        gpio_set_dir(a2pico_led(), GPIO_OUT);
    }

    stdio_init_all();

    while (true) {
        while (multicore_fifo_rvalid()) {
            uint32_t value = multicore_fifo_pop_blocking();
            if (value == 0xffffffff) {
                printf("%8c\r", ' ');
            } else {
                printf("%08X\r", value);
            }
        }

        if (a2pico_led() >= 0) {
            gpio_put(a2pico_led(), stdio_usb_connected());
        }
    }
}
