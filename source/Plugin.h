#pragma once

#include <JuceHeader.h>
#include <RTNeural/RTNeural.h>

namespace Param
{
    namespace ID
    {
        static const juce::String Gain { "gain" };
    }

    namespace Name
    {
        static const juce::String Gain { "Gain" };
    }

    namespace Unit
    {
        static const juce::String dB { "dB" };
    }

    namespace Range
    {
        static constexpr float GainMin { -36.0f };
        static constexpr float GainMax { -24.0f };
        static constexpr float GainInc { .1f };
        static constexpr float GainSkw { 1.f };
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
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

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

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

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

    dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, dsp::IIR::Coefficients<float>> dcBlocker;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RTNeuralExamplePlugin)
};
