
// Phase Vocoding implementation via filter-bank approach
// Information obtained from DAFX book ch. 7
// Pitch Shifting Process:
//      1. Input Signal (Time Domain)
//      2. Window Data
//      3. STFT Signal (to FFT data)
//      4. Convert to Magnitude and Phase form
//      5. Multiply phases/frequencies with pitch parameter
//      6. Convert from Magnitude and Phase form
//      7. IFFT Signal (to Time Domain)
//      8. Overlap Add Signals
//
// Using the fft.c/h written by:
//          Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
// 
// fft.c/h makes a window for the input signal and converts signals to and from FFT with rfft function
//
#define _USE_MATH_DEFINES
#include <cmath>
#include "Shifter.h"

# pragma mark - Initialization and Constructors -

// Constructor
// Set Initial Parameters and sample rate (44100)
Shifter::Shifter() : currentSampleRate(INIT_SAMPLE_RATE)
{
    setParameters(Parameters());
    setSampleRate(INIT_SAMPLE_RATE);
}

// Deconstructor
Shifter::~Shifter()
{
}

// Set our new parameter settings
void Shifter::setParameters(const Parameters& newParams)
{
    parameters = newParams;
}

// Set new sample rate
void Shifter::setSampleRate(const double sampleRate)
{
    jassert(sampleRate > 0);
    currentSampleRate = sampleRate;
}

// Clear/initialize our buffers
void Shifter::prepareToPlay()
{
    
}

// Initialize our Arrays upon startup
void Shifter::initArrays(data *dat)
{
    // Set FFT Oversampling Factor - determines the overlap between adjacent STFT frames
    osamp = WINDOW_SIZE/HOP_SIZE;
    // Set Overlap Percentage - # of samples that will overlap
    overlap = (WINDOW_SIZE - HOP_SIZE) / (float)WINDOW_SIZE;
    // Set Overlap Samples - how much overlap there will be between frames
    overlap_samples = overlap * WINDOW_SIZE;
    
    // Apply hanning window to our main window (less overlap)
    hanning(dat->win, WINDOW_SIZE);
    // Zero out previous window
    memset(dat->pre_win, 0, WINDOW_SIZE*sizeof(float));
    
    // Set expected omega frequency value
    omega = 2. * M_PI * osamp * (float)HOP_SIZE / (float)WINDOW_SIZE;

    // Scale window for overlap add
    for (int i = 0; i < WINDOW_SIZE; i++) dat->win[i] *= 2. / osamp;
    
    // Reset phase buffers
    setBuffers(dat);
    dat->status = false;
}

// Reset phase buffers
void Shifter::setBuffers(data *dat)
{
    // Zero out buffers
    memset(dat->prev_phs, 0, WINDOW_SIZE/2*sizeof(float));
    memset(dat->sumPhase, 0, WINDOW_SIZE/2*sizeof(float));
}

# pragma mark - Pitch Shifting Algorithm -

# pragma mark - Mono Channel Processing
void Shifter::processMono(float* const samples, const int numSamples)
{
    processChannel(samples, numSamples, &monoData);
}

# pragma mark - Stereo Channel Processing

void Shifter::processStereo(float* const left, float* const right, const int numSamples)
{
    processChannel(left, numSamples, &leftData);
    processChannel(right, numSamples, &rightData);
}

