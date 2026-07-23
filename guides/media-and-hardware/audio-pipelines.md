# Audio Pipelines & Playback

Modern applications often require audio feedback, media playback, or VoIP capabilities. The framework's audio subsystem (`ui_audio_sink.h`, `ui_media.h`) abstracts platform-specific audio APIs (like ALSA, CoreAudio, WASAPI) into a unified audio pipeline.

## The Audio Sink

An audio sink is the ultimate destination for sound generation. You can have multiple sinks, but typically an application binds to the primary OS output device.

```c
#include "ui_audio_sink.h"

// Initialize the default audio sink (usually 48kHz, stereo, float32)
ui_audio_sink_t* main_sink = ui_audio_sink_create_default();

// Start the audio thread
ui_audio_sink_start(main_sink);
```

## Playing Audio Files

To play a sound (e.g., a notification chime or background music), you decode the asset and connect it to a mixer channel on the sink.

```c
#include "ui_asset_streamer.h"

// Load and decode an audio file (e.g., WAV, OGG)
ui_audio_source_t* chime = ui_asset_load_audio("assets/sounds/chime.ogg");

// Play the sound on the main sink with a volume of 0.8 (80%)
ui_audio_sink_play(main_sink, chime, 0.8f);
```

## Advanced Audio Pipelines

For continuous streams (like VoIP or procedurally generated audio), you can define a custom audio generator callback. The audio sink runs on a dedicated high-priority thread, so your callback must be lock-free and extremely fast.

```c
// A callback that fills the audio buffer with a simple sine wave
void sine_wave_generator(void* user_data, float* buffer, int num_frames, int num_channels) {
    float* phase = (float*)user_data;

    for (int i = 0; i < num_frames; i++) {
        float sample = sinf(*phase) * 0.2f; // Generate sample

        // Interleave channels (L, R)
        for (int c = 0; c < num_channels; c++) {
            buffer[i * num_channels + c] = sample;
        }

        *phase += 0.05f; // Increment phase
    }
}

// Create a streaming source and attach it to the sink
float my_phase = 0.0f;
ui_audio_source_t* synth = ui_audio_source_create_stream(sine_wave_generator, &my_phase);
ui_audio_sink_play(main_sink, synth, 1.0f);
```

## A/V Synchronization

When playing video, audio and video frames must be kept in sync. The `ui_av_sync.h` module uses the audio sink's playback clock as the master time source. The video renderer queries `ui_audio_sink_get_presentation_time()` to determine which video frame to draw to the screen.
