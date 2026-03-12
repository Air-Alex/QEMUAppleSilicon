/*
 * QEMU System Emulator
 *
 * Copyright (c) 2003-2008 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "qemu/osdep.h"
#include "qemu/timer.h"

/***********************************************************/
/* real time host monotonic timer */

int64_t clock_start;

#ifdef _WIN32

int64_t clock_freq;

static void __attribute__((constructor)) init_get_clock(void)
{
    LARGE_INTEGER freq;
    int ret;
    ret = QueryPerformanceFrequency(&freq);
    if (ret == 0) {
        fprintf(stderr, "Could not calibrate ticks\n");
        exit(1);
    }
    clock_freq = freq.QuadPart;
    clock_start = get_clock();
}

#elif defined(__APPLE__)

mach_timebase_info_data_t tb = { 0 };

static uint64_t gcd(uint64_t a, uint64_t b)
{
    if (a == 0)
        return b;
    if (b == 0)
        return a;
    if (a == b)
        return a;
    if (a > b)
        return gcd(a - b, b);
    return gcd(a, b - a);
}

static void __attribute__((constructor)) init_get_clock(void)
{
#ifdef __aarch64__
        uint64_t freq;
    __asm__ volatile("mrs %0, CNTFRQ_EL0" : "=r"(freq));
    uint64_t g = gcd(1000000000ULL, freq);
    tb.numer = 1000000000ULL / g;
    tb.denom = freq / g;
#else
    mach_timebase_info(&tb);
#endif
}

#else

int use_rt_clock;

static void __attribute__((constructor)) init_get_clock(void)
{
    struct timespec ts;

    use_rt_clock = 0;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        use_rt_clock = 1;
    }
    clock_start = get_clock();
}

#endif
