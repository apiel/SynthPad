/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

struct SineWaveSound : public juce::SynthesiserSound
{
    SineWaveSound() {}

    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

//==============================================================================
struct SineWaveVoice : public juce::SynthesiserVoice
{
    SineWaveVoice() {}

    bool canPlaySound(juce::SynthesiserSound *sound) override
    {
        return dynamic_cast<SineWaveSound *>(sound) != nullptr;
    }

    void startNote(int midiNoteNumber, float velocity,
                   juce::SynthesiserSound *, int /*currentPitchWheelPosition*/) override
    {
        currentAngle = 0.0;
        level = velocity * 0.15;
        tailOff = 0.0;

        auto cyclesPerSecond = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
        auto cyclesPerSample = cyclesPerSecond / getSampleRate();

        angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;
    }

    void stopNote(float /*velocity*/, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            if (tailOff == 0.0)
                tailOff = 1.0;
        }
        else
        {
            clearCurrentNote();
            angleDelta = 0.0;
        }
    }

    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}

    void renderNextBlock(juce::AudioSampleBuffer &outputBuffer, int startSample, int numSamples) override
    {
        if (angleDelta != 0.0)
        {
            if (tailOff > 0.0) // [7]
            {
                while (--numSamples >= 0)
                {
                    auto currentSample = (float)(std::sin(currentAngle) * level * tailOff);

                    for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                        outputBuffer.addSample(i, startSample, currentSample);

                    currentAngle += angleDelta;
                    ++startSample;

                    tailOff *= 0.99; // [8]

                    if (tailOff <= 0.005)
                    {
                        clearCurrentNote(); // [9]

                        angleDelta = 0.0;
                        break;
                    }
                }
            }
            else
            {
                while (--numSamples >= 0) // [6]
                {
                    auto currentSample = (float)(std::sin(currentAngle) * level);

                    for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                        outputBuffer.addSample(i, startSample, currentSample);

                    currentAngle += angleDelta;
                    ++startSample;
                }
            }
        }
    }

private:
    double currentAngle = 0.0, angleDelta = 0.0, level = 0.0, tailOff = 0.0;
};

//==============================================================================
class SynthAudioSource : public juce::AudioSource
{
public:
    SynthAudioSource(juce::MidiKeyboardState &keyState)
        : keyboardState(keyState)
    {
        for (auto i = 0; i < 4; ++i) // [1]
            synth.addVoice(new SineWaveVoice());

        synth.addSound(new SineWaveSound()); // [2]
    }

    void setUsingSineWaveSound()
    {
        synth.clearSounds();
    }

    void prepareToPlay(int /*samplesPerBlockExpected*/, double sampleRate) override
    {
        synth.setCurrentPlaybackSampleRate(sampleRate); // [3]
    }

