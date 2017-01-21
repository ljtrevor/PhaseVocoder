/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.
	

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
PitchShifterAudioProcessor::PitchShifterAudioProcessor()
{
	// Init our channel data upon startup
	shifter.initArrays(&shifter.monoData);
	shifter.initArrays(&shifter.leftData);
	shifter.initArrays(&shifter.rightData);
}

PitchShifterAudioProcessor::~PitchShifterAudioProcessor()
{
}

//==============================================================================
const String PitchShifterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

int PitchShifterAudioProcessor::getNumParameters()
{
	return 0;
}

float PitchShifterAudioProcessor::getParameter(int index)
{
	return 0.0f;
}

void PitchShifterAudioProcessor::setParameter(int index, float newValue)
{
}

const String PitchShifterAudioProcessor::getParameterName(int index)
{
	return String();
}

const String PitchShifterAudioProcessor::getParameterText(int index)
{
	return String();
}

const String PitchShifterAudioProcessor::getInputChannelName(int channelIndex) const
{
	return String(channelIndex + 1);
}

const String PitchShifterAudioProcessor::getOutputChannelName(int channelIndex) const
{
	return String(channelIndex + 1);
}

bool PitchShifterAudioProcessor::isInputChannelStereoPair(int index) const
{
	return true;
}

bool PitchShifterAudioProcessor::isOutputChannelStereoPair(int index) const
{
	return true;
}


bool PitchShifterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PitchShifterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PitchShifterAudioProcessor::silenceInProducesSilenceOut() const
{
	return false;
}

double PitchShifterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PitchShifterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PitchShifterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PitchShifterAudioProcessor::setCurrentProgram (int index)
{
}

const String PitchShifterAudioProcessor::getProgramName (int index)
{
    return String();
}

void PitchShifterAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//Update Pitch Shifter's Parameters every time knob is turned
void PitchShifterAudioProcessor::updateShifter(void)
{
	//Get Shifter Paramters
	Shifter::Parameters shifterParams = shifter.getParameters();

	//Set Shifter Parameters
	shifterParams.pitch = pitchVal;
	shifterParams.mix = mixVal;

	// Actually set them in parameters struct
	shifter.setParameters(shifterParams);

}

//==============================================================================
void PitchShifterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void PitchShifterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PitchShifterAudioProcessor::setPreferredBusArrangement (bool isInput, int bus, const AudioChannelSet& preferredSet)
{
    // Reject any bus arrangements that are not compatible with your plugin

    const int numChannels = preferredSet.size();

   #if JucePlugin_IsMidiEffect
    if (numChannels != 0)
        return false;
   #elif JucePlugin_IsSynth
    if (isInput || (numChannels != 1 && numChannels != 2))
        return false;
   #else
    if (numChannels != 1 && numChannels != 2)
        return false;

    if (! AudioProcessor::setPreferredBusArrangement (! isInput, bus, preferredSet))
        return false;
   #endif

    return AudioProcessor::setPreferredBusArrangement (isInput, bus, preferredSet);
}
#endif

void PitchShifterAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    //const int totalNumInputChannels  = getTotalNumInputChannels();
    //const int totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...

	//Process Audio
	if (getNumInputChannels() == 1)
	{
		//obtain channel 1 data
		float *monoChannel = buffer.getWritePointer(0);
		//Process mono data
		shifter.processMono(monoChannel, buffer.getNumChannels());
	}
	else if (getNumInputChannels() == 2)
	{
		//Obtain channel 1 and 2 data
		float *leftChannel = buffer.getWritePointer(0), *rightChannel = buffer.getWritePointer(1);
		//Process Data
		shifter.processStereo(leftChannel, rightChannel, buffer.getNumSamples());
	}
   
}

//==============================================================================
bool PitchShifterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* PitchShifterAudioProcessor::createEditor()
{
    return new PitchShifterAudioProcessorEditor (*this);
}

//==============================================================================
void PitchShifterAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void PitchShifterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PitchShifterAudioProcessor();
}
