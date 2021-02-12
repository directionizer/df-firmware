#ifndef SAMPLER_H
#define SAMPLER_H

#include <avr/interrupt.h>
#include <stdint.h>

#define SAMPLER_BUFFER_SIZE 128

typedef struct
{
    int c1;
    int c2;
    int c3;
} frame;

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
    static volatile frame samples[SAMPLER_BUFFER_SIZE];

    static void start();
    static void stop();
    static bool ready();

    friend void ADC_vect(void);
};

extern InterruptSampler Sampler;

#endif /* SAMPLER_H */
