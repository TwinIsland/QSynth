#pragma once

#include <stdbool.h>

#include "pedal.h"
#include "qsynth.h"
#include "../core/stream.h"

typedef struct
{
    bool (*pedal_create)(void **instance_ptr, double sample_rate);
    void (*pedal_destroy)(void *instance);
    double (*pedal_process)(void *instance, double sample);
    void (*pedal_set_params)(void *instance, double params[PEDAL_MAX_PARAMS]);
} PedalVTable;

typedef struct
{
    const char *name;
    const char *category;
    const char *description;

    const double param_default[PEDAL_MAX_PARAMS];
    const char *param_description[PEDAL_MAX_PARAMS];
    int param_n;
    PedalVTable vtable;
} PedalConfig;

typedef struct
{
    PedalType type;
    const PedalConfig *cfg;
    bool bypass;
    double params[PEDAL_MAX_PARAMS];

    void *pedal_instance_left;  // pedal instance for left channel
    void *pedal_instance_right; // pedal instance for right channel
    PedalVTable vtable;
} Pedal;

static inline void pedal_process(Pedal *pedal, double *left, double *right)
{
    if (!pedal || !pedal->pedal_instance_left || !pedal->pedal_instance_right || !left || !right)
        return;
    
    *left = pedal->vtable.pedal_process(pedal->pedal_instance_left, *left);
    *right = pedal->vtable.pedal_process(pedal->pedal_instance_right, *right);
}

bool pedal_create(Pedal **pedal_ptr, PedalType type, double sample_rate);
void pedal_destroy(Pedal *pedal);
void pedal_set_param(Pedal *pedal, size_t param_idx, float param_val);
const PedalConfig *pedal_get_cfg(PedalType pedal);

// pedal chain utils
typedef struct PedalNode
{
    Pedal *pedal;
    struct PedalNode *next;
} PedalNode;

typedef struct
{
    PedalNode *head;
    size_t pedal_n;

    // streaming state
    double stream_buf[PEDALCHAIN_BUFFER_SIZE];
    AudioStreamBuffer streamer;
} PedalChain; // basically a single direction linklist

bool pedal_chain_create(PedalChain **pedal_chain_ptr);
bool pedal_chain_destroy(PedalChain *pedal_chain, bool is_destory_pedals);
int pedal_chain_append(PedalChain *pedal_chain, Pedal *pedal);
bool pedal_chain_remove(PedalChain *pedal_chain, int idx);
bool pedal_chain_swap(PedalChain *pedal_chain, int idx1, int idx2);
bool pedal_chain_insert(PedalChain *pedal_chain, int idx, Pedal *pedal);
PedalNode *pedal_chain_get(PedalChain *pedal_chain, int idx);

void pedal_chain_print(PedalChain *pedal_chain);

static inline void pedal_chain_process(PedalChain *pedal_chain, double *left, double *right)
{
    if (!pedal_chain || !left || !right)
        return;

    PedalNode *current = pedal_chain->head;

    while (current)
    {
        if (current->pedal)
        {
            pedal_process(current->pedal, left, right);
        }
        current = current->next;
    }
}

static inline size_t pedal_chain_size(PedalChain *pedal_chain)
{
    return pedal_chain ? pedal_chain->pedal_n : 0;
}
