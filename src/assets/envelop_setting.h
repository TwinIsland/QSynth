#pragma once

// PLUCK INSTRUMENTS (Guitar, Piano, Harp)
// Fast attack, quick decay, little sustain, medium release
#define ENVELOPE_OPT_PLUCK {.attack_ratio = 0.005, \
                            .decay_ratio = 0.3,    \
                            .release_ratio = 0.2,  \
                            .sustain_level = 0.1}

// PAD INSTRUMENTS (Strings, Choir, Atmosphere)
// Slow attack, gentle decay, long sustain, slow release
#define ENVELOPE_OPT_PAD {.attack_ratio = 0.2,  \
                          .decay_ratio = 0.1,   \
                          .release_ratio = 0.3, \
                          .sustain_level = 0.8}

// BASS INSTRUMENTS (Sub bass, Electric bass)
// Medium attack, quick decay, good sustain, long release
#define ENVELOPE_OPT_BASS {.attack_ratio = 0.02, \
                           .decay_ratio = 0.15,  \
                           .release_ratio = 0.4, \
                           .sustain_level = 0.7}

// LEAD INSTRUMENTS (Lead synth, Solo instruments)
// Fast attack, medium decay, medium sustain, medium release
#define ENVELOPE_OPT_LEAD {.attack_ratio = 0.01,  \
                           .decay_ratio = 0.25,   \
                           .release_ratio = 0.25, \
                           .sustain_level = 0.6}

// PERCUSSION (Drums, Hits, Impacts)
// Instant attack, long decay, no sustain, short release
#define ENVELOPE_OPT_PERCUSSION {.attack_ratio = 0.001, \
                                 .decay_ratio = 0.7,    \
                                 .release_ratio = 0.1,  \
                                 .sustain_level = 0}

// ORGAN/SUSTAINED (Hammond organ, Sustained brass)
// Medium attack, no decay, full sustain, quick release
#define ENVELOPE_OPT_ORGAN {.attack_ratio = 0.05, \
                                 .decay_ratio = 0,    \
                                 .release_ratio = 0.1,  \
                                 .sustain_level = 0}

