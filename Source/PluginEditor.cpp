/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE Plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//Creates a slider with customized parameters

void PitchShifterAudioProcessorEditor::createSlider(Slider &slider, Slider::SliderStyle style, double defaultVal,
	double min, double max, double incr, std::string name)
{
	// Define Slider parameters
	slider.setSliderStyle(style);
	slider.setRange(min, max, incr);
	slider.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
	slider.setPopupDisplayEnabled(true, this);
	slider.setTextValueSuffix(" " + name);
	slider.setValue(defaultVal);

	// Add a listener (user interaction)
	slider.addListener(this);
	// Add slider to editor
	addAndMakeVisible(&slider);
	// set name
	slider.setComponentID(name);
}

void PitchShifterAudioProcessorEditor::createLabel(Label &label, std::string name)
{
	// Define Label parameters
	label.setSize(50, 20);
	label.setEnabled(true);
	label.setText(name, dontSendNotification);
	addAndMakeVisible(label);
	label.setVisible(true);
	label.isAlwaysOnTop();
}
//==============================================================================
PitchShifterAudioProcessorEditor::PitchShifterAudioProcessorEditor (PitchShifterAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);

	//Create Sliders
	createSlider(mixSlider, Slider::Rotary, processor.mixVal, 0.0, 1.0f, 0.01f, "Mix");
	createSlider(pitchSlider, Slider::Rotary, processor.pitchVal, 0.1, 4.0, 0.1, "Semitone");

	//Create Labels
	createLabel(mixText, "Mix");
	createLabel(pitchText, "Pitch");
}

PitchShifterAudioProcessorEditor::~PitchShifterAudioProcessorEditor()
{
}

//==============================================================================
void PitchShifterAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::lightgreen);

	//Texts and Line Graph
	g.setColour(Colours::black);
	g.setFont(30.f);
	g.drawFittedText("Pitch Shifter", getLocalBounds(), Justification::bottomLeft, 1);
	g.setFont(20.0f);
	g.drawFittedText("by Maitreya and Trevor", getLocalBounds(), Justification::bottomRight, 2);

	//Backgrounds
	g.setColour(Colours::grey);
}

void PitchShifterAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

	//Set Bounds for Sliders
	mixSlider.setBounds(90, 50, 100, 100);
	pitchSlider.setBounds(270, 50, 100, 100);

	//Set bounds for texts
	mixText.setBounds(105, 205, 10, 10);
	pitchText.setBounds(280, 205, 10, 10);
}

void PitchShifterAudioProcessorEditor::sliderValueChanged(Slider* slider)
{
	// Update Parameter Values from Sliders
	if (slider->getComponentID().compare("Mix") == 0) {
		processor.mixVal = slider->getValue();
	}
	else if (slider->getComponentID().compare("Semitone") == 0) {
		processor.pitchVal = slider->getValue();
	}

	// Update the Pitch Shifter Parameters
	processor.updateShifter();
}