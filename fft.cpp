#include "fft.h"

#include <math.h>

RealFFT::RealFFT(int size)
  : size(size)
{}

void
RealFFT::execute(double* in)
{
    // Hamming window
    double bin_offset = 2.0 * M_PI / (size - 1);
    double angular_bin = 0.0;
    for (int i = 0; i < size; i++) {
        in[i] *= 0.53836 - (0.46164 * cos(angular_bin));
        angular_bin += bin_offset;
    }

    // Translation of algorithm described by Sorensen, et. al.
    // Digit reverse counter
    int j = 0;
    for (int i = 0; i < size - 1; i++) {
        if (i < j) {
            double temp = in[i];
            in[i] = in[j];
            in[j] = temp;
        }
        int k = size / 2;
        while (k <= j) {
            j -= k;
            k /= 2;
        }
        j += k;
    }

    // Length two butterflies
    for (int i = 0; i < size; i += 2) {
        double temp = in[i];
        in[i] = temp + in[i + 1];
        in[i + 1] = temp - in[i + 1];
    }

    // Compute log2(size)
    int m = size;
    for (int i = 0; i < 16; i++) {
        if (m == 1) {
            m = i;
            break;
        }

        m >>= 1;
    }

    // Other butterflies
    int n2 = 1;
    for (int k = 1; k < m; k++) {
        int n4 = n2;
        n2 *= 2;
        int n1 = n2 * 2;
        for (int i = 0; i < size; i += n1) {
            double temp = in[i];
            in[i] = temp + in[i + n2];
            in[i + n2] = temp - in[i + n2];
            in[i + n2 + n4] = -in[i + n2 + n4];
            double a = 2.0 * M_PI / n1;
            for (int j = 1; j < n4; j++) {
                int i1 = i + j;
                int i2 = i - j + n2;
                int i3 = i + j + n2;
                int i4 = i - j + n1;
                double cc = cos(a);
                double ss = sin(a);
                a += 2.0 * M_PI / n1;
                double t1 = (in[i3] * cc) + (in[i4] * ss);
                double t2 = (in[i3] * ss) - (in[i4] * cc);
                in[i4] = in[i2] - t2;
                in[i3] = -in[i2] - t2;
                in[i2] = in[i1] - t1;
                in[i1] = in[i1] + t1;
            }
        }
    }
}
