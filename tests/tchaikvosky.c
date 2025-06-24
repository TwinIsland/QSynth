#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "qsynth.h"
#include "instruments.h"

// Musical note definitions
#define C3 48
#define CS3 49
#define D3 50
#define DS3 51
#define E3 52
#define F3 53
#define FS3 54
#define G3 55
#define GS3 56
#define A3 57
#define AS3 58
#define B3 59
#define C4 60
#define CS4 61
#define D4 62
#define DS4 63
#define E4 64
#define F4 65
#define FS4 66
#define G4 67
#define GS4 68
#define A4 69
#define AS4 70
#define B4 71
#define C5 72
#define CS5 73
#define D5 74
#define DS5 75
#define E5 76
#define F5 77
#define FS5 78
#define G5 79
#define GS5 80
#define A5 81
#define AS5 82
#define B5 83
#define C6 84

// Play a chord (multiple notes simultaneously)
void play_chord(Synthesizer *synth, InstrumentType instrument, int *notes, int note_count,
                int duration_ms, float amplitude, float velocity, float pan)
{
    for (int i = 0; i < note_count; i++)
    {
        if (notes[i] > 0)
        {
            NoteCfg cfg = {
                .midi_note = notes[i],
                .duration_ms = duration_ms,
                .amplitude = amplitude,
                .velocity = velocity,
                .pan = pan};
            synth_play_note(synth, instrument, NOTE_CONTROL_DURATION, &cfg);
        }
    }
}

// Play single note
void play_note(Synthesizer *synth, InstrumentType instrument, int note,
               int duration_ms, float amplitude, float velocity, float pan)
{
    if (note > 0)
    {
        NoteCfg cfg = {
            .midi_note = note,
            .duration_ms = duration_ms,
            .amplitude = amplitude,
            .velocity = velocity,
            .pan = pan};
        synth_play_note(synth, instrument, NOTE_CONTROL_DURATION, &cfg);
    }
}

