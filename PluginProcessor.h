#pragma once
#include <JuceHeader.h>
#include <vector>

struct Grain {
    double position, currentSample, duration;
    float pitch;
    bool active = false;

    void start(double pos, double dur, float p) {
        position = pos; duration = dur; pitch = p;
        currentSample = 0; active = true;
    }
};

class SeedAudioProcessor : public juce::AudioProcessor {
public:
    SeedAudioProcessor();
    ~SeedAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "SEED"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    static const int visualBufferSize = 512;
    float visualBuffer[visualBufferSize];
    std::atomic<int> visualBufferWritePos{ 0 };

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioBuffer<float> mDelayBuffer;
    int mWritePos = 0;
    std::vector<Grain> grains;
    int nextGrainCounter = 0;
    const int maxGrains = 25;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SeedAudioProcessor)
};

// Jagoda Jazownik, 2025