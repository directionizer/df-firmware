#include "fft.h"
#include "sampler.h"

#include <math.h>

RealFFT fft = RealFFT(SAMPLER_BUFFER_SIZE);

void
setup()
{
    Sampler.start();

    Serial.begin(19200);
}

void
loop()
{
    if (Sampler.ready()) {
        fft.execute(Sampler.samples.c1);
        fft.execute(Sampler.samples.c2);
        fft.execute(Sampler.samples.c3);

        double c1 = atan2(Sampler.samples.c1[90], Sampler.samples.c1[39]);
        double c2 = atan2(Sampler.samples.c2[90], Sampler.samples.c2[39]);
        double c3 = atan2(Sampler.samples.c3[90], Sampler.samples.c3[39]);
        double phi = 360.0 + 36.0 + (180.0 * (c2 - c1) / M_PI);

        while (phi > 360.0) {
            phi -= 360.0;
        }
        Serial.println(phi);

        Serial.flush();
        Sampler.start();
    }
}
