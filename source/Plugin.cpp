#include "Plugin.h"
#include "PluginEditor.h"

#include <algorithm>

static const std::vector<mrta::ParameterInfo> parameters
{
    { Param::ID::Enabled,  Param::Name::Enabled,  Param::Range::EnabledOff, Param::Range::EnabledOn, true },
    { Param::ID::Gain,  Param::Name::Gain,  Param::Unit::dB,  -0.0f,  Param::Range::GainMin,  Param::Range::GainMax,  Param::Range::GainInc,  Param::Range::GainSkw },
    { Param::ID::Offset,   Param::Name::Offset,   Param::Unit::Ms,  20.f,  Param::Range::OffsetMin,   Param::Range::OffsetMax,   Param::Range::OffsetInc,   Param::Range::OffsetSkw },
    { Param::ID::Depth,    Param::Name::Depth,    Param::Unit::Ms,  20.f,  Param::Range::DepthMin,    Param::Range::DepthMax,    Param::Range::DepthInc,    Param::Range::DepthSkw },
    { Param::ID::Rate,     Param::Name::Rate,     Param::Unit::Hz,  2.0f, Param::Range::RateMin,     Param::Range::RateMax,     Param::Range::RateInc,     Param::Range::RateSkw }
};

//==============================================================================
RTNeuralExamplePlugin::RTNeuralExamplePlugin() :
#if JUCE_IOS || JUCE_MAC
    AudioProcessor (juce::JUCEApplicationBase::isStandaloneApp() ?
        BusesProperties().withInput ("Input", juce::AudioChannelSet::mono(), true)
                         .withOutput ("Output", juce::AudioChannelSet::stereo(), true) :
        BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
#else
    AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
#endif
    parameterManager(*this, ProjectInfo::projectName, parameters),
    lfoDelayLine(100.f, 2),
    enableRamp(0.05f)
{

    parameterManager.registerParameterCallback(Param::ID::Enabled,
    [this](float newValue, bool force)
    {
        enableRamp.setTarget(std::fmin(std::fmax(newValue, 0.f), 1.f), force);
    });

    parameterManager.registerParameterCallback(Param::ID::Gain,
    [this] (float value, bool /*force*/)
    {
        inputGain.setGainDecibels (value);
    });

    parameterManager.registerParameterCallback(Param::ID::Offset,
    [this] (float newValue, bool /*force*/)
    {
        lfoDelayLine.setOffset(newValue);
    });

    
    parameterManager.registerParameterCallback(Param::ID::Depth,
    [this](float newValue, bool /*force*/)
    {
        lfoDelayLine.setDepth(newValue);
    });
    
    parameterManager.registerParameterCallback(Param::ID::Rate,
    [this] (float newValue, bool /*force*/)
    {
        lfoDelayLine.setModulationRate(newValue);
    });

    MemoryInputStream jsonStream (BinaryData::gru_torch_chowtape_json, BinaryData::gru_torch_chowtape_jsonSize, false);
    auto jsonInput = nlohmann::json::parse (jsonStream.readEntireStreamAsString().toStdString());

    // Left
    {
        auto& gru = neuralNetT[0].get<0>();
        RTNeural::torch_helpers::loadGRU<float> (jsonInput, "gru.", gru);

        auto& dense = neuralNetT[0].get<1>();
        RTNeural::torch_helpers::loadDense<float> (jsonInput, "dense.", dense);
    }

    // Right
    {
        auto& gru = neuralNetT[1].get<0>();
        RTNeural::torch_helpers::loadGRU<float> (jsonInput, "gru.", gru);

        auto& dense = neuralNetT[1].get<1>();
        RTNeural::torch_helpers::loadDense<float> (jsonInput, "dense.", dense);
    }
}

RTNeuralExamplePlugin::~RTNeuralExamplePlugin()
{
}

//==============================================================================
const String RTNeuralExamplePlugin::getName() const
{
    return JucePlugin_Name;
}

bool RTNeuralExamplePlugin::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool RTNeuralExamplePlugin::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool RTNeuralExamplePlugin::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double RTNeuralExamplePlugin::getTailLengthSeconds() const
{
    return 0.0;
}

int RTNeuralExamplePlugin::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int RTNeuralExamplePlugin::getCurrentProgram()
{
    return 0;
}

void RTNeuralExamplePlugin::setCurrentProgram (int index)
{
}

const String RTNeuralExamplePlugin::getProgramName (int index)
{
    return {};
}

void RTNeuralExamplePlugin::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void RTNeuralExamplePlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    *dcBlocker.state = *dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, 35.0f);

    dsp::ProcessSpec spec { sampleRate, static_cast<uint32> (samplesPerBlock), 2 };
    inputGain.prepare (spec);
    inputGain.setRampDurationSeconds (0.05);
    dcBlocker.prepare (spec);

    const unsigned int numChannels { static_cast<unsigned int>(std::max(getMainBusNumInputChannels(), getMainBusNumOutputChannels())) };
    lfoDelayLine.prepare(sampleRate, 100.f, numChannels);
    enableRamp.prepare(sampleRate);

    fxBuffer.setSize(static_cast<int>(numChannels), samplesPerBlock);
    fxBuffer.clear();

    neuralNetT[0].reset();
    neuralNetT[1].reset();

    parameterManager.updateParameters(true);
}

void RTNeuralExamplePlugin::releaseResources()
{
    lfoDelayLine.clear();
}

bool RTNeuralExamplePlugin::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void RTNeuralExamplePlugin::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    parameterManager.updateParameters();

    const unsigned int numChannels { static_cast<unsigned int>(buffer.getNumChannels()) };
    const unsigned int numSamples { static_cast<unsigned int>(buffer.getNumSamples()) };

    dsp::AudioBlock<float> block (buffer);
    dsp::ProcessContextReplacing<float> context (block);

    inputGain.process (context);

    // use compile-time model
    // for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    // {
    //     auto* x = buffer.getWritePointer (ch);
    //     for (int n = 0; n < buffer.getNumSamples(); ++n)
    //     {
    //         float input[] = { x[n] };
    //         x[n] = neuralNetT[ch].forward (input);
    //     }
    // }
    // buffer.applyGain (5.0f);

    dcBlocker.process (context);

    for (int ch = 0; ch < static_cast<int>(numChannels); ++ch)
        fxBuffer.copyFrom(ch, 0, buffer, ch, 0, static_cast<int>(numSamples));

    lfoDelayLine.process(fxBuffer.getArrayOfWritePointers(), fxBuffer.getArrayOfReadPointers(), numChannels, numSamples);
    enableRamp.applyGain(fxBuffer.getArrayOfWritePointers(), numChannels, numSamples);

    for (int ch = 0; ch < static_cast<int>(numChannels); ++ch)
        buffer.copyFrom(ch, 0, fxBuffer, ch, 0, static_cast<int>(numSamples));

    ignoreUnused (midiMessages);
}

//==============================================================================
bool RTNeuralExamplePlugin::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* RTNeuralExamplePlugin::createEditor()
{
    return new RTNeuralExamplePluginEditor (*this);
}

//==============================================================================
void RTNeuralExamplePlugin::getStateInformation (MemoryBlock& destData)
{
    parameterManager.getStateInformation(destData);
}

void RTNeuralExamplePlugin::setStateInformation (const void* data, int sizeInBytes)
{
    parameterManager.setStateInformation(data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RTNeuralExamplePlugin();
}
