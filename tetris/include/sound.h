#ifndef SOUND_H
#define SOUND_H

/* ═══════════════════════════════════════════════════════════════════════════
 *  8-BIT CRUSHED AUDIO ENGINE — Procedural chiptune SFX via OpenAL
 *
 *  All sounds are generated at runtime from math — no WAV files needed.
 *  11025 Hz sample rate + 4-bit quantization = authentic retro crunch.
 * ═══════════════════════════════════════════════════════════════════════════ */

#ifdef __APPLE__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wdeprecated-declarations"
  #include <OpenAL/al.h>
  #include <OpenAL/alc.h>
  /* Keep deprecation warnings suppressed for entire file — OpenAL is
     deprecated on macOS but still fully functional and ships with the OS */
#else
  #include <AL/al.h>
  #include <AL/alc.h>
#endif

/* Suppress unused-function warnings — not all generators used by every game */
#if defined(__GNUC__) || defined(__clang__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-function"
#endif

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

/* ─────────────────── CONSTANTS ─────────────────────────── */

#define SND_SAMPLE_RATE    11025   /* Low rate = crunchy retro sound */
#define SND_MAX_SOURCES    16      /* Source pool size */
#define SND_BITCRUSH_BITS  4       /* 4-bit = 16 amplitude levels */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ─────────────────── SOUND EFFECT IDS ──────────────────── */

typedef enum {
    SFX_MOVE,           /* Piece moved left/right */
    SFX_ROTATE,         /* Piece rotated */
    SFX_HARD_DROP,      /* Hard drop slam */
    SFX_LOCK,           /* Piece locked into grid */
    SFX_HOLD,           /* Piece held/swapped */
    SFX_LINE_CLEAR,     /* 1-3 lines cleared */
    SFX_TETRIS,         /* 4 lines cleared */
    SFX_COMBO,          /* Combo continuation */
    SFX_LEVEL_UP,       /* Level increased */
    SFX_GAME_OVER,      /* Game ended */
    SFX_PAUSE,          /* Game paused/unpaused */
    SFX_MENU_SELECT,    /* Menu button clicked */
    SFX_MENU_NAV,       /* Menu navigation (theme switch etc) */
    SFX_COUNT
} SoundEffect;

/* ─────────────────── SOUND STATE ───────────────────────── */

static ALCdevice  *g_snd_device   = NULL;
static ALCcontext *g_snd_context  = NULL;
static ALuint      g_snd_sources[SND_MAX_SOURCES];
static int         g_snd_next_src = 0;
static ALuint      g_snd_buffers[SFX_COUNT];
static float       g_snd_volume   = 0.6f;
static bool        g_snd_ready    = false;
static bool        g_snd_muted    = false;

/* ─────────────────── DSP HELPERS ───────────────────────── */

/* Bitcrush: reduce to N-bit depth for that crunchy 8-bit sound */
static void snd_bitcrush(unsigned char *buf, int n, int bits) {
    int levels = 1 << bits;
    float step = 256.0f / (float)levels;
    for (int i = 0; i < n; i++) {
        buf[i] = (unsigned char)((int)((float)buf[i] / step) * step + step * 0.5f);
    }
}

/* Apply attack-decay envelope to avoid harsh clicks */
static void snd_envelope(unsigned char *buf, int n,
                          float attack_ms, float decay_ms) {
    int attack_samp = (int)(attack_ms * 0.001f * SND_SAMPLE_RATE);
    int decay_samp  = (int)(decay_ms  * 0.001f * SND_SAMPLE_RATE);
    int decay_start = n - decay_samp;

    for (int i = 0; i < n; i++) {
        float env = 1.0f;
        if (i < attack_samp && attack_samp > 0)
            env = (float)i / (float)attack_samp;
        else if (i > decay_start && decay_samp > 0)
            env = (float)(n - i) / (float)decay_samp;

        int sample = (int)buf[i] - 128;
        sample = (int)((float)sample * env);
        buf[i] = (unsigned char)(128 + sample);
    }
}

/* ─────────────────── WAVEFORM GENERATORS ──────────────── */

/* All generators: return OpenAL buffer handle, 8-bit mono, bitcrushed */

static ALuint snd_gen_square(float freq, float duration_s, float amp) {
    int n = (int)(SND_SAMPLE_RATE * duration_s);
    if (n < 1) n = 1;
    unsigned char *buf = (unsigned char *)malloc((size_t)n);
    float period = (float)SND_SAMPLE_RATE / freq;

    for (int i = 0; i < n; i++) {
        float t = fmodf((float)i, period);
        float val = (t < period * 0.5f) ? amp : -amp;
        buf[i] = (unsigned char)(128 + (int)(val * 72.0f));
    }

    snd_envelope(buf, n, 2.0f, duration_s * 400.0f);
    snd_bitcrush(buf, n, SND_BITCRUSH_BITS);

    ALuint al_buf;
    alGenBuffers(1, &al_buf);
    alBufferData(al_buf, AL_FORMAT_MONO8, buf, n, SND_SAMPLE_RATE);
    free(buf);
    return al_buf;
}

