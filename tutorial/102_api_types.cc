// Copyright 2022 Aurora Operations, Inc.

#include "tutorial/102_api_types.hh"

double stopping_distance_m(double speed_mps, double acceleration_mpss) {
    // This first implementation uses only the most basic kinematic equations.
    // We could refactor it later to be more efficient.

    // t = (v - v0) / a
    double t_s = -speed_mps / acceleration_mpss;

    // (x - x0) = (v0 * t) + (1/2)(a * t^2)
    return speed_mps * t_s + 0.5 * acceleration_mpss * t_s * t_s;
}
