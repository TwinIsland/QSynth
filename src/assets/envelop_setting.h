#pragma once

// PLUCK INSTRUMENTS (Guitar, Piano, Harp)
// Fast attack, quick decay, little sustain, medium release
#define ENVELOPE_OPT_PLUCK {.attack_time = 0.005,  \
                            .decay_time = 0.2,     \
                            .sustain_level = 0.1,  \
                            .release_time = 0.3}

// PAD INSTRUMENTS (Strings, Choir, Atmosphere)
// Slow attack, gentle decay, long sustain, slow release
#define ENVELOPE_OPT_PAD {.attack_time = 0.8,     \
                          .decay_time = 0.3,      \
                          .sustain_level = 0.8,   \
                          .release_time = 1.2}

// BASS INSTRUMENTS (Sub bass, Electric bass)
// Medium attack, quick decay, good sustain, long release
#define ENVELOPE_OPT_BASS {.attack_time = 0.02,   \
                           .decay_time = 0.15,    \
                           .sustain_level = 0.7,  \
                           .release_time = 0.8}

// LEAD INSTRUMENTS (Lead synth, Solo instruments)
// Fast attack, medium decay, medium sustain, medium release
#define ENVELOPE_OPT_LEAD {.attack_time = 0.01,   \
                           .decay_time = 0.2,     \
                           .sustain_level = 0.6,  \
                           .release_time = 0.4}

// PERCUSSION (Drums, Hits, Impacts)
// Instant attack, long decay, no sustain, short release
#define ENVELOPE_OPT_PERCUSSION {.attack_time = 0.001, \
                                 .decay_time = 0.5,     \
                                 .sustain_level = 0.0,  \
                                 .release_time = 0.1}

// ORGAN/SUSTAINED (Hammond organ, Sustained brass)
// Medium attack, minimal decay, full sustain, quick release
#define ENVELOPE_OPT_ORGAN {.attack_time = 0.05,  \
                            .decay_time = 0.01,   \
                            .sustain_level = 1.0, \
                            .release_time = 0.1}

// STAB INSTRUMENTS (Staccato strings, Short brass hits)
// Fast attack, medium decay, low sustain, fast release
#define ENVELOPE_OPT_STAB {.attack_time = 0.003,  \
                           .decay_time = 0.1,     \
                           .sustain_level = 0.2,  \
                           .release_time = 0.15}

// BELL INSTRUMENTS (Bells, Chimes, Metallic sounds)
// Fast attack, very long decay, no sustain, long release
#define ENVELOPE_OPT_BELL {.attack_time = 0.002,  \
                           .decay_time = 2.0,     \
                           .sustain_level = 0.0,  \
                           .release_time = 1.5}

// WOODWIND INSTRUMENTS (Flute, Clarinet, Saxophone)
// Medium attack, short decay, high sustain, medium release
#define ENVELOPE_OPT_WOODWIND {.attack_time = 0.1,   \
                               .decay_time = 0.05,   \
                               .sustain_level = 0.9, \
                               .release_time = 0.2}

// BRASS INSTRUMENTS (Trumpet, Trombone, French Horn)
// Medium attack, short decay, high sustain, quick release
#define ENVELOPE_OPT_BRASS {.attack_time = 0.08,  \
                            .decay_time = 0.03,   \
                            .sustain_level = 0.85,\
                            .release_time = 0.15}
