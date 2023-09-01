/*
  ==============================================================================

    LFODelayLine.cpp
    Created: 31 Aug 2023 4:02:19pm
    Author:  Riccardo Simionato

  ==============================================================================
*/

#include "LFODelayLine.h"

#ifndef M_PI
  #define M_PI 3.14159265358979323846
#endif

namespace mrta
{

LFODelayLine::LFODelayLine(float maxTimeMs, unsigned int numChannels) :
    delayLine(static_cast<unsigned int>(std::ceil(std::fmin(maxTimeMs, 1.f) * sampleRate)), numChannels), offsetRamp(0.05f), modDepthRamp(0.05f)
{
}


LFODelayLine::~LFODelayLine()
{
}

void LFODelayLine::prepare(double newSampleRate, float maxTimeMs, unsigned int numChannels)
{
    sampleRate = newSampleRate;

    delayLine.prepare(static_cast<unsigned int>(std::round(maxTimeMs * static_cast<float>(0.001 * sampleRate))), numChannels);
    delayLine.setDelaySamples(static_cast<unsigned int>(std::ceil(0.001 * sampleRate)));
    // Set fixed delay to 1ms

    offsetRamp.prepare(sampleRate, true, offsetMs * static_cast<float>(0.001 * sampleRate));
    modDepthRamp.prepare(sampleRate, true, modDepthMs * static_cast<float>(0.001 * sampleRate));

    phaseState[0] = 0.f;
    // phaseState[1] = static_cast<float>(M_PI / 2.0);
    phaseState[1] = 0.f;
    outState[0] = 0.f;
    outState[1] = 0.f;
}

void LFODelayLine::clear()
{
    delayLine.clear();
}

void LFODelayLine::process(float* const* output, const float* const* input, unsigned int numChannels, unsigned int numSamples)
{
    for (unsigned int n = 0; n < numSamples; ++n)
    {
        float lfo[2] { 0.f, 0.f };
        
        lfo[0] = 0.5f + 0.5f * std::sin(phaseState[0]);
        lfo[1] = 0.5f + 0.5f * std::sin(phaseState[1]);
    
        // Increment and wrap phase states
        phaseState[0] = std::fmod(phaseState[0] + phaseInc, static_cast<float>(2 * M_PI));
        phaseState[1] = std::fmod(phaseState[1] + phaseInc, static_cast<float>(2 * M_PI));

        // Apply mod depth and offset ramps
        modDepthRamp.applyGain(lfo, numChannels);
        offsetRamp.applySum(lfo, numChannels);

        // Read inputs
        float x[2] { 0.f, 0.f };
        for (unsigned int ch = 0; ch < numChannels; ++ch)
            // x[ch] = input[ch][n] + outState[ch];
            x[ch] = input[ch][n];

        // Process delay
        delayLine.process(outState, x, lfo, numChannels);

        // Write to output buffers
        for (unsigned int ch = 0; ch < numChannels; ++ch)
            output[ch][n] = outState[ch];
    }
}

void LFODelayLine::setOffset(float newOffsetMs)
{
    // Since the fixed delay is set to 1ms
    // We can deduct that from the offset ramp
    offsetMs = std::fmax(newOffsetMs - 1.f, 0.f);
    offsetRamp.setTarget(offsetMs * static_cast<float>(0.001 * sampleRate));
}

void LFODelayLine::setDepth(float newDepthMs)
{
    modDepthMs = std::fmax(newDepthMs, 0.f);
    modDepthRamp.setTarget(modDepthMs * static_cast<float>(0.001 * sampleRate));
}
void LFODelayLine::setModulationRate(float newModRateHz)
{
    modRate = std::fmax(newModRateHz, 0.f);
    phaseInc = static_cast<float>(2.0 * M_PI / sampleRate) * modRate;
}
}
