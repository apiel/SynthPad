/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

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

    //==============================================================================
    void prepareToPlay(int samplesPerBlockExpected, double newSampleRate) override
    {
        sampleRate = newSampleRate;
        expectedSamplesPerBlock = samplesPerBlockExpected;
    }

    void getNextAudioBlock(const AudioSourceChannelInfo &bufferToFill) override
    {
        bufferToFill.clearActiveBufferRegion();
        auto originalPhase = phase;

        for (auto chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan)
        {
            phase = originalPhase;

            auto *channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);

            for (auto i = 0; i < bufferToFill.numSamples; ++i)
            {
                channelData[i] = amplitude * std::sin(phase);

                // increment the phase step for the next sample
                phase = std::fmod(phase + phaseDelta, MathConstants<float>::twoPi);
            }
        }
    }

    void releaseResources() override
    {
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

private:
    //==============================================================================
    // Your private member variables go here...
    float phase = 0.0f;
    float phaseDelta = 0.0f;
    float frequency = 5000.0f;
    float amplitude = 0.2f;

    double sampleRate = 0.0;
    int expectedSamplesPerBlock = 0;
    Point<float> lastMousePosition;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
