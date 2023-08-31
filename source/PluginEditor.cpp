#include "Plugin.h"
#include "PluginEditor.h"

RTNeuralExamplePluginEditor::RTNeuralExamplePluginEditor(RTNeuralExamplePlugin& p) :
    AudioProcessorEditor(&p), audioProcessor(p),

    genericParameterEditor(audioProcessor.getParameterManager())
{
    addAndMakeVisible(genericParameterEditor);
    const int numOfParams { static_cast<int>(audioProcessor.getParameterManager().getParameters().size()) };
    setSize(300, numOfParams * genericParameterEditor.parameterWidgetHeight);
}

RTNeuralExamplePluginEditor::~RTNeuralExamplePluginEditor()
{
}

//==============================================================================
void RTNeuralExamplePluginEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void RTNeuralExamplePluginEditor::resized()
{
    genericParameterEditor.setBounds(getLocalBounds());
}
