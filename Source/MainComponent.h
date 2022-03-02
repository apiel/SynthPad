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
    SynthAudioSource()
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
        // incomingMidi.addEvent

        synth.renderNextBlock(*bufferToFill.buffer, incomingMidi,
                              bufferToFill.startSample, bufferToFill.numSamples); // [5]
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill, juce::MidiBuffer incomingMidi)
    {
        bufferToFill.clearActiveBufferRegion();

        synth.renderNextBlock(*bufferToFill.buffer, incomingMidi,
                              bufferToFill.startSample, bufferToFill.numSamples); // [5]
    }

private:
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
    MainComponent()
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

        // printf("yoyoyoyoyo\n");
        dumpDeviceInfo();
    }

    ~MainComponent()
    {
        // This shuts down the audio device and clears the audio source.
        shutdownAudio();
    }

    //==============================================================================
    // void prepareToPlay(int samplesPerBlockExpected, double newSampleRate) override
    // {
    //     sampleRate = newSampleRate;
    //     expectedSamplesPerBlock = samplesPerBlockExpected;
    // }

    // void getNextAudioBlock(const AudioSourceChannelInfo &bufferToFill) override
    // {
    //     bufferToFill.clearActiveBufferRegion();
    //     auto originalPhase = phase;

    //     for (auto chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan)
    //     {
    //         phase = originalPhase;

    //         auto *channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);

    //         for (auto i = 0; i < bufferToFill.numSamples; ++i)
    //         {
    //             channelData[i] = amplitude * std::sin(phase);

    //             // increment the phase step for the next sample
    //             phase = std::fmod(phase + phaseDelta, MathConstants<float>::twoPi);
    //         }
    //     }
    // }

    // void releaseResources() override
    // {
    // }

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        synthAudioSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override
    {
        // synthAudioSource.getNextAudioBlock(bufferToFill);

        // https://docs.juce.com/master/tutorial_plugin_examples.html
        juce::MidiBuffer midi;
        if (amplitude == 0.0f)
        {
            midi.addEvent(juce::MidiMessage::noteOff(1, 64), 10);
        }
        else
        {
            midi.addEvent(juce::MidiMessage::noteOn(1, 64, (juce::uint8)127), 10);
        }

        synthAudioSource.getNextAudioBlock(bufferToFill, midi);
    }

    void releaseResources() override
    {
        synthAudioSource.releaseResources();
    }

    //==============================================================================
    void paint(Graphics &g) override
    {
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

        g.setColour(juce::Colours::orange);
        auto headerHeight = 150.0f;
        g.drawLine(0, headerHeight, getWidth(), headerHeight, 2);

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
        lastMousePosition = e.position;

        frequency = (float)(getHeight() - e.y) * 10.0f;
        amplitude = jmin(0.9f, 0.2f * e.position.x / (float)getWidth());

        phaseDelta = (float)(MathConstants<double>::twoPi * frequency / sampleRate);

        repaint();
    }

    void mouseUp(const MouseEvent &) override
    {
        amplitude = 0.0f;
        repaint();
    }

    void logMessage(const String &m)
    {
        diagnosticsBox.moveCaretToEnd();
        diagnosticsBox.insertTextAtCaret(m + newLine);
    }

    void dumpDeviceInfo()
    {
        logMessage("--------------------------------------");
        logMessage("Current audio device type: " + (audioDeviceManager.getCurrentDeviceTypeObject() != nullptr
                                                        ? audioDeviceManager.getCurrentDeviceTypeObject()->getTypeName()
                                                        : "<none>"));

        if (AudioIODevice *device = audioDeviceManager.getCurrentAudioDevice())
        {
            logMessage("Current audio device: " + device->getName().quoted());
            logMessage("Sample rate: " + String(device->getCurrentSampleRate()) + " Hz");
            logMessage("Block size: " + String(device->getCurrentBufferSizeSamples()) + " samples");
            logMessage("Output Latency: " + String(device->getOutputLatencyInSamples()) + " samples");
            logMessage("Input Latency: " + String(device->getInputLatencyInSamples()) + " samples");
            logMessage("Bit depth: " + String(device->getCurrentBitDepth()));
            logMessage("Input channel names: " + device->getInputChannelNames().joinIntoString(", "));
            // logMessage("Active input channels: " + getListOfActiveBits(device->getActiveInputChannels()));
            logMessage("Output channel names: " + device->getOutputChannelNames().joinIntoString(", "));
            // logMessage("Active output channels: " + getListOfActiveBits(device->getActiveOutputChannels()));
        }
        else
        {
            logMessage("No audio device open");
        }
    }

private:
    AudioDeviceManager audioDeviceManager;
    TextEditor diagnosticsBox;
    SynthAudioSource synthAudioSource;

    float phase = 0.0f;
    float phaseDelta = 0.0f;
    float frequency = 5000.0f;
    float amplitude = 0.2f;

    double sampleRate = 0.0;
    int expectedSamplesPerBlock = 0;
    Point<float> lastMousePosition;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
