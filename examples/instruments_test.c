#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <time.h>

#include "qsynth.h"
#include "instruments.h"

// Musical note definitions (MIDI note numbers)
#define C4  60
#define CS4 61
#define D4  62
#define DS4 63
#define E4  64
#define F4  65
#define FS4 66
#define G4  67
#define GS4 68
#define A4  69
#define AS4 70
#define B4  71
#define C5  72

// Song structure
typedef struct {
    InstrumentType instrument;
    int notes[16];
    int note_count;
    int duration_ms;
    float velocity;
    float pan;
    int delay_ms;
} Track;

// Test individual instrument
void test_instrument(Synthesizer *synth, InstrumentType inst_type, const char* test_name)
{
    printf("Testing: %s\n", test_name);
    
    const InstrumentInfo inst = instrument_get_info(inst_type);
    printf("  Instrument: %s (%s)\n", inst.name, inst.category);
    printf("  Description: %s\n", inst.description);
    
    // Play a simple C major arpeggio
    int arpeggio[] = {C4, E4, G4, C5};
    
    for (int i = 0; i < 4; i++)
    {
        NoteCfg cfg = {
            .amplitude = 0.8f,
            .duration_ms = 800,
            .midi_note = arpeggio[i],
            .pan = 0.5f,
            .velocity = 0.8f,
        };
        
        int voice = synth_play_note(synth, inst_type, &cfg);
        if (voice == -1)
        {
            printf("  ERROR: Failed to play note %d\n", arpeggio[i]);
        }
        
        Sleep(300); // Overlap notes slightly
    }
    
    Sleep(1000); // Wait for notes to finish
    printf("  Test completed\n\n");
}

// Play a track
void play_track(Synthesizer *synth, const Track *track)
{
    const InstrumentInfo inst = instrument_get_info(track->instrument);
    printf("Playing track: %s\n", inst.name);
    
    for (int i = 0; i < track->note_count; i++)
    {
        if (track->notes[i] > 0) // 0 = rest
        {
            NoteCfg cfg = {
                .amplitude = 0.7f,
                .duration_ms = track->duration_ms,
                .midi_note = track->notes[i],
                .pan = track->pan,
                .velocity = track->velocity,
            };
            
            synth_play_note(synth, track->instrument, &cfg);
        }
        
        Sleep(track->delay_ms);
    }
}

// Main song arrangement
void play_full_song(Synthesizer *synth)
{
    printf("\n=== QSYNTH SHOWCASE SONG ===\n");
    printf("A musical journey through all instruments...\n\n");
    
    // Define tracks for different sections
    Track tracks[] = {
        // INTRO - Ethereal Pad
        {
            .instrument = INST_ETHEREAL_PAD,
            .notes = {C4, 0, E4, 0, G4, 0, C5, 0},
            .note_count = 8,
            .duration_ms = 2000,
            .velocity = 0.6f,
            .pan = 0.5f,
            .delay_ms = 500
        },
        
        // BASS LINE - Warm Bass
        {
            .instrument = INST_WARM_BASS,
            .notes = {C4-24, C4-24, G4-24, G4-24, A4-24, A4-24, F4-24, F4-24},
            .note_count = 8,
            .duration_ms = 800,
            .velocity = 0.9f,
            .pan = 0.3f,
            .delay_ms = 400
        },
        
        // LEAD MELODY - Lead Square
        {
            .instrument = INST_LEAD_SQUARE,
            .notes = {C5, D4, E4, F4, G4, A4, B4, C5},
            .note_count = 8,
            .duration_ms = 600,
            .velocity = 0.8f,
            .pan = 0.7f,
            .delay_ms = 300
        },
        
        // PLUCK ARPEGGIOS - Metallic Pluck
        {
            .instrument = INST_METALLIC_PLUCK,
            .notes = {C4, E4, G4, C5, G4, E4, C4, 0},
            .note_count = 8,
            .duration_ms = 400,
            .velocity = 0.7f,
            .pan = 0.6f,
            .delay_ms = 200
        },
        
        // WOBBLE SECTION - Wobble Bass
        {
            .instrument = INST_WOBBLE_BASS,
            .notes = {C4-12, 0, C4-12, 0, D4-12, 0, E4-12, 0},
            .note_count = 8,
            .duration_ms = 1200,
            .velocity = 0.9f,
            .pan = 0.4f,
            .delay_ms = 300
        },
        
        // BELL HARMONY - Bell Lead
        {
            .instrument = INST_BELL_LEAD,
            .notes = {E4, G4, B4, D4+12, B4, G4, E4, C4},
            .note_count = 8,
            .duration_ms = 800,
            .velocity = 0.6f,
            .pan = 0.8f,
            .delay_ms = 400
        },
        
        // DEEP DRONE FINALE
        {
            .instrument = INST_DEEP_DRONE,
            .notes = {C4-24, 0, 0, 0},
            .note_count = 4,
            .duration_ms = 3000,
            .velocity = 0.8f,
            .pan = 0.5f,
            .delay_ms = 1000
        }
    };
    
    int track_count = sizeof(tracks) / sizeof(Track);
    
    // Play each section
    for (int i = 0; i < track_count; i++)
    {
        printf("\n--- Section %d ---\n", i + 1);
        play_track(synth, &tracks[i]);
        Sleep(500); // Brief pause between sections
    }
    
    // FINALE - Play all instruments together (chord progression)
    printf("\n--- GRAND FINALE: ALL INSTRUMENTS ---\n");
    
    int chord_progression[] = {C4, F4, G4, C4};
    
    for (int chord = 0; chord < 4; chord++)
    {
        printf("Chord %d...\n", chord + 1);
        
        // Play the same chord on different instruments with different octaves
        NoteCfg bass_cfg = {
            .amplitude = 0.6f, .duration_ms = 2000, 
            .midi_note = chord_progression[chord] - 24, 
            .pan = 0.2f, .velocity = 0.8f
        };
        synth_play_note(synth, INST_WARM_BASS, &bass_cfg);
        
        NoteCfg pad_cfg = {
            .amplitude = 0.4f, .duration_ms = 2000, 
            .midi_note = chord_progression[chord], 
            .pan = 0.5f, .velocity = 0.6f
        };
        synth_play_note(synth, INST_ETHEREAL_PAD, &pad_cfg);
        
        NoteCfg lead_cfg = {
            .amplitude = 0.5f, .duration_ms = 1500, 
            .midi_note = chord_progression[chord] + 12, 
            .pan = 0.8f, .velocity = 0.7f
        };
        synth_play_note(synth, INST_LEAD_SQUARE, &lead_cfg);
        
        Sleep(1500);
    }
    
    Sleep(2000); // Let final chord ring out
    printf("\nSong complete!\n");
}

