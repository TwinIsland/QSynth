#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "pedal.h"
#include "pedal_core.h"
#include "../core/stream.h"

#include "../pedals/reverb.h"
#include "../pedals/distortion.h"
#include "../pedals/phaser.h"

static const PedalConfig pedal_info_db[PEDAL_COUNT] = {
    [PEDAL_REVERB] = {
        .name = "Reverb",
        .category = "Spatial",
        .description = "Digital reverb effect with room simulation",
        .param_n = 5,
        .param_description = {
            [0] = "Room Size (0.0-1.0)",
            [1] = "Decay Time (0.1-10.0 seconds)",
            [2] = "Damping (0.0-1.0)",
            [3] = "Wet/Dry Mix (0.0-1.0)",
            [4] = "Pre-delay (0-100ms)",
            [5] = "Output Level (0.0-2.0)",
        },
        .param_default = {[0] = 1.0, [1] = 0.8, [2] = 0.1, [3] = 0.8, [4] = 50.0, [5] = 0.5},
        .vtable = {
            .pedal_create = reverb_create,
            .pedal_destroy = reverb_destroy,
            .pedal_process = reverb_process,
            .pedal_set_params = reverb_set_params,
        },
    },

    [PEDAL_DISTORTION] = {
        .name = "Overdrive",
        .category = "Distortion",
        .description = "Warm overdrive with tone control and asymmetric clipping",
        .param_description = {
            [0] = "Gain (1.0-20.0)",
            [1] = "Drive (0.0-1.0)",
            [2] = "Tone (0.0-1.0)",
            [3] = "Output Level (0.0-2.0)",
            [4] = "Asymmetry (0.0-1.0)",
            [5] = NULL,
        },
        .param_default = {
            [0] = 3.0, // 3x gain
            [1] = 0.6, // 60% drive
            [2] = 0.7, // Bright tone
            [3] = 0.8, // 80% output
            [4] = 0.3, // 30% asymmetry
        },
        .vtable = {
            .pedal_create = distortion_create,
            .pedal_destroy = distortion_destroy,
            .pedal_process = distortion_process,
            .pedal_set_params = distortion_set_params,
        },
    },

    [PEDAL_PHASER] = {
        .name = "Phaser",
        .category = "Modulation",
        .description = "Classic phaser with sweeping allpass filters",
        .param_description = {
            [0] = "Rate (0.1-10 Hz)",
            [1] = "Depth (0.0-1.0)",
            [2] = "Feedback (0.0-0.9)",
            [3] = "Wet/Dry Mix (0.0-1.0)",
            [4] = "Center Freq (100-2000 Hz)",
            [5] = NULL,
        },
        .param_default = {
            [0] = 0.5,   // 0.5 Hz rate
            [1] = 0.8,   // 80% depth
            [2] = 0.6,   // 60% feedback
            [3] = 0.5,   // 50% wet
            [4] = 800.0, // 800 Hz center
        },
        .vtable = {
            .pedal_create = phaser_create,
            .pedal_destroy = phaser_destroy,
            .pedal_process = phaser_process,
            .pedal_set_params = phaser_set_params,
        },
    },
};

bool pedal_create(Pedal **pedal_ptr, PedalType type, double sample_rate)
{
    if (!pedal_ptr)
        return false;

    Pedal *pedal = calloc(1, sizeof(Pedal));
    if (!pedal)
        return false;

    pedal->vtable = pedal_info_db[type].vtable;
    pedal->type = type;
    pedal->cfg = &pedal_info_db[type];
    pedal->bypass = false;

    memcpy(pedal->params, pedal_info_db[type].param_default, sizeof(pedal_info_db[type].param_default));

    // Create instances for both channels
    if (!pedal->vtable.pedal_create(&pedal->pedal_instance_left, sample_rate) ||
        !pedal->vtable.pedal_create(&pedal->pedal_instance_right, sample_rate))
    {
        pedal_destroy(pedal);
        return false;
    }

    // Set params for both instances
    pedal->vtable.pedal_set_params(pedal->pedal_instance_left, pedal->params);
    pedal->vtable.pedal_set_params(pedal->pedal_instance_right, pedal->params);

    *pedal_ptr = pedal;
    return true;
}

void pedal_destroy(Pedal *pedal)
{
    if (!pedal)
        return;

    if (pedal->pedal_instance_left)
        pedal->vtable.pedal_destroy(pedal->pedal_instance_left);
    if (pedal->pedal_instance_right)
        pedal->vtable.pedal_destroy(pedal->pedal_instance_right);

    free(pedal);
}

void pedal_set_param(Pedal *pedal, size_t param_idx, float param_val)
{
    pedal->params[param_idx] = param_val;

    pedal->vtable.pedal_set_params(pedal->pedal_instance_left, pedal->params);
    pedal->vtable.pedal_set_params(pedal->pedal_instance_right, pedal->params);
}

