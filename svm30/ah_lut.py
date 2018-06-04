#!/usr/bin/env python2.7
# -*- coding: utf-8 -*-
"""
(c) Copyright 2018 Sensirion AG, Switzerland
"""
import math


# Generate a look-up table for the interpolation of the relative humidity to
# absolute humidity conversion within a specifig temperature region (e.g. -20C
# to 70C in steps of 10C)
# As part of the output, the mean error over each discrete (rh, t) point within
# a region of interest is printed (`quantify_ah_lut_error`)

def calc_ah(t, rh):
    """ Mathematically correct AH computation """
    return 216.7 * (
            (rh / 100. * 6.112 * math.exp(17.62 * t / (243.12 + t))) /
            (273.15 + t))


def gen_ah_lut(t_range):
    """ Generate the AH Look Up Table at 100%RH (0..100 scales linearly) """
    return [calc_ah(t, 100) for t in t_range]


def ah_lookup(ah_lut, t_lo, t_hi, temp, rh):
    if rh == 0:
        return 0

    t_step = (t_hi - t_lo) / (len(ah_lut) - 1)
    t = temp - t_lo
    i = int(t / t_step)
    rem = t % t_step

    if i >= len(ah_lut) - 1:
        return ah_lut[-1] * (rh / 100.)

    if rem == 0:
        return ah_lut[i] * (rh / 100.)
    return (ah_lut[i] + (ah_lut[i + 1] - ah_lut[i]) * rem / t_step) * rh / 100.


def c_ah_lookup(ah_lut, t_lo, t_hi, temp, rh):
    """ Fixed point implementation (for C conversion)
    The only non-fixed point aspect is the final division by 1000. for
    comparison with the floating point version """
    if rh == 0:
        return 0

    rh = int(rh * 1000)
    norm_humi = (rh * 82) >> 13
    temp = int(temp * 1000)
    t_lo = int(t_lo * 1000)
    t_hi = int(t_hi * 1000)

    t_step = int((t_hi - t_lo) / (len(ah_lut) - 1))
    t = temp - t_lo
    i = t / t_step
    rem = t % t_step

    if i >= len(ah_lut) - 1:
        return (ah_lut[-1] * norm_humi) / 1000.
    if rem == 0:
        return (ah_lut[i] * norm_humi) / 1000.

    return ((ah_lut[i] + (ah_lut[i + 1] - ah_lut[i]) * rem / t_step) * norm_humi) / 1000.


def quantify_ah_lut_error(ah_lut, t_lo, t_hi, t_range, rh_range):
    s_float = 0
    s_int = 0
    for t in t_range:
        for rh in rh_range:
            s_float += abs(calc_ah(t, rh) - ah_lookup(ah_lut, t_lo, t_hi, t, rh))
            s_int += abs(calc_ah(t, rh) - c_ah_lookup(ah_lut, t_lo, t_hi, t, rh))
    div = (len(t_range) * len(rh_range))
    return (s_float / div, s_int / div)


if __name__ == '__main__':
    T_LO = -20
    T_HI = 70
    T_STEP = 10
    lut = gen_ah_lut(range(T_LO, T_HI+1, T_STEP))
    print("AH [g/m^m] (float)")
    print('{' +
          ', '.join(['{}'.format(ah) for ah in lut]) +
          '}')
    print("AH [mg/m^3] (int)")
    print('{' +
          ', '.join(['{}'.format(int(round(ah * 1000))) for ah in lut]) +
          '}')
    print("error avg(abs(ah(t,rh) - lookup(t,rh))) for T: -20..45, RH: 20..80 (float, int)")
    print(quantify_ah_lut_error(lut, T_LO, T_HI, range(T_LO, 45, 1), range(20, 80, 1)))

    # for t, rh in [(-20, 0), (-20, 100), (-19.9, 100), (-15.1, 90), (-15, 90),
    #         (-14.9, 90), (20, 50), (20, 100), (22.5, 50),  (55, 70),  (65, 70), (75, 70), (140, 50)]:
    #     print("AH({}, {}) = {} ~ {} ~ {}".format(t, rh, calc_ah(t, rh),
    #                                         ah_lookup(lut, T_LO, T_HI, t, rh),
    #                                         c_ah_lookup(lut, T_LO, T_HI, t, rh)))
    for i in range(0, 101, 10):
        print(int(ah_lookup(lut, T_LO, T_HI, i, i) * 1000))
