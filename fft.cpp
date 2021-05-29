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

#include <math.h>

RealFFT::RealFFT(int size)
  : size(size)
{}

void
RealFFT::execute(double* in)
{
    // To reduce the effects of spectral leakage from sampling a finite portion
    // of time, a Hamming window is applied to the FFT input.
    double bin_offset = 2.0 * M_PI / (size - 1);
    double angular_bin = 0.0;
    for (int i = 0; i < size; i++) {
        in[i] *= 0.53836 - (0.46164 * cos(angular_bin));
        angular_bin += bin_offset;
    }

    // Compute log2(size). Since size is always a power of two, we can use the
    // trailing zeroes, which are computed with a compiler intrinsic.
    int bits = __builtin_ctz(size);

    // First, as we are operating in-place, the final output will be scrambled.
    // To correct for this, we will reorder the input based on the reverse
    // binary representation of each index before taking the FFT. That way, the
    // FFT process will un-scramble the order.
    int reversed = 0;
    for (int i = 0; i < size - 1; i++) {
        if (i < reversed) {
            // Swap
            double temp = in[i];
            in[i] = in[reversed];
            in[reversed] = temp;
        }

        // Calculate the next bit reversed number using an XOR recurrence from
        // Oliver Serang, described in arXiv:1708.01873 [cs.MS].
        unsigned int tail = i ^ (i + 1);
        tail <<= __builtin_clz(tail) - (sizeof(unsigned int) * 8) + bits;
        reversed ^= tail;
    }

    // As the first set of butterflies are completely real, we can use a much
    // simpler and faster loop for evaluating them.
    for (int i = 0; i < size; i += 2) {
        double temp = in[i];
        in[i] = temp + in[i + 1];
        in[i + 1] = temp - in[i + 1];
    }

    // Compute the other passes of the FFT using the real-optimized butterfly
    // described by Sorensen, et. al. in doi:10.1109/TASSP.1987.1165220.
    int n2 = 1;
    for (int k = 1; k < bits; k++) {
        int n4 = n2;
        n2 *= 2;
        int n1 = n2 * 2;
        for (int i = 0; i < size; i += n1) {
            double temp = in[i];
            in[i] = temp + in[i + n2];
            in[i + n2] = temp - in[i + n2];
            in[i + n2 + n4] = -in[i + n2 + n4];

            // Compute each real-optimized butterfly with the four indices.
            for (int j = 1; j < n4; j++) {
                int i1 = i + j;
                int i2 = i - j + n2;
                int i3 = i + j + n2;
                int i4 = i - j + n1;

                // Calculate sin and cos of the rotation angle.
                double beta = 2.0 * M_PI * j / n1;
                double cos_beta = cos(beta);
                double sin_beta = sin(beta);

                double t1 = (in[i3] * cos_beta) + (in[i4] * sin_beta);
                double t2 = (in[i3] * sin_beta) - (in[i4] * cos_beta);
                in[i4] = in[i2] - t2;
                in[i3] = -in[i2] - t2;
                in[i2] = in[i1] - t1;
                in[i1] = in[i1] + t1;
            }
        }
    }
}
