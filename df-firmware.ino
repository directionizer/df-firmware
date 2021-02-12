#include "sampler.h"

void
setup()
{
    Sampler.start();

    Serial.begin(9600);
}

void
loop()
{
    if (Sampler.ready()) {
        for (int i = 0; i < SAMPLER_BUFFER_SIZE; i++) {
            Serial.print(Sampler.samples[i].c1);
            Serial.print(" ");
            Serial.print(Sampler.samples[i].c2);
            Serial.print(" ");
            Serial.println(Sampler.samples[i].c3);
        }

        for (int i = 0; i < 32; i++) {
            Serial.println("500 500 0");
        }

        Serial.flush();
        Sampler.start();
    }
}
