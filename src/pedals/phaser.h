#pragma once

#include "pedal.h"

#include <stdbool.h>


bool phaser_create(void **instance_ptr, double sample_rate);
void phaser_destroy(void *instance);
double phaser_process(void *instance, double sample);
void phaser_set_params(void *instance, double params[PEDAL_MAX_PARAMS]);
