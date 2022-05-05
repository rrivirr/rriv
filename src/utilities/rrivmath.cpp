#include "rrivmath.h"

#define LN10 2.3025850929940456840179914546844

namespace rrivmath
{

  float power(float x, int y)
  {
    float temp;
    if (y == 0)
      return 1;
    temp = power(x, y / 2);
    if (y % 2 == 0)
      return temp * temp;
    else
    {
      if (y > 0)
        return x * temp * temp;
      else
        return (temp * temp) / x;
    }
  }

  double ln(double x)
  {
    double old_sum = 0.0;
    double xmlxpl = (x - 1) / (x + 1);
    double xmlxpl_2 = xmlxpl * xmlxpl;
    double denom = 1.0;
    double frac = xmlxpl;
    double term = frac; // denom start from 1.0
    double sum = term;

    while (sum != old_sum)
    {
      old_sum = sum;
      denom += 2.0;
      frac *= xmlxpl_2;
      sum += frac / denom;
    }
    return 2.0 * sum;
  }

  double log10(double x)
  {
    return ln(x) / LN10;
  }

  float floor(float x)
  {
    int i = (int)x;
    return (float)((x < 0.0f) ? i - 1 : i);
  }

}