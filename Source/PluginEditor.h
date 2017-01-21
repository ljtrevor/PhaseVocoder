/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.
	GUI file

  ==============================================================================
*/

#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"


//==============================================================================
/**
*/
class PitchShifterAudioProcessorEditor  : public AudioProcessorEditor, private Slider::Listener
{
public:
    PitchShifterAudioProcessorEditor (PitchShifterAudioProcessor&);
    ~PitchShifterAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
	//Slider Listener
	void sliderValueChanged(Slider* slider);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PitchShifterAudioProcessor& processor;

	// Create Slider function
	void createSlider(Slider &slider, Slider::SliderStyle style, double defaultVal,
		double min, double max, double incr, std::string name);

	// Create Label function
	void createLabel(Label &label, std::string name);

	// Slider variables
	Slider mixSlider;
	Slider pitchSlider;


	// Text Label variables
	Label mixText;
	Label pitchText;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PitchShifterAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED
