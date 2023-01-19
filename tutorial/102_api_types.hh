// Copyright 2022 Aurora Operations, Inc.

#include "au/au.hh"
#include "au/units/meters.hh"
#include "au/units/seconds.hh"

// For testing/tutorial purposes.
using namespace au;

// The distance (in meters) it would take to stop, starting from a given speed and acceleration.
//
// Parameters:
// - speed_mps:  The starting speed, in meters per second.
// - acceleration_mpss:  The braking acceleration, in meters per second squared.
//
// Preconditions:
// - speed_mps >= 0.0
// - acceleration_mpss < 0.0
double stopping_distance_m(double speed_mps, double acceleration_mpss);
