#include <stdio.h>
#include <windows.h>

#include "qsynth.h"

int main()
{
    Synthesizer *synth;
    if (!synth_init(&synth, 44100.0, 2))
    {
        printf("Failed to initialize synthesizer\n");
        return 1;
    }

    if (!synth_start(synth))
    {
        printf("Failed to start synthesizer: %s\n",
               synth_get_error_string(synth_get_last_error()));
        synth_cleanup(synth);
        return 1;
    }

    synth_pedalchain_append(synth, PEDAL_PHASER);

    int melody[] = {60, 64, 67, 72, 67, 64, 60};

    for (int i = 0; i < 7; i++)
    {
        NoteCfg cfg = (NoteCfg){
            .amplitude = 0.5f,
            .midi_note = melody[i],
            .pan = 0.5,
        };

        int voice = synth_play_note(synth, INST_WARM_BASS, NOTE_CONTROL_MANUAL, &cfg);

        if (voice == -1)
        {
            printf(synth_get_error_string(synth_get_last_error()));
        }

        Sleep(400);

        synth_end_note(synth, voice);
    }

    // synth_pedalchain_print(synth);

    printf("\nduration control mode test\n");
    for (int i = 0; i < 7; i++)
    {
        NoteCfg cfg = (NoteCfg){
            .amplitude = 1,
            .duration_ms = 400,
            .midi_note = melody[i],
            .pan = 0.5,
        };

        int voice = synth_play_note(synth, INST_WARM_BASS, NOTE_CONTROL_DURATION, &cfg);

        if (voice == -1)
        {
            printf(synth_get_error_string(synth_get_last_error()));
        }
        Sleep(400);
    }

    synth_print_stat(synth);

    synth_stop(synth);
    synth_cleanup(synth);

    return 0;
}