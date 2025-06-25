#pragma once

#include "pedal.h"

#include <stdbool.h>


bool reverb_create(void **instance_ptr, double sample_rate);
void reverb_destroy(void *instance);
double reverb_process(void *instance, double sample);
void reverb_set_params(void *instance, double params[PEDAL_MAX_PARAMS]);
