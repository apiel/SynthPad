/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
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

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double newSampleRate)
{
    sampleRate = newSampleRate;
    expectedSamplesPerBlock = samplesPerBlockExpected;
}

void MainComponent::getNextAudioBlock(const AudioSourceChannelInfo &bufferToFill)
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

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint(Graphics &g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

    auto centreY = (float)getHeight() / 2.0f;
    auto radius = amplitude * 200.0f;

    if (radius >= 0.0f)
    {
        // Draw an ellipse based on the mouse position and audio volume
        g.setColour(Colours::lightgreen);

        g.fillEllipse(jmax(0.0f, lastMousePosition.x) - radius / 2.0f,
                      jmax(0.0f, lastMousePosition.y) - radius / 2.0f,
                      radius, radius);
    }

    // Draw a representative sine wave.
    Path wavePath;
    wavePath.startNewSubPath(0, centreY);

    for (auto x = 1.0f; x < (float)getWidth(); ++x)
        wavePath.lineTo(x, centreY + amplitude * (float)getHeight() * 2.0f * std::sin(x * frequency * 0.0001f));

    g.setColour(getLookAndFeel().findColour(Slider::thumbColourId));
    g.strokePath(wavePath, PathStrokeType(2.0f));
}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}

void MainComponent::mouseDown(const MouseEvent &e)
{
    mouseDrag(e);
}

void MainComponent::mouseDrag(const MouseEvent &e)
{
    lastMousePosition = e.position;

    frequency = (float)(getHeight() - e.y) * 10.0f;
    amplitude = jmin(0.9f, 0.2f * e.position.x / (float)getWidth());

    phaseDelta = (float)(MathConstants<double>::twoPi * frequency / sampleRate);

    repaint();
}

void MainComponent::mouseUp(const MouseEvent &)
{
    amplitude = 0.0f;
    repaint();
}
