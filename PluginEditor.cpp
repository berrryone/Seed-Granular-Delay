#include "PluginProcessor.h"
#include "PluginEditor.h"

SeedAudioEditor::SeedAudioEditor(SeedAudioProcessor& p) : AudioProcessorEditor(&p), audioProcessor(p) {
    // Wczytywanie tlo123.png z BinaryData
    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::tlo123_png, BinaryData::tlo123_pngSize);

    auto setup = [this](juce::Slider& s) {
        s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 18);
        s.setLookAndFeel(&modernLF);
        s.setColour(juce::Slider::textBoxTextColourId, juce::Colours::limegreen);
        s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        addAndMakeVisible(s);
        };

    setup(sSize); setup(sPitch); setup(sDens); setup(sMix);

    aSize = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.apvts, "size", sSize);
    aPitch = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.apvts, "pitch", sPitch);
    aDens = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.apvts, "density", sDens);
    aMix = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.apvts, "mix", sMix);

    startTimerHz(60);
    setSize(500, 450);
}

SeedAudioEditor::~SeedAudioEditor() {
    sSize.setLookAndFeel(nullptr); sPitch.setLookAndFeel(nullptr);
    sDens.setLookAndFeel(nullptr); sMix.setLookAndFeel(nullptr);
    stopTimer();
}

void SeedAudioEditor::paint(juce::Graphics& g) {
    if (backgroundImage.isValid()) {
        g.drawImageWithin(backgroundImage, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::fillDestination);
    }
    else {
        g.fillAll(juce::Colour(0xFF111111)); // Backup if image fails
    }

    // Optional: add a slight dark overlay to keep the text readable
    g.setColour(juce::Colours::black.withAlpha(0.2f));
    g.fillRect(getLocalBounds());

    g.setColour(juce::Colours::limegreen);
    g.setFont(juce::FontOptions("Comic Sans MS", 36.0f, juce::Font::bold));
    g.drawText("SEED GRANULAR", 25, 20, 450, 40, juce::Justification::left);

    drawMonitor(g, juce::Rectangle<int>(25, 80, 450, 180));

    g.setFont(juce::FontOptions("Comic Sans MS", 13.0f, juce::Font::plain));
    juce::StringArray n = { "SIZE_MS", "PITCH", "DENSITY", "MIX" };
    for (int i = 0; i < 4; ++i) g.drawText(n[i], 25 + (i * 115), 415, 110, 20, juce::Justification::centred);
}

void SeedAudioEditor::drawMonitor(juce::Graphics& g, juce::Rectangle<int> area) {
    g.setColour(juce::Colours::black.withAlpha(0.8f)); // Dark background for the scope
    g.fillRect(area);

    g.setColour(juce::Colours::limegreen.withAlpha(0.1f));
    for (int x = area.getX(); x < area.getRight(); x += 40) g.drawVerticalLine(x, (float)area.getY(), (float)area.getBottom());

    juce::Path w;
    w.startNewSubPath((float)area.getX(), (float)area.getCentreY());
    for (int i = 0; i < SeedAudioProcessor::visualBufferSize; ++i) {
        float x = (float)area.getX() + ((float)i / SeedAudioProcessor::visualBufferSize) * area.getWidth();
        float y = (float)area.getCentreY() + (audioProcessor.visualBuffer[i] * area.getHeight() * 0.8f);
        w.lineTo(x, y);
    }
    g.setColour(juce::Colours::limegreen); g.strokePath(w, juce::PathStrokeType(1.5f));
    g.setColour(juce::Colours::limegreen.withAlpha(0.4f)); g.drawRect(area, 1);
}

void SeedAudioEditor::resized() {
    auto r = juce::Rectangle<int>(25, 300, 450, 110);
    sSize.setBounds(r.removeFromLeft(115));
    sPitch.setBounds(r.removeFromLeft(115));
    sDens.setBounds(r.removeFromLeft(115));
    sMix.setBounds(r.removeFromLeft(115));
}

// Jagoda Jazownik, 2025