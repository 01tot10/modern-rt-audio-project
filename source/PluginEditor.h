#pragma once

#include <JuceHeader.h>
#include "Plugin.h"

class RTNeuralExamplePluginEditor : public juce::AudioProcessorEditor
{
public:
    RTNeuralExamplePluginEditor(RTNeuralExamplePlugin&);
    ~RTNeuralExamplePluginEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    RTNeuralExamplePlugin& audioProcessor;
    mrta::GenericParameterEditor genericParameterEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RTNeuralExamplePluginEditor)
};