void play_tchaikovsky_concerto_opening(Synthesizer *synth)
{
    printf("🎼 Tchaikovsky Piano Concerto No. 1 in B♭ minor, Op. 23\n");
    printf("   Opening Theme - Atmospheric Arrangement\n");
    printf("   Performed by QSynth Digital Orchestra\n\n");

    // Set overall volume for classical feel
    synth_set_master_volume(synth, 0.7);

    // ===== MEASURE 1-2: The Famous Opening Chords =====
    printf("♪ The immortal opening chords...\n");

    // First massive chord - B♭ minor with added drama
    // (This is the chord that opens the concerto - one of the most famous in classical music)
    int opening_chord[] = {AS3 - 12, AS3, D4, F4, AS4, D5}; // B♭ minor chord, wide voicing

    // Deep bass foundation
    play_note(synth, INST_DEEP_DRONE, AS3 - 24, 3000, 0.8f, 0.9f, 0.3f);
    Sleep(100);

    // Powerful orchestral chord
    play_chord(synth, INST_WARM_BASS, opening_chord, 6, 2500, 0.7f, 0.8f, 0.5f);
    Sleep(200);

    // Bell-like piano entrance on top
    play_note(synth, INST_BELL_LEAD, AS5, 2000, 0.6f, 0.7f, 0.7f);
    Sleep(1800);

    // ===== MEASURE 3-4: Answering Phrase =====
    printf("♪ Answering phrase in F major...\n");

    // Second chord - F major (the answer to the opening statement)
    int second_chord[] = {F3, F3 + 12, A4, C5, F5}; // F major

    play_chord(synth, INST_WARM_BASS, second_chord, 5, 2000, 0.6f, 0.7f, 0.5f);
    Sleep(150);
    play_note(synth, INST_BELL_LEAD, F5, 1800, 0.5f, 0.6f, 0.6f);
    Sleep(1700);

    // ===== MEASURE 5-8: The Melodic Theme Begins =====
    printf("♪ The lyrical theme emerges...\n");

    // Bass line foundation (left hand of piano part)
    int bass_line[] = {AS3 - 12, AS3 - 12, F3, F3, D3, D3, AS3 - 12, AS3 - 12};
    int bass_durations[] = {600, 600, 600, 600, 600, 600, 1200, 600};

    // Start bass line
    for (int i = 0; i < 3; i++)
    {
        play_note(synth, INST_DEEP_DRONE, bass_line[i], bass_durations[i], 0.6f, 0.8f, 0.2f);
        Sleep(bass_durations[i] - 100);
    }

    // Melodic theme (the beautiful lyrical melody)
    // This is the main theme that the piano will later elaborate on
    int melody[] = {AS4, C5, D5, F5, AS5, AS5, A5, AS5, F5, D5};
    int melody_durations[] = {800, 400, 400, 600, 1200, 400, 400, 800, 600, 1000};
    float melody_dynamics[] = {0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 0.7f, 0.6f, 0.8f, 0.7f, 0.6f};

    Sleep(1000); // Start melody while bass is playing

    for (int i = 0; i < 10; i++)
    {
        play_note(synth, INST_BELL_LEAD, melody[i], melody_durations[i],
                  melody_dynamics[i], 0.8f, 0.7f);
        Sleep(melody_durations[i] - 50);

        // Continue bass accompaniment
        if (i < 8)
        {
            play_note(synth, INST_WARM_BASS, bass_line[i + 3], bass_durations[i + 3], 0.4f, 0.6f, 0.3f);
        }
    }

    // ===== MEASURE 9-12: Harmonic Development =====
    printf("♪ Harmonic development and orchestral colors...\n");

    // Add atmospheric pad for orchestral depth
    int pad_harmony[] = {D4, F4, AS4, D5}; // Sustained harmony
    play_chord(synth, INST_ETHEREAL_PAD, pad_harmony, 4, 4000, 0.3f, 0.5f, 0.5f);

    Sleep(500);

    // Decorative melodic figures (like woodwind solos in the orchestra)
    int decorative[] = {F5, G5, AS5, C6, AS5, G5, F5, D5, C5, AS4};

    for (int i = 0; i < 10; i++)
    {
        play_note(synth, INST_METALLIC_PLUCK, decorative[i], 300, 0.4f, 0.7f, 0.6f + (i % 3) * 0.1f);
        Sleep(250);
    }

    // ===== MEASURE 13-16: Building to Climax =====
    printf("♪ Building intensity toward the piano entrance...\n");

    // Ascending bass line (building tension)
    int ascending_bass[] = {AS3 - 12, C3, D3, F3, AS3, C4, D4, F4};

    for (int i = 0; i < 8; i++)
    {
        play_note(synth, INST_WARM_BASS, ascending_bass[i], 400, 0.6f + i * 0.05f, 0.8f, 0.3f);

        // Add rhythmic punctuation
        if (i % 2 == 0)
        {
            play_note(synth, INST_METALLIC_PLUCK, ascending_bass[i] + 12, 200, 0.3f, 0.9f, 0.7f);
        }

        Sleep(350);
    }

    // ===== FINAL CLIMACTIC CHORD =====
    printf("♪ Climactic resolution...\n");

    // Final massive orchestral chord - B♭ major (resolution of the minor)
    int final_chord[] = {AS3 - 24, AS3 - 12, AS3, D4, F4, AS4, D5, F5, AS5};

    // Layer the final chord with different instruments for orchestral effect
    play_chord(synth, INST_DEEP_DRONE, final_chord, 3, 4000, 0.8f, 0.9f, 0.4f);
    Sleep(100);
    play_chord(synth, INST_WARM_BASS, final_chord + 2, 4, 3800, 0.7f, 0.8f, 0.5f);
    Sleep(100);
    play_chord(synth, INST_BELL_LEAD, final_chord + 5, 4, 3500, 0.6f, 0.7f, 0.6f);
    Sleep(100);

    // Final ethereal shimmer
    play_note(synth, INST_ETHEREAL_PAD, AS5, 3000, 0.3f, 0.5f, 0.8f);

    Sleep(3500);

    printf("\n♪ \"...and thus begins one of the greatest piano concertos ever written.\"\n");
    printf("   - Tchaikovsky Piano Concerto No. 1, Opening Theme\n");
    printf("   - This leads into the famous piano entrance at measure 35\n\n");
}

int main()
{
    printf("🎹 QSynth Presents: Classical Masterworks Series\n");
    printf("================================================\n\n");

    // Initialize synthesizer
    Synthesizer *synth;
    if (!synth_init(&synth, 44100.0, 2))
    {
        printf("❌ ERROR: Failed to initialize synthesizer\n");
        return 1;
    }

    if (!synth_start(synth))
    {
        printf("❌ ERROR: %s\n", synth_get_error_string(synth_get_last_error()));
        synth_cleanup(synth);
        return 1;
    }

    printf("✅ QSynth Digital Orchestra initialized\n");
    printf("📊 Sample Rate: 44.1 kHz | Channels: Stereo | Voices: %d\n\n", MAX_VOICE_ACTIVE);

    // Brief pause for dramatic effect
    printf("🎭 Preparing for performance...\n");
    Sleep(2000);
    printf("🔇 Silence in the concert hall...\n\n");
    Sleep(1000);

    // Play the masterpiece
    play_tchaikovsky_concerto_opening(synth);

    // Performance statistics
    printf("📈 Performance Statistics:\n");
    synth_print_stat(synth);

    // Elegant shutdown
    printf("\n Thank you for experiencing this classical masterwork\n");
    printf("   rendered through QSynth's advanced synthesis engine.\n\n");

    synth_print_stat(synth);
    synth_stop(synth);
    synth_cleanup(synth);

    printf("🔚 Performance complete. Press Enter to exit...\n");
    getchar();

    return 0;
}