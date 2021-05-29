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
#include "fft.h"
#include "sampler.h"

#include <math.h>

#define CACHE_SIZE 3
#define ENABLE_LED 6
#define OUTPUT_LEDS 4

static const byte leds[OUTPUT_LEDS] = { 2, 3, 4, 5 };
static RealFFT fft = RealFFT(SAMPLER_BUFFER_SIZE);

void
setup()
{
    // Set up output LEDs
    pinMode(ENABLE_LED, OUTPUT);
    for (int i = 0; i < OUTPUT_LEDS; i++) {
        pinMode(leds[i], OUTPUT);
    }

    // Turn off all lights on the display board
    digitalWrite(ENABLE_LED, 1);

    // Begin recording the three microphones
    Sampler.start();

    // Initialize serial port for USB communication
    Serial.begin(9600);
}

void
loop()
{
    static double results[CACHE_SIZE] = { 0.0, 0.0, 0.0 };

    // Keep polling Sampler until it is done recording
    if (!Sampler.working()) {
        // Map c1, c2, and c3 to frequency domain
        fft.execute((double*)Sampler.samples.c1);
        fft.execute((double*)Sampler.samples.c2);
        fft.execute((double*)Sampler.samples.c3);

        // From the FFT, find the reference phase delay, phi. 1000 Hz frequency
        // information is stored in bucket 39. For the FFT implementation, given
        // bucket n, the real component is stored at index n and the imaginary
        // component is stored at index 128 - n + 1.
        double p1 = atan2(Sampler.samples.c1[90], Sampler.samples.c1[39]);
        double p2 = atan2(Sampler.samples.c2[90], Sampler.samples.c2[39]);
        double p3 = atan2(Sampler.samples.c3[90], Sampler.samples.c3[39]);

        // Estimate an approach angle from the three phase values
        double angle = estimate_angle(p1, p2, p3);

        // Add the new result to the cache array
        if (!isnan(angle)) {
            for (int i = 0; i < CACHE_SIZE - 1; i++) {
                results[i] = results[i + 1];
            }
            results[CACHE_SIZE - 1] = angle;
        }

        // Average the recent results in rectangular coordinates and then find
        // the angle of that to get the average angle
        double ax = 0.0;
        double ay = 0.0;
        for (int i = 0; i < CACHE_SIZE; i++) {
            ax += cos(results[i]);
            ay += sin(results[i]);
        }
        double avg = atan2(ay / CACHE_SIZE, ax / CACHE_SIZE);

        // Print the found angle in degrees to the serial port
        Serial.println(rad_to_deg(avg));

        // Map the angle from radians to a number between 0 and 15 for the leds
        write_to_display((int)round(8.0 * avg / M_PI) % 16);

        // Force the Arduino to wait until everything has been printed, then
        // restart recording
        Serial.flush();
        Sampler.start();
    }
}

double
estimate_angle(double p1, double p2, double p3)
{
    double candidates[3][2];

    // Compute the wave delay, dt, from phase information. Relative phase
    // differences are shifted by pi/5 to correct for sampling delay.
    candidates[0][0] = wrap_angle((M_PI / 5.0) + (p2 - p1));
    candidates[1][0] = -1.0 * wrap_angle((2.0 * M_PI / 5.0) + (p3 - p1));
    candidates[2][0] = wrap_angle((M_PI / 5.0) + (p3 - p2));

    // Compute the candidate angles
    for (int i = 0; i < 3; i++) {
        double dt = candidates[i][0] / (M_PI * 2.0 * 1000.0);
        double rel_angle = acos((dt * 34027.0) / 5.2);
        double base_angle = 2.0 * M_PI * (i + 1) / 3.0;
        candidates[i][0] = wrap_angle(base_angle - rel_angle) + M_PI / 2.0;
        candidates[i][1] = wrap_angle(base_angle + rel_angle) + M_PI / 2.0;
    }

    // Find the two closest candidates and average them
    double angle = NAN;
    double distance = M_PI / 8; // Arbitrary minimum threshold
    for (int i = 0; i < 8; i++) {
        for (int j = i + 1; j < 3; j++) {
            for (int k = 0; k < 4; k++) {
                double a = candidates[i][k & 0x01];
                double b = candidates[j][(k >> 1) & 0x01];
                double distance_ab = fabs(a - b);
                if (distance_ab < distance) {
                    distance = distance_ab;

                    // Average the two to find the angle
                    double ax = cos(a) + cos(b);
                    double ay = sin(a) + sin(b);
                    angle = atan2(ay / 2, ax / 2);
                }
            }
        }
    }

    return angle;
}

void
write_to_display(int n)
{
    // Turn off the LED display
    digitalWrite(ENABLE_LED, 1);

    // Set the correct light
    for (int i = 0; i < OUTPUT_LEDS; i++) {
        digitalWrite(leds[i], n & 0x01);
        n >>= 1;
    }

    // Turn the display back on
    digitalWrite(ENABLE_LED, 0);
}

double
rad_to_deg(double n)
{
    n = 180.0 * n / M_PI;
    while (n >= 360.0) {
        n -= 360.0;
    }
    while (n < 0.0) {
        n += 360.0;
    }
    return n;
}

double
wrap_angle(double n)
{
    while (n >= M_PI) {
        n -= 2.0 * M_PI;
    }
    while (n < -M_PI) {
        n += 2.0 * M_PI;
    }
    return n;
}
