//
//  Shifter.h
//  PhaseShifter Header
//
//
//

#ifndef __fooHarmonizer__Shifter__
#define __fooHarmonizer__Shifter__

// #include <stdio.h>
#include "../JuceLibraryCode/JuceHeader.h"
#include "fft.h"

#define INIT_SAMPLE_RATE        44100
#define WINDOW_SIZE             1024 //Was 4096
#define HOP_SIZE                (WINDOW_SIZE/2) //Was 1/4Window Size

class Shifter
{
public:
    // Constructor
    Shifter();
    // Deconstructor
    ~Shifter();

    // Data structure for our channel data analysis
    typedef struct
    {
        // Windows for our data
        float win[WINDOW_SIZE];                 // Window containing windowing type
        float cur_win[WINDOW_SIZE];             // Window containing our current data samples
        float pre_win[WINDOW_SIZE];             // Window containing previous frame's data
        // Previous Phase data
        float prev_phs[WINDOW_SIZE/2];               // Previous phase array
        // FFT Data (Phase and Mag data we obtain)
        float cur_phs[WINDOW_SIZE/2];           // Current phase array
        float cur_mag[WINDOW_SIZE/2];           // Current magnitude array
        // Cumulative Phase data
        float sumPhase[WINDOW_SIZE/2];          // Cumulative phase array
        // Temporary array to write our pitch shifted data to
        float outData[WINDOW_SIZE*2];           // Output Data array
        // Analysis Stage
        float anaFreq[WINDOW_SIZE];             // Analysis frequency
        float anaMagn[WINDOW_SIZE];             // Analysis magnitude
        // Synthesis Stage
        float synFreq[WINDOW_SIZE];             // Synthesis frequency
        float synMagn[WINDOW_SIZE];             // Synthesis magnitude
        // Status flag to check whether or not we have init the data
        bool status;                            // set to false
    } data;
    
    // Data structure holds pitch shifter's parameters
    struct Parameters
    {
		Parameters() noexcept
			: pitch(1.0f),
			lpf(20000.0f),
			hpf(0.0f),
			mix(0.50f),
			order(0),
			filter(1)
			{}

		float pitch;
        float lpf;
        float hpf;
        float mix;
        int order;
        int filter;
    };
    
    // Get parameters
    const Parameters& getParameters() const noexcept
    {
        return parameters;
    };
    
    // Set parameters
    void setParameters(const Parameters& newParams);
    
    // Set sample rate
    void setSampleRate(const double sampleRate);
    
    // Clear buffers
    void prepareToPlay();
    
    // Init arrays
    void initArrays(data *dat);
    // Reset buffers every time we change
    void setBuffers(data *dat);
    
    // Process Mono Channel
    void processMono(float* const samples, const int numSamples);
    
    // Process Stereo Channel
    void processStereo(float* const left, float* const right, const int numSamples);
    
    // Process Channel of Data
    void processChannel(float* const samples, const int numSamples, data *myData) noexcept;
    
    // Update Lowpass Filter's Parameters
    void updateLPFilter(void);
    
    // Updates Highpass Filter's Parameters
    void updateHPFilter(void);
    
    // Process Filters
    void processFilters(float* const samples, const int numSamples);
    
    // Variables for processing FFT windows
    float magn, freqPerBin, expct, overlap, overlap_samples, omega;
    
    // Oversampling factor
    long osamp;
    
    // Our separate data channels
    data monoData, leftData, rightData;
    
protected:
    
private:
    // Pitch shifter's parameters
    Parameters parameters;
    // Pitch shifter's sample rate
    double currentSampleRate;
    // Filters
    IIRFilter lpassFilter, hpassFilter;
};


#endif /* defined(__fooHarmonizer__Shifter__) */