const PedalConfig *pedal_get_cfg(PedalType pedal)
{
    if ((int)pedal >= (int)PEDAL_COUNT)
    {
        printf("none pedal with id (%d)", (int)pedal);
        return NULL;
    }
    return &pedal_info_db[(int)pedal];
}

// pedal chain implements
bool pedal_chain_create(PedalChain **pedal_chain_ptr)
{
    if (!pedal_chain_ptr)
        return false;

    PedalChain *chain = malloc(sizeof(PedalChain));
    if (!chain)
        return false;

    chain->head = NULL;
    chain->pedal_n = 0;

    stream_init(&chain->streamer, chain->stream_buf, PEDALCHAIN_BUFFER_SIZE);

    *pedal_chain_ptr = chain;
    return true;
}

bool pedal_chain_destroy(PedalChain *pedal_chain, bool is_destory_pedals)
{
    if (!pedal_chain)
        return false;

    PedalNode *current = pedal_chain->head;
    while (current)
    {
        PedalNode *next = current->next;

        if (is_destory_pedals)
            pedal_destroy(current->pedal);

        free(current);
        current = next;
    }

    free(pedal_chain);
    return true;
}

int pedal_chain_append(PedalChain *pedal_chain, Pedal *pedal)
{
    if (!pedal_chain || !pedal)
        return -1;

    PedalNode *new_node = malloc(sizeof(PedalNode));
    if (!new_node)
        return -1;

    new_node->pedal = pedal;
    new_node->next = NULL;

    // if chain is empty, make this the head
    if (!pedal_chain->head)
    {
        pedal_chain->head = new_node;
    }
    else
    {
        // find the last node and append
        PedalNode *current = pedal_chain->head;
        while (current->next)
        {
            current = current->next;
        }
        current->next = new_node;
    }

    pedal_chain->pedal_n++;
    return pedal_chain->pedal_n - 1;
}

bool pedal_chain_remove(PedalChain *pedal_chain, int idx)
{
    if (!pedal_chain || idx < 0 || idx >= (int)pedal_chain->pedal_n)
    {
        return false;
    }

    // removing head if idx=0
    if (idx == 0)
    {
        PedalNode *to_remove = pedal_chain->head;
        pedal_chain->head = pedal_chain->head->next;
        free(to_remove);
        pedal_chain->pedal_n--;
        return true;
    }

    PedalNode *current = pedal_chain->head;
    for (int i = 0; i < idx - 1; i++)
    {
        current = current->next;
    }

    PedalNode *to_remove = current->next;
    current->next = to_remove->next;
    free(to_remove);
    pedal_chain->pedal_n--;

    return true;
}

bool pedal_chain_swap(PedalChain *pedal_chain, int idx1, int idx2)
{
    if (!pedal_chain || idx1 == idx2)
        return false;
    if (idx1 < 0 || idx1 >= (int)pedal_chain->pedal_n)
        return false;
    if (idx2 < 0 || idx2 >= (int)pedal_chain->pedal_n)
        return false;

    PedalNode *node1 = pedal_chain_get(pedal_chain, idx1);
    PedalNode *node2 = pedal_chain_get(pedal_chain, idx2);

    if (!node1 || !node2)
        return false;

    // swap the pedal pointers
    Pedal *temp = node1->pedal;
    node1->pedal = node2->pedal;
    node2->pedal = temp;

    return true;
}

bool pedal_chain_insert(PedalChain *pedal_chain, int idx, Pedal *pedal)
{
    if (!pedal_chain || !pedal)
        return false;
    if (idx < 0 || idx > (int)pedal_chain->pedal_n)
        return false;

    PedalNode *new_node = malloc(sizeof(PedalNode));
    if (!new_node)
        return false;

    new_node->pedal = pedal;

    // insert at beginning
    if (idx == 0)
    {
        new_node->next = pedal_chain->head;
        pedal_chain->head = new_node;
        pedal_chain->pedal_n++;
        return true;
    }

    // find the node before insertion point
    PedalNode *current = pedal_chain->head;
    for (int i = 0; i < idx - 1; i++)
    {
        current = current->next;
    }

    // insert the new node
    new_node->next = current->next;
    current->next = new_node;
    pedal_chain->pedal_n++;

    return true;
}

PedalNode *pedal_chain_get(PedalChain *pedal_chain, int idx)
{
    if (!pedal_chain || idx < 0 || idx >= (int)pedal_chain->pedal_n)
    {
        return NULL;
    }

    PedalNode *current = pedal_chain->head;
    for (int i = 0; i < idx; i++)
    {
        current = current->next;
    }
    return current;
}

void pedal_chain_process_stereo(PedalChain *pedal_chain, double *left, double *right)
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

void pedal_chain_print(PedalChain *pedal_chain)
{
    if (!pedal_chain)
    {
        printf("Chain: NULL\n");
        return;
    }

    printf("Chain (%zu pedals): ", pedal_chain->pedal_n);
    PedalNode *current = pedal_chain->head;
    int i = 0;
    while (current)
    {
        printf("[%d]->", i++);
        current = current->next;
    }
    printf("NULL\n");
}
