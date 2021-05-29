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
#include "sampler.h"

#include <avr/interrupt.h>

InterruptSampler Sampler;

// Ignore actual timer overflow (only used for ADC trigger)
EMPTY_INTERRUPT(TIMER1_OVF_vect);

ISR(ADC_vect)
{
    Sampler.sample(ADC);
}

volatile channels InterruptSampler::samples;
volatile int InterruptSampler::cursor = 0;
volatile uint8_t InterruptSampler::channel = 0;
volatile bool InterruptSampler::done = false;

inline void
InterruptSampler::sample(int adc)
{
    // Collect ADC into samples, round-robin
    if (cursor != SAMPLER_BUFFER_SIZE) {
        switch (channel) {
            case 0:
                samples.c1[cursor] = (double)adc;
                channel = 1;
                break;
            case 1:
                samples.c2[cursor] = (double)adc;
                channel = 2;
                break;
            case 2:
                samples.c3[cursor] = (double)adc;
                channel = 0;
                cursor += 1;
                break;
        }

        // Set ADC to next channel
        ADMUX = channel;
    } else {
        stop();
        done = true;
    }
}

void
InterruptSampler::initialize()
{
    // Configure the ADC (prescaler = 64)
    ADCSRA = _BV(ADEN) | _BV(ADPS1) | _BV(ADPS2);
    ADCSRB = _BV(ADTS1) | _BV(ADTS2);
    ADMUX = 0;

    // Initialize ADC
    ADCSRA |= _BV(ADSC);
    while (!(ADCSRA & _BV(ADIF)))
        ;

    // Overflow Timer 1 every 100 us (10 kHz)
    TCCR1A = _BV(WGM11);
    TCCR1B = _BV(WGM12) | _BV(WGM13) | _BV(CS11);
    TIMSK1 = _BV(TOIE1);
    ICR1 = 200 - 1;

    // Trigger ADC on Timer 1 overflow
    ADCSRA |= _BV(ADATE) | _BV(ADIE) | _BV(ADIF);
}

void
InterruptSampler::start()
{
    channel = 0;
    done = false;
    cursor = 0;
    initialize();
}

inline void
InterruptSampler::stop()
{
    // Disable Timer 1 and ADC
    TCCR1B = _BV(WGM12) | _BV(WGM13);
    ADCSRA = 0;
}

bool
InterruptSampler::working()
{
    return !done;
}