static ALuint snd_gen_noise(float duration_s, float amp) {
    int n = (int)(SND_SAMPLE_RATE * duration_s);
    if (n < 1) n = 1;
    unsigned char *buf = (unsigned char *)malloc((size_t)n);

    for (int i = 0; i < n; i++) {
        float val = ((float)(rand() % 256) / 128.0f - 1.0f) * amp;
        buf[i] = (unsigned char)(128 + (int)(val * 72.0f));
    }

    snd_envelope(buf, n, 1.0f, duration_s * 600.0f);
    snd_bitcrush(buf, n, SND_BITCRUSH_BITS);

    ALuint al_buf;
    alGenBuffers(1, &al_buf);
    alBufferData(al_buf, AL_FORMAT_MONO8, buf, n, SND_SAMPLE_RATE);
    free(buf);
    return al_buf;
}

static ALuint snd_gen_sweep(float freq_start, float freq_end,
                             float duration_s, float amp) {
    int n = (int)(SND_SAMPLE_RATE * duration_s);
    if (n < 1) n = 1;
    unsigned char *buf = (unsigned char *)malloc((size_t)n);
    float phase = 0.0f;

    for (int i = 0; i < n; i++) {
        float t = (float)i / (float)n;
        float freq = freq_start + (freq_end - freq_start) * t;
        phase += 2.0f * (float)M_PI * freq / (float)SND_SAMPLE_RATE;
        /* Square wave from sine (hard clip) */
        float sample = sinf(phase) > 0.0f ? amp : -amp;
        buf[i] = (unsigned char)(128 + (int)(sample * 72.0f));
    }

    snd_envelope(buf, n, 3.0f, duration_s * 500.0f);
    snd_bitcrush(buf, n, SND_BITCRUSH_BITS);

    ALuint al_buf;
    alGenBuffers(1, &al_buf);
    alBufferData(al_buf, AL_FORMAT_MONO8, buf, n, SND_SAMPLE_RATE);
    free(buf);
    return al_buf;
}

/* Mixed: square wave + noise for "thud" effects */
static ALuint snd_gen_thud(float freq, float duration_s, float sq_amp, float noise_amp) {
    int n = (int)(SND_SAMPLE_RATE * duration_s);
    if (n < 1) n = 1;
    unsigned char *buf = (unsigned char *)malloc((size_t)n);
    float period = (float)SND_SAMPLE_RATE / freq;

    for (int i = 0; i < n; i++) {
        float t_sq = fmodf((float)i, period);
        float sq = (t_sq < period * 0.5f) ? sq_amp : -sq_amp;
        float ns = ((float)(rand() % 256) / 128.0f - 1.0f) * noise_amp;
        /* Decay the frequency component faster */
        float decay = 1.0f - (float)i / (float)n;
        float val = sq * decay * decay + ns * decay;
        int s = (int)(val * 72.0f);
        if (s > 127) s = 127;
        if (s < -128) s = -128;
        buf[i] = (unsigned char)(128 + s);
    }

    snd_envelope(buf, n, 0.5f, duration_s * 300.0f);
    snd_bitcrush(buf, n, SND_BITCRUSH_BITS);

    ALuint al_buf;
    alGenBuffers(1, &al_buf);
    alBufferData(al_buf, AL_FORMAT_MONO8, buf, n, SND_SAMPLE_RATE);
    free(buf);
    return al_buf;
}

/* Arpeggio: rapid sequence of notes (chiptune style) */
static ALuint snd_gen_arpeggio(const float *freqs, int num_notes,
                                float note_dur_s, float amp) {
    int note_samp = (int)(SND_SAMPLE_RATE * note_dur_s);
    int n = note_samp * num_notes;
    if (n < 1) n = 1;
    unsigned char *buf = (unsigned char *)malloc((size_t)n);
    float phase = 0.0f;

    for (int i = 0; i < n; i++) {
        int note_idx = i / note_samp;
        if (note_idx >= num_notes) note_idx = num_notes - 1;
        float freq = freqs[note_idx];
        phase += 2.0f * (float)M_PI * freq / (float)SND_SAMPLE_RATE;
        /* Square wave */
        float sample = sinf(phase) > 0.0f ? amp : -amp;
        buf[i] = (unsigned char)(128 + (int)(sample * 72.0f));
    }

    snd_envelope(buf, n, 1.0f, note_dur_s * 400.0f);
    snd_bitcrush(buf, n, SND_BITCRUSH_BITS);

    ALuint al_buf;
    alGenBuffers(1, &al_buf);
    alBufferData(al_buf, AL_FORMAT_MONO8, buf, n, SND_SAMPLE_RATE);
    free(buf);
    return al_buf;
}

/* ─────────────────── INIT / CLEANUP ───────────────────── */

static int snd_init(void) {
    g_snd_device = alcOpenDevice(NULL);
    if (!g_snd_device) return -1;

    g_snd_context = alcCreateContext(g_snd_device, NULL);
    if (!g_snd_context) {
        alcCloseDevice(g_snd_device);
        g_snd_device = NULL;
        return -1;
    }
    alcMakeContextCurrent(g_snd_context);

    /* Pre-allocate source pool */
    alGenSources(SND_MAX_SOURCES, g_snd_sources);

    g_snd_ready = true;
    return 0;
}

