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
