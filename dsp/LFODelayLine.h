/*
  ==============================================================================

    LFODelayLine.cpp
    Created: 31 Aug 2023 4:02:19pm
    Author:  Riccardo Simionato

  ==============================================================================
*/

#include "DelayLine.h"
#include "Ramp.h"

namespace mrta
{


class LFODelayLine
{
public:
    LFODelayLine(float maxTimeMs, unsigned int numChannels);
    ~LFODelayLine();
    // No default ctor
    LFODelayLine() = delete;

    // No copy semantics
    LFODelayLine(const LFODelayLine&) = delete;
    const LFODelayLine& operator=(const LFODelayLine&) = delete;

    // No move semantics
    LFODelayLine(LFODelayLine&&) = delete;
    const LFODelayLine& operator=(LFODelayLine&&) = delete;
    
    // Update sample rate, reallocates and clear internal buffers
    void prepare(double sampleRate, float maxTimeMs, unsigned int numChannels);

    // Clear contents of internal buffer
    void clear();

    // Process audio
    void process(float* const* output, const float* const* input, unsigned int numChannels, unsigned int numSamples);

    // Set delay offset in ms
    void setOffset(float newOffsetMs);
    
    // Set delay time modulation rate in Hz
    void setModulationRate(float newModRateHz);

    // Set modulation depth in ms
    void setDepth(float newDepthMs);
private:
    double sampleRate { 48000.0 };

    mrta::DelayLine delayLine;

    mrta::Ramp<float> offsetRamp;
    mrta::Ramp<float> modDepthRamp;
    
    float phaseState[2] { 0.f, 0.f };
    float phaseInc { 0.f };

    float offsetMs { 0.f };
    float modDepthMs { 0.f };
    float outState[2] { 0.f, 0.f };
    float modRate { 10.f };


};

}
