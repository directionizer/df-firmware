/*
 * Copyright (C) 2021  Mason Ahner, Jared Beller, and John Fiorini
 * This file is part of df-firmware.
 *
 * df-firmware is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * df-firmware is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef SAMPLER_H
#define SAMPLER_H

#include <avr/interrupt.h>
#include <stdint.h>

#define SAMPLER_BUFFER_SIZE 128

typedef struct
{
    double c1[SAMPLER_BUFFER_SIZE];
    double c2[SAMPLER_BUFFER_SIZE];
    double c3[SAMPLER_BUFFER_SIZE];
} channels;

extern "C" void
TIMER1_OVF_vect(void) __attribute__((signal));
extern "C" void
ADC_vect(void) __attribute__((signal));

class InterruptSampler
{
  private:
    static volatile uint8_t channel;
    static volatile bool done;
    static volatile int cursor;

    static void sample(int);
    static void initialize();

  public:
    static volatile channels samples;

    static void start();
    static void stop();
    static bool working();

    friend void ADC_vect(void);
};

extern InterruptSampler Sampler;

#endif /* SAMPLER_H */