    void releaseResources() override {}

    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override
    {
        bufferToFill.clearActiveBufferRegion();

        juce::MidiBuffer incomingMidi;
        keyboardState.processNextMidiBuffer(incomingMidi, bufferToFill.startSample,
                                            bufferToFill.numSamples, true); // [4]

        synth.renderNextBlock(*bufferToFill.buffer, incomingMidi,
                              bufferToFill.startSample, bufferToFill.numSamples); // [5]
    }

private:
    juce::MidiKeyboardState &keyboardState;
    juce::Synthesiser synth;
};

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public AudioAppComponent
{
public:
    //==============================================================================
    MainComponent() : synthAudioSource(keyboardState)
    {
        // Make sure you set the size of the component after
        // you add any child components.
        setSize(800, 600);

        audioDeviceManager.setCurrentAudioDeviceType("ALSA", false);

        addAndMakeVisible(diagnosticsBox);
        diagnosticsBox.setMultiLine(true);
        diagnosticsBox.setReturnKeyStartsNewLine(true);
        diagnosticsBox.setReadOnly(true);
        diagnosticsBox.setScrollbarsShown(true);
        diagnosticsBox.setCaretVisible(false);
        diagnosticsBox.setPopupMenuEnabled(true);

        // Some platforms require permissions to open input channels so request that here
        if (RuntimePermissions::isRequired(RuntimePermissions::recordAudio) && !RuntimePermissions::isGranted(RuntimePermissions::recordAudio))
        {
            RuntimePermissions::request(RuntimePermissions::recordAudio,
                                        [&](bool granted)
                                        { if (granted)  setAudioChannels (2, 2); });
        }
        else
        {
            // Specify the number of input and output channels that we want to open
            setAudioChannels(2, 2);
        }
    }

    ~MainComponent()
    {
        // This shuts down the audio device and clears the audio source.
        shutdownAudio();
    }

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        synthAudioSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    }

    // void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override
    // {
    //     // synthAudioSource.getNextAudioBlock(bufferToFill);

    //     // https://docs.juce.com/master/tutorial_plugin_examples.html
    //     juce::MidiBuffer midi;
    //     if (amplitude == 0.0f)
    //     {
    //         midi.addEvent(juce::MidiMessage::noteOff(1, 64), 10);
    //     }
    //     else
    //     {
    //         midi.addEvent(juce::MidiMessage::noteOn(1, 64, (juce::uint8)127), 10);
    //     }

    //     synthAudioSource.getNextAudioBlock(bufferToFill, midi);
    // }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override
    {
        synthAudioSource.getNextAudioBlock(bufferToFill);
    }

    void releaseResources() override
    {
        synthAudioSource.releaseResources();
    }

    //==============================================================================
    void paint(Graphics &g) override
    {
        int width = getWidth();
        int height = getHeight();

        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

        g.setColour(juce::Colours::orange);
        g.drawLine(0, headerHeight, width, headerHeight, 2);

        g.setColour(juce::Colours::grey);
        // for (int x = 0; x < width; x += noteSize)
        // {
        //     g.drawLine(x, headerHeight, x, height, 1);
        // }
        // for (int y = headerHeight; y < height; y += noteSize)
        // {
        //     g.drawLine(0, y, width, y, 1);
        // }

        int radius = 10;
        g.setColour(Colours::lightgreen);
        g.fillEllipse(jmax(0.0f, lastMousePosition.x) - radius / 2.0f,
                      jmax(headerHeight, lastMousePosition.y) - radius / 2.0f,
                      radius, radius);
    }

    void resized() override
    {
        diagnosticsBox.setBounds(10, 10, getWidth() / 3, 100);
    }

    void mouseDown(const MouseEvent &e) override
    {
        mouseDrag(e);
    }

    void mouseDrag(const MouseEvent &e) override
    {
        int width = getWidth();
        int height = getHeight();
        int x = 1 + (e.position.getX() / noteSize);
        int y = 1 + ((e.position.getY() - headerHeight) / noteSize);
        int note = x * y;
        // printf("calc %d\n", note);
        note = note % 128;
        // printf("note: %d\n", note);
        if (note != lastNote)
        {
            keyboardState.noteOff(1, lastNote, 0);
            keyboardState.noteOn(1, note, 0.8f);
            // printf("node %d\n", note);
            lastNote = note;
        }
        lastMousePosition = e.position;
        repaint();
    }

    void mouseUp(const MouseEvent &) override
    {
        keyboardState.noteOff(1, lastNote, 0);
        repaint();
    }

    void logMessage(const String &m)
    {
        diagnosticsBox.moveCaretToEnd();
        diagnosticsBox.insertTextAtCaret(m + newLine);
    }

private:
    AudioDeviceManager audioDeviceManager;
    TextEditor diagnosticsBox;
    SynthAudioSource synthAudioSource;
    juce::MidiKeyboardState keyboardState;

    Point<float> lastMousePosition;
    int lastNote = 255;
    const int noteSize = 50;
    const float headerHeight = 150.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
