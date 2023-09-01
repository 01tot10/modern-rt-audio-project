#pragma once

#include <JuceHeader.h>
#include <RTNeural/RTNeural.h>
#include "LFODelayLine.h"

namespace Param
{
    namespace ID
    {
        static const juce::String Gain { "gain" };
        static const juce::String Enabled { "enabled" };
        static const juce::String Offset { "offset" };
        static const juce::String Depth { "depth" };
        static const juce::String Rate { "rate" };
    }

    namespace Name
    {
        static const juce::String Gain { "Gain" };
        static const juce::String Enabled { "Enabled" };
        static const juce::String Offset { "Offset" };
        static const juce::String Depth { "Depth" };
        static const juce::String Rate { "Rate" };
    }

    namespace Unit
    {
        static const juce::String dB { "dB" };
        static const juce::String Ms { "ms" };
        static const juce::String Hz { "Hz" };
        static const juce::String Pct { "%" };
    }

    namespace Range
    {
        static constexpr float GainMin { -24.0f };
        static constexpr float GainMax { 12.0f };
        static constexpr float GainInc { 0.1f };
        static constexpr float GainSkw { 1.f };

        static constexpr float OffsetMin { 1.f };
        static constexpr float OffsetMax { 50.f };
        static constexpr float OffsetInc { 0.1f };
        static constexpr float OffsetSkw { 0.5f };

        static constexpr float DepthMin { 0.f };
        static constexpr float DepthMax { 25.f };
        static constexpr float DepthInc { 0.1f };
        static constexpr float DepthSkw { 0.7f };
    
        static constexpr float RateMin { 0.1f };
        static constexpr float RateMax { 5.f };
        static constexpr float RateInc { 0.1f };
        static constexpr float RateSkw { 0.5f };

        static const juce::String EnabledOff { "Off" };
        static const juce::String EnabledOn { "On" };
    }
}

class RTNeuralExamplePlugin  : public AudioProcessor
{
public:
    //==============================================================================
    RTNeuralExamplePlugin();
    ~RTNeuralExamplePlugin();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    mrta::ParameterManager& getParameterManager() { return parameterManager; }

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    // static const unsigned int MaxDelaySizeSamples { 1 << 12 };
    static const unsigned int MaxChannels { 2 };
    static const unsigned int MaxProcessBlockSamples{ 32 };

private:
    //==============================================================================
    mrta::ParameterManager parameterManager;

    // input gain
    dsp::Gain<float> inputGain;

    // neural network
    RTNeural::ModelT<float, 1, 1,
        RTNeural::GRULayerT<float, 1, 64>,
        RTNeural::DenseT<float, 64, 1>
    > neuralNetT[2];

    // delay
    mrta::LFODelayLine lfoDelayLine;
    mrta::Ramp<float> enableRamp;
    juce::AudioBuffer<float> fxBuffer;

    dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, dsp::IIR::Coefficients<float>> dcBlocker;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RTNeuralExamplePlugin)
};
