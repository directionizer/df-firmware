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
#ifndef FFT_H
#define FFT_H

class RealFFT
{
  private:
    int size;

  public:
    RealFFT(int);
    void execute(double*);
};

#endif /* FFT_H */
