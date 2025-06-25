#pragma once

#include <stdbool.h>

#include "pedal.h"

bool distortion_create(void **instance_ptr, double sample_rate);
void distortion_destroy(void *instance);
double distortion_process(void *instance, double sample);
void distortion_set_params(void *instance, double params[PEDAL_MAX_PARAMS]);