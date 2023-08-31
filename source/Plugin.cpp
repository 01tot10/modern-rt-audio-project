#include "Plugin.h"
#include "PluginEditor.h"

static const std::vector<mrta::ParameterInfo> parameters
{
    { Param::ID::Gain,  Param::Name::Gain,  Param::Unit::dB,  -0.0f,  Param::Range::GainMin,  Param::Range::GainMax,  Param::Range::GainInc,  Param::Range::GainSkw }
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
    parameterManager(*this, ProjectInfo::projectName, parameters)
{

    parameterManager.registerParameterCallback(Param::ID::Gain,
    [this] (float value, bool /*force*/)
    {
        inputGain.setGainDecibels (value);
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

    neuralNetT[0].reset();
    neuralNetT[1].reset();

    parameterManager.updateParameters(true);
}

void RTNeuralExamplePlugin::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
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

    dsp::AudioBlock<float> block (buffer);
    dsp::ProcessContextReplacing<float> context (block);

    inputGain.process (context);

    // use compile-time model
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* x = buffer.getWritePointer (ch);
        for (int n = 0; n < buffer.getNumSamples(); ++n)
        {
            float input[] = { x[n] };
            x[n] = neuralNetT[ch].forward (input);
        }
    }

    dcBlocker.process (context);
    buffer.applyGain (5.0f);

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
