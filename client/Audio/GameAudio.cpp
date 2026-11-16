#include "GameAudio.hh"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

namespace {

    constexpr unsigned int SAMPLE_RATE = 44100;

    bool sound_enabled = true;
    bool loaded = false;

    // A fixed sequence rather than rand(): the sounds should be identical on
    // every run and on every machine.
    struct Noise {
        std::uint32_t state = 0x13579BDFu;

        float next() {
            state = state * 1664525u + 1013904223u;
            return static_cast<float>(state >> 8u) / 8388608.f - 1.f;
        }
    };

    // One percussive note: a sine at `frequency` with an exponential decay,
    // plus a little noise at the attack for the woody edge a sine alone lacks.
    void AppendTone(std::vector<sf::Int16> &samples, float seconds, float frequency,
                    float decay, float noise_mix, float gain) {
        Noise noise;
        const auto count = static_cast<std::size_t>(seconds * SAMPLE_RATE);
        samples.reserve(samples.size() + count);
        for (std::size_t i = 0; i < count; i++) {
            const float t = static_cast<float>(i) / SAMPLE_RATE;
            const float envelope = std::exp(-decay * t);
            // The noise fades much faster than the tone, so it reads as the
            // strike rather than as hiss.
            const float body = std::sin(6.2831853f * frequency * t);
            const float edge = noise.next() * std::exp(-decay * 6.f * t);
            const float value = (body * (1.f - noise_mix) + edge * noise_mix) * envelope * gain;
            samples.push_back(static_cast<sf::Int16>(std::clamp(value, -1.f, 1.f) * 32000.f));
        }
    }

    void AppendSilence(std::vector<sf::Int16> &samples, float seconds) {
        samples.insert(samples.end(), static_cast<std::size_t>(seconds * SAMPLE_RATE), 0);
    }

    bool Build(sf::SoundBuffer &buffer, const std::vector<sf::Int16> &samples) {
        return buffer.loadFromSamples(samples.data(), samples.size(), 1, SAMPLE_RATE);
    }

    // Buffers must outlive the sounds that reference them, so both live in
    // function-local statics with the same lifetime.
    struct Voices {
        sf::SoundBuffer move_buffer, capture_buffer, check_buffer, over_buffer, click_buffer;
        sf::Sound move, capture, check, over, click;
    };

    Voices &voices() {
        static Voices instance;
        return instance;
    }

    void Play(sf::Sound &sound) {
        if (!sound_enabled || !loaded) return;
        sound.play();
    }

}

void GosChess::LoadSounds() {
    if (loaded) return;
    Voices &v = voices();

    std::vector<sf::Int16> samples;

    // A piece set down: short, low, mostly thud.
    samples.clear();
    AppendTone(samples, 0.09f, 196.f, 42.f, 0.35f, 0.55f);
    if (!Build(v.move_buffer, samples)) return;

    // A capture: lower and longer, with more grit.
    samples.clear();
    AppendTone(samples, 0.14f, 132.f, 30.f, 0.55f, 0.75f);
    if (!Build(v.capture_buffer, samples)) return;

    // Check: two quick rising notes, so it cuts through.
    samples.clear();
    AppendTone(samples, 0.07f, 660.f, 26.f, 0.05f, 0.45f);
    AppendTone(samples, 0.12f, 880.f, 20.f, 0.05f, 0.45f);
    if (!Build(v.check_buffer, samples)) return;

    // The end of a game: two notes falling away.
    samples.clear();
    AppendTone(samples, 0.18f, 440.f, 11.f, 0.03f, 0.5f);
    AppendSilence(samples, 0.02f);
    AppendTone(samples, 0.40f, 294.f, 7.f, 0.03f, 0.5f);
    if (!Build(v.over_buffer, samples)) return;

    // A button: barely there, just enough to confirm the press.
    samples.clear();
    AppendTone(samples, 0.035f, 1050.f, 90.f, 0.15f, 0.3f);
    if (!Build(v.click_buffer, samples)) return;

    v.move.setBuffer(v.move_buffer);
    v.capture.setBuffer(v.capture_buffer);
    v.check.setBuffer(v.check_buffer);
    v.over.setBuffer(v.over_buffer);
    v.click.setBuffer(v.click_buffer);
    loaded = true;
}

bool GosChess::SoundsLoaded() { return loaded; }

void GosChess::SetSoundEnabled(bool enabled) { sound_enabled = enabled; }

bool GosChess::SoundEnabled() { return sound_enabled; }

void GosChess::PlayMoveSound() { Play(voices().move); }

void GosChess::PlayCaptureSound() { Play(voices().capture); }

void GosChess::PlayCheckSound() { Play(voices().check); }

void GosChess::PlayGameOverSound() { Play(voices().over); }

void GosChess::PlayUiClickSound() { Play(voices().click); }

bool GosChess::SaveSoundsTo(const std::string &directory) {
    if (!loaded) return false;
    Voices &v = voices();
    const std::pair<const sf::SoundBuffer *, const char *> all[] = {
            {&v.move_buffer,    "move.wav"},
            {&v.capture_buffer, "capture.wav"},
            {&v.check_buffer,   "check.wav"},
            {&v.over_buffer,    "gameover.wav"},
            {&v.click_buffer,   "click.wav"},
    };
    bool ok = true;
    for (const auto &[buffer, name]: all) {
        ok = buffer->saveToFile(directory + "/" + name) && ok;
    }
    return ok;
}