static void snd_cleanup(void) {
    if (!g_snd_ready) return;

    alDeleteSources(SND_MAX_SOURCES, g_snd_sources);
    alDeleteBuffers(SFX_COUNT, g_snd_buffers);

    alcMakeContextCurrent(NULL);
    if (g_snd_context) alcDestroyContext(g_snd_context);
    if (g_snd_device)  alcCloseDevice(g_snd_device);
    g_snd_context = NULL;
    g_snd_device  = NULL;
    g_snd_ready   = false;
}

/* ─────────────────── PLAY ──────────────────────────────── */

static void snd_toggle_mute(void) {
    g_snd_muted = !g_snd_muted;
}

static void snd_play(SoundEffect sfx) {
    if (!g_snd_ready || g_snd_muted) return;
    if (sfx < 0 || sfx >= SFX_COUNT) return;

    ALuint src = g_snd_sources[g_snd_next_src];
    g_snd_next_src = (g_snd_next_src + 1) % SND_MAX_SOURCES;

    alSourceStop(src);
    alSourcei(src, AL_BUFFER, (ALint)g_snd_buffers[sfx]);
    alSourcef(src, AL_GAIN, g_snd_volume);
    alSourcei(src, AL_LOOPING, AL_FALSE);
    alSourcePlay(src);
}

/* Play with custom volume multiplier (0.0 – 1.0) */
static void snd_play_vol(SoundEffect sfx, float vol) {
    if (!g_snd_ready || g_snd_muted) return;
    if (sfx < 0 || sfx >= SFX_COUNT) return;

    ALuint src = g_snd_sources[g_snd_next_src];
    g_snd_next_src = (g_snd_next_src + 1) % SND_MAX_SOURCES;

    alSourceStop(src);
    alSourcei(src, AL_BUFFER, (ALint)g_snd_buffers[sfx]);
    alSourcef(src, AL_GAIN, g_snd_volume * vol);
    alSourcei(src, AL_LOOPING, AL_FALSE);
    alSourcePlay(src);
}

/* ─────────────────── GENERATE ALL GAME SFX ────────────── */

static void snd_generate_all(void) {
    if (!g_snd_ready) return;

    /* Move piece — short low blip */
    g_snd_buffers[SFX_MOVE] = snd_gen_square(180.0f, 0.04f, 0.6f);

    /* Rotate — short higher blip */
    g_snd_buffers[SFX_ROTATE] = snd_gen_square(360.0f, 0.04f, 0.7f);

    /* Hard drop — low thud with noise */
    g_snd_buffers[SFX_HARD_DROP] = snd_gen_thud(80.0f, 0.12f, 0.9f, 0.7f);

    /* Lock piece — medium click */
    g_snd_buffers[SFX_LOCK] = snd_gen_thud(200.0f, 0.06f, 0.6f, 0.3f);

    /* Hold piece — swap blip */
    g_snd_buffers[SFX_HOLD] = snd_gen_square(500.0f, 0.05f, 0.5f);

    /* Line clear — ascending sweep */
    g_snd_buffers[SFX_LINE_CLEAR] = snd_gen_sweep(300.0f, 900.0f, 0.25f, 0.8f);

    /* Tetris (4 lines) — longer, higher sweep */
    g_snd_buffers[SFX_TETRIS] = snd_gen_sweep(250.0f, 1200.0f, 0.4f, 1.0f);

    /* Combo — quick ascending arpeggio */
    {
        float notes[] = {440.0f, 554.0f, 660.0f, 880.0f};
        g_snd_buffers[SFX_COMBO] = snd_gen_arpeggio(notes, 4, 0.05f, 0.7f);
    }

    /* Level up — triumphant ascending arpeggio */
    {
        float notes[] = {523.0f, 659.0f, 784.0f, 1047.0f};
        g_snd_buffers[SFX_LEVEL_UP] = snd_gen_arpeggio(notes, 4, 0.08f, 0.8f);
    }

    /* Game over — descending sweep */
    g_snd_buffers[SFX_GAME_OVER] = snd_gen_sweep(600.0f, 80.0f, 0.8f, 0.9f);

    /* Pause — medium tone */
    g_snd_buffers[SFX_PAUSE] = snd_gen_square(440.0f, 0.1f, 0.5f);

    /* Menu select — crisp click */
    g_snd_buffers[SFX_MENU_SELECT] = snd_gen_square(660.0f, 0.05f, 0.7f);

    /* Menu navigation — subtle tick */
    g_snd_buffers[SFX_MENU_NAV] = snd_gen_square(880.0f, 0.03f, 0.4f);
}

/* Restore diagnostic settings */
#if defined(__GNUC__) || defined(__clang__)
  #pragma GCC diagnostic pop
#endif
#ifdef __APPLE__
  #pragma clang diagnostic pop
#endif

#endif /* SOUND_H */
