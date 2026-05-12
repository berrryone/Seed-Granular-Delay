#pragma once
#include "PluginProcessor.h"

class ModernLookAndFeel : public juce::LookAndFeel_V4 {
public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, const float startAngle, const float endAngle,
        juce::Slider& s) override {
        auto radius = (float)juce::jmin(width / 2, height / 2) - 10.0f;
        auto cx = (float)x + (float)width * 0.5f;
        auto cy = (float)y + (float)height * 0.5f;
        auto angle = startAngle + sliderPos * (endAngle - startAngle);

        g.setColour(juce::Colour(0xFF202020).withAlpha(0.6f)); // Semi-transparent ring
        g.drawEllipse(cx - radius, cy - radius, radius * 2, radius * 2, 4.0f);

        juce::Path p;
        p.addCentredArc(cx, cy, radius, radius, 0.0f, startAngle, angle, true);
        g.setColour(juce::Colours::limegreen);
        g.strokePath(p, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        g.setColour(juce::Colours::white);
        g.fillEllipse(cx + std::sin(angle) * radius - 3, cy - std::cos(angle) * radius - 3, 6, 6);
    }
};

class SeedAudioEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
    SeedAudioEditor(SeedAudioProcessor&);
    ~SeedAudioEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override { repaint(); }

private:
    ModernLookAndFeel modernLF;
    SeedAudioProcessor& audioProcessor;

    juce::Image backgroundImage; // Obrazek t³a

    juce::Slider sSize, sPitch, sDens, sMix;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> aSize, aPitch, aDens, aMix;

    void drawMonitor(juce::Graphics& g, juce::Rectangle<int> area);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SeedAudioEditor)
};

// Jagoda Jazownik, 2025