// Test all instruments systematically
void test_all_instruments(Synthesizer *synth)
{
    printf("=== TESTING ALL INSTRUMENTS ===\n\n");
    
    // First, print the instrument library
    instrument_print_all();
    
    printf("\n=== INDIVIDUAL INSTRUMENT TESTS ===\n");
    
    // Test each instrument type
    for (int i = 0; i < INST_COUNT; i++)
    {
        const InstrumentInfo inst = instrument_get_info((InstrumentType)i);

        test_instrument(synth, (InstrumentType)i, inst.name);
        Sleep(200); // Brief pause between tests
    }
}

// Main function
int main()
{
    printf("QSYNTH Comprehensive Instrument Test & Demo\n");
    printf("==========================================\n\n");
    
    // Initialize synthesizer
    Synthesizer *synth;
    if (!synth_init(&synth, 44100.0, 2))
    {
        printf("ERROR: Failed to initialize synthesizer\n");
        return 1;
    }
    
    if (!synth_start(synth))
    {
        printf("ERROR: %s\n", synth_get_error_string(synth_get_last_error()));
        synth_cleanup(synth);
        return 1;
    }
    
    printf("Synthesizer initialized successfully!\n");
    printf("Sample Rate: 44100 Hz, Channels: 2 (Stereo)\n\n");
    
    // Show instrument categories
    instrument_print_categories();
    
    // User menu
    printf("Choose test mode:\n");
    printf("1. Test all instruments individually\n");
    printf("2. Play full showcase song\n");
    printf("3. Both (recommended)\n");
    printf("4. Quick demo (just the song)\n");
    printf("Enter choice (1-4): ");
    
    int choice = scanf("%d", &choice); // Default choice for demo purposes
    // In real code: scanf("%d", &choice);
    
    printf("%d\n\n", choice);
    
    switch (choice)
    {
        case 1:
            test_all_instruments(synth);
            break;
            
        case 2:
            play_full_song(synth);
            break;
            
        case 3:
            test_all_instruments(synth);
            printf("\n");
            Sleep(2000);
            play_full_song(synth);
            break;
            
        case 4:
        default:
            printf("Playing quick showcase...\n");
            play_full_song(synth);
            break;
    }
    
    printf("\n=== DEMO COMPLETE ===\n");
    printf("Thank you for testing QSYNTH!\n");
    
    // Cleanup
    synth_stop(synth);
    synth_cleanup(synth);
    
    printf("Press any key to exit...\n");
    getchar();
    
    return 0;
}