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
        printf(synth_get_error_string(synth_get_last_error()));
        return 1;
    }

    int melody[] = {60, 64, 67, 72, 67, 64, 60};

    for (int i = 0; i < 7; i++)
    {
        NoteCfg cfg = (NoteCfg){
            .amplitude = 1,
            .midi_note = melody[i],
            .pan = 0.5,
        };

        int voice = synth_play_note(synth, INST_WARM_BASS, NOTE_CONTROL_MANUAL, &cfg);

        if (voice == -1)
        {
            printf(synth_get_error_string(synth_get_last_error()));
        }

        Sleep(1000);

        synth_end_note(synth, voice);
    }

    printf("\nduration control mode test\n");
    for (int i = 0; i < 7; i++)
    {
        NoteCfg cfg = (NoteCfg){
            .amplitude = 1,
            .duration_ms = 1000,
            .midi_note = 60,
            .pan = 0.5,
        };

        int voice = synth_play_note(synth, INST_WARM_BASS, NOTE_CONTROL_DURATION, &cfg);

        if (voice == -1)
        {
            printf(synth_get_error_string(synth_get_last_error()));
        }
        Sleep(1000);
    }

    synth_print_stat(synth);

    synth_stop(synth);
    synth_cleanup(synth);

    return 0;
}