#include "sampler.h"

#include "Arduino.h"

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
    ADCSRA = bit(ADEN) | bit(ADPS1) | bit(ADPS2);
    ADCSRB = bit(ADTS1) | bit(ADTS2);
    ADMUX = 0;

    // Initialize ADC
    ADCSRA |= bit(ADSC);
    while (!(ADCSRA & bit(ADIF)))
        ;

    // Overflow Timer 1 every 100 us (10 kHz)
    TCCR1A = bit(WGM11);
    TCCR1B = bit(WGM12) | bit(WGM13) | bit(CS11);
    TIMSK1 = bit(TOIE1);
    ICR1 = 200 - 1;

    // Trigger ADC on Timer 1 overflow
    ADCSRA |= bit(ADATE) | bit(ADIE) | bit(ADIF);
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
    TCCR1B = bit(WGM12) | bit(WGM13);
    ADCSRA = 0;
}

bool
InterruptSampler::working()
{
    return !done;
}