// Process Channel Data
inline void Shifter::processChannel(float* const samples, const int numSamples, data *myData) noexcept
{
    // Assert that the samples are not null
    jassert (samples != nullptr);
    
    // Init vars
    long i, j, index;
    float tmp;
    
    // Set Frequencies Per Bin - frequency width per bin = SR/WINDOW_SIZE (bandwidth per bin for each window size)
    freqPerBin  = currentSampleRate/(float)WINDOW_SIZE;
    
    // Init our arrays upon start-up
    if (myData->status == false)
    {
        setBuffers(myData);
        myData->status = true;
    }
    
    // Return filtered data if parameters.pitch is just 1.0
    if (parameters.pitch == 1.0f)
    {
        if (parameters.filter) processFilters(samples, numSamples);
        return;
    }
    
    // Process our samples; Increment frames by "hop size", or frame widths.
    // This is our overlap add implementation where we take our data frame by frame
    // and then add it back together in the end.
    for (i = 0; i < numSamples; i += HOP_SIZE)
    {
# pragma mark - Analysis
        // Set our incoming samples to the current stft window
        for (j = 0; j < WINDOW_SIZE; j++) myData->cur_win[j] = samples[i+j];
        // Applies windowing to data
        apply_window(myData->cur_win, myData->win, WINDOW_SIZE);
        
        // Obtain minimum phase by shifting time domain data before taking FFT
        fftshift(myData->cur_win, WINDOW_SIZE);
        
# pragma mark - FFT/Convert to Magnitudes + Phases
        // FFT real values (Convert to frequency domain)
        rfft(myData->cur_win, WINDOW_SIZE/2, FFT_FORWARD);
        // Get real and imaginary #s of the FFT'd window
        complex *cbuf = (complex *)myData->cur_win;
        
        // Get Magnitude and Phase (polar coordinates)
        for (j = 0; j < WINDOW_SIZE/2; j++)
        {
            myData->cur_mag[j] = cmp_abs(cbuf[j]);
            myData->cur_phs[j] = atan2f(cbuf[j].im, cbuf[j].re);
        }

# pragma mark - Time-Frequency Processing
        // Get frequencies of FFT'd signal (analysis stage)
        for (j = 0; j < WINDOW_SIZE/2; j++)
        {
            // Get phase shift (true frequency w/ unwrapped phase shift)
            tmp = myData->cur_phs[j] - myData->prev_phs[j];
            // Set prev_phase to cur_phase
            myData->prev_phs[j] = myData->cur_phs[j];
            
            // Get Frequency Deviation (convert to radians)
            tmp -= j*omega;
            
            // Wrap Frequency Deviation to +/- Pi interval
            tmp = fmod(tmp + M_PI, -2 * M_PI) + M_PI;
            
            // Get deviation from bin freq from the +/- pi interval (convert from radians)
            tmp = osamp * tmp / (2. * M_PI);
            
            // Compute true frequency (new phase of freq bin j) by adding phase shift
            tmp = (long)j * freqPerBin + tmp * freqPerBin;
            
            // Store true frequency
            myData->anaFreq[j] = tmp;
        }
        
        // Zero our processing buffers
        memset(myData->synMagn, 0, WINDOW_SIZE*sizeof(float));
        memset(myData->synFreq, 0, WINDOW_SIZE*sizeof(float));
        // Set new frequencies according to our pitch value
        // Filter-bank implementation
        // - Represents Frequencies as a sum on sinusoids
        // - Sinuosoids are modulated in amplitude/frequency
        for (j = 0; j < WINDOW_SIZE/2; j++)
        {
            // Get phase index to pitch shift our FFT data
            index = j * parameters.pitch;
            
            // if the phase index is within our FFT data range, we
            // overlap our current magnitude and set our analysis
            // frequency
            if (index < WINDOW_SIZE/2)
            {
                myData->synMagn[index] += myData->cur_mag[j];
                myData->synFreq[index] = myData->anaFreq[j];
            }
        }
        
# pragma mark - Synthesis
        // Write our new magnitudes and phases
        for (j = 0; j < WINDOW_SIZE/2; j++) {
            // get magnitude and true frequency from synthesis arrays
            myData->cur_mag[j] = myData->synMagn[j];
            
            // Subtract mid frequency bin (get actual pitch-shifted frequency from position j bin)
            tmp = myData->synFreq[j] - (float)j * freqPerBin;
            
            // Get bin deviation from frequency deviation (get actual pitch shifted frequency from frequency bandwidth)
            tmp /= freqPerBin;
            
            // Factor in overlap factor
            tmp = 2. * M_PI * tmp / (float)osamp;
            
            // Add the overlap phase back in (convert to radians)
            tmp += j*omega;
            
            // Accumulate delta phase to get bin phase
            myData->sumPhase[j] += tmp;
            myData->cur_phs[j] = myData->sumPhase[j];
        }
        
        // Back to Cartesian coordinates
        for (j = 0; j < WINDOW_SIZE/2; j++) {
            cbuf[j].re = myData->cur_mag[j] * cosf(myData->cur_phs[j]);
            cbuf[j].im = myData->cur_mag[j] * sinf(myData->cur_phs[j]);
        }
        
        // IFFT back to time domain signal
        rfft((float*)cbuf, WINDOW_SIZE/2, FFT_INVERSE);
 
# pragma mark - Output
        // Write to output (of hop size frame)
        for (j = 0; j < HOP_SIZE; j++) myData->outData[i+j] = myData->pre_win[j + HOP_SIZE] + myData->cur_win[j];
    
        // Move previous window (move previous hop-size frame to previous 256 frames)
        for (j = 0; j < WINDOW_SIZE; j++) myData->pre_win[j] = (j < overlap_samples) ? myData->pre_win[j + HOP_SIZE] : 0;
        
        // Update previous window (add current window to current previous win)
        for (j = 0; j < WINDOW_SIZE; j++) myData->pre_win[j] += myData->cur_win[j];
    }
    
    // Filter data if filter button is on
    if (parameters.filter) processFilters(myData->outData, numSamples);
        
    // Combine input data with output data
    for (i = 0; i < numSamples; i++) samples[i] = samples[i] * (1.0 - parameters.mix) + myData->outData[i] * parameters.mix;
}

# pragma mark - Filter Processing

// Updates Lowpass Filter's Parameters
void Shifter::updateLPFilter(void)
{
    IIRCoefficients low_coef = IIRCoefficients::makeLowPass(currentSampleRate, parameters.lpf);
    lpassFilter.setCoefficients(low_coef);
}

// Updates Highpass Filter's Parameters
void Shifter::updateHPFilter(void)
{
    IIRCoefficients high_coef = IIRCoefficients::makeLowPass(currentSampleRate, parameters.hpf);
    hpassFilter.setCoefficients(high_coef);
}

// Apply filtering to our input data
void Shifter::processFilters(float* const samples, const int numSamples)
{
    // If LPF->HPF order selected
    if (!parameters.order)
    {
        lpassFilter.processSamples(samples, numSamples);
        hpassFilter.processSamples(samples, numSamples);
    }
    else
    {
        hpassFilter.processSamples(samples, numSamples);
        lpassFilter.processSamples(samples, numSamples);
    }
}