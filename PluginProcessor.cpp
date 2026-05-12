#include "PluginProcessor.h"
#include "PluginEditor.h"

SeedAudioProcessor::SeedAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    grains.resize(maxGrains);
    for (int i = 0; i < visualBufferSize; ++i) visualBuffer[i] = 0.0f;
}

SeedAudioProcessor::~SeedAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout SeedAudioProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "size", 1 }, "Size", 10.0f, 300.0f, 50.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "pitch", 1 }, "Pitch", 0.5f, 2.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "density", 1 }, "Density", 0.1f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "mix", 1 }, "Mix", 0.0f, 1.0f, 0.5f));
    return { params.begin(), params.end() };
}

void SeedAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    juce::ignoreUnused(samplesPerBlock);
    mDelayBuffer.setSize(2, (int)sampleRate * 2);
    mDelayBuffer.clear();
}

void SeedAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ignoreUnused(midiMessages);
    float sRate = (float)getSampleRate();
    float sizeMs = static_cast<float>(*apvts.getRawParameterValue("size"));
    float pitch = static_cast<float>(*apvts.getRawParameterValue("pitch"));
    float density = static_cast<float>(*apvts.getRawParameterValue("density"));
    float mix = static_cast<float>(*apvts.getRawParameterValue("mix"));

    int gSizeSamples = (int)((sizeMs / 1000.0f) * sRate);
    int spawnRate = (int)((float)gSizeSamples * (1.1f - density));

    for (int i = 0; i < buffer.getNumSamples(); ++i) {
        float in = buffer.getSample(0, i);
        mDelayBuffer.setSample(0, mWritePos, in);

        if (--nextGrainCounter <= 0) {
            for (auto& g : grains) { if (!g.active) { g.start(mWritePos, gSizeSamples, pitch); break; } }
            nextGrainCounter = spawnRate;
        }

        float out = 0;
        for (auto& g : grains) {
            if (g.active) {
                float win = std::sin(juce::MathConstants<float>::pi * (float)g.currentSample / (float)g.duration);
                int rPos = (int)(g.position - g.currentSample * g.pitch) % mDelayBuffer.getNumSamples();
                if (rPos < 0) rPos += mDelayBuffer.getNumSamples();
                out += mDelayBuffer.getSample(0, rPos) * win;
                g.currentSample += 1.0;
                if (g.currentSample >= g.duration) g.active = false;
            }
        }

        float fOut = (in * (1.0f - mix)) + (out * mix * 0.5f);
        for (int c = 0; c < buffer.getNumChannels(); ++c) buffer.setSample(c, i, fOut);

        visualBuffer[visualBufferWritePos % visualBufferSize] = fOut;
        visualBufferWritePos++;
        mWritePos = (mWritePos + 1) % mDelayBuffer.getNumSamples();
    }
}

void SeedAudioProcessor::getStateInformation(juce::MemoryBlock& d) {
    auto s = apvts.copyState(); std::unique_ptr<juce::XmlElement> x(s.createXml()); copyXmlToBinary(*x, d);
}
void SeedAudioProcessor::setStateInformation(const void* d, int s) {
    std::unique_ptr<juce::XmlElement> x(getXmlFromBinary(d, s)); if (x != nullptr) apvts.replaceState(juce::ValueTree::fromXml(*x));
}
juce::AudioProcessorEditor* SeedAudioProcessor::createEditor() { return new SeedAudioEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SeedAudioProcessor(); }

// Jagoda Jazownik, 2025