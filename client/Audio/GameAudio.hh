#ifndef GOSCHESS_GAMEAUDIO_HH
#define GOSCHESS_GAMEAUDIO_HH

#include <string>

// Sound.
//
// The waveforms are synthesised when the game starts rather than loaded from
// files: a handful of short percussive blips is a few lines of arithmetic, and
// generating them avoids adding binary assets to the repository whose licence
// would have to be tracked. It also means the sounds cannot go missing.

namespace GosChess {

    // Builds the waveforms. Safe to call more than once; failure is not fatal,
    // the game simply runs silent.
    void LoadSounds();

    // False when the waveforms could not be built or no audio device opened.
    // Worth reporting rather than leaving the game mysteriously silent.
    bool SoundsLoaded();

    void SetSoundEnabled(bool enabled);

    bool SoundEnabled();

    // A piece landing on an empty square.
    void PlayMoveSound();

    // A piece landing on an occupied one - heavier, so the two are
    // distinguishable without looking.
    void PlayCaptureSound();

    // Overrides the other two when a move gives check.
    void PlayCheckSound();

    void PlayGameOverSound();

    // Any button, in the menu or beside the board.
    void PlayUiClickSound();

    // Writes the synthesised waveforms out as WAV files. For checking that
    // what was generated is actually audible, which "it loaded" does not tell
    // you. Returns false if anything could not be written.
    bool SaveSoundsTo(const std::string &directory);

}

#endif
