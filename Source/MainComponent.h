// Music 256a / HW2 /Synth of your Choice
// CCRMA, Stanford University
//
// Author: Walker Davis with lots of help and Faust code from Romain Michon and Tim O'Brien
// Description: Saw Waves with Circles

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "Phat.h" //Phat Phaust stacked triangular waves



class OtherLookAndFeel :
    public LookAndFeel_V3 //my Beautiful Inspiring GUI
{
public:
    OtherLookAndFeel()
    {
        setColour (Slider::rotarySliderFillColourId, Colours::red);
    }
    
    //this class makes the rotary sliders more interesting
    //i like these colors becuase they are pretty childish, sort of like a phat mono synth

    void drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                           const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider) override
    {
        
        
        const float radius = jmin (width / 2, height / 2) - 10.0f;
        const float centreX = x + width * 0.5f;
        const float centreY = y + height * 0.5f;
        const float rx = centreX - radius;
        const float ry = centreY - radius;
        const float rw = radius * 2.0f;
        const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        
        // fill
        g.setColour (Colours::orange);
        g.fillEllipse (rx, ry, rw, rw);
        // outline
        g.setColour (Colours::red);
        g.drawEllipse (rx, ry, rw, rw, 15.0f);
        
        Path p;
        const float pointerLength = radius * 0.75f;
        const float pointerThickness = 5.0f;
        p.addRectangle (-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        p.applyTransform (AffineTransform::rotation (angle).translated (centreX, centreY));
        
        // pointer
        g.setColour (Colours::yellow);
        g.fillPath (p);
        
    }
};

class MainContentComponent :
        public AudioAppComponent,
        private Slider::Listener
{
public:
    //========================================================
    MainContentComponent() : currentSampleRate(0.0)
    {//all of my dials for the synth and resonant lowpass filter
        addAndMakeVisible (dial1); //frequency
        dial1.setSliderStyle (Slider::Rotary);
        dial1.setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
        dial1.setLookAndFeel (&otherLookAndFeel);
        dial1.setRange (32.70, 523.25);
        //dial1.setSkewFactorFromMidPoint (500.0); // [4]
        dial1.setValue(32.0);
        dial1.addListener (this);
        
        
        addAndMakeVisible (dial2); //cutoff frequency of filter
        dial2.setSliderStyle (Slider::Rotary);
        dial2.setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
        dial2.setLookAndFeel (&otherLookAndFeel);
        dial2.setRange (33.0, 13000.0);
        //dial2.setSkewFactorFromMidPoint (500.0); // [4]
        dial2.setValue(33.0);
        dial2.addListener (this);
        
        addAndMakeVisible (dial3); //resonance of filter
        dial3.setSliderStyle (Slider::Rotary);
        dial3.setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
        dial3.setLookAndFeel (&otherLookAndFeel);
        dial3.setRange (1.0, 40.0);
        //dial3.setSkewFactorFromMidPoint (500.0); // [4]
        dial3.setValue(1.0);
        dial3.addListener (this);
        
        addAndMakeVisible (dial4); //gain
        dial4.setSliderStyle (Slider::Rotary);
        dial4.setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
        dial4.setLookAndFeel (&otherLookAndFeel);
        dial4.setRange (0.00, 0.50);
        //dial4.setSkewFactorFromMidPoint (500.0); // [4]
        dial4.setValue(0.0);
        dial4.addListener (this);
        
        setSize (600, 200);
        
        nChans = 2; // number of output audio channels
        
        setAudioChannels (0, nChans);
        
        audioBuffer = new float*[nChans];
        
        
    }
    
    ~MainContentComponent()
    {
        shutdownAudio();
        delete [] audioBuffer;
    }
//=====================================
    
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        currentSampleRate = sampleRate;
        blockSize = samplesPerBlockExpected;
        
        phat.init(sampleRate);
        phat.buildUserInterface(&phatControl); //
        
        
        for(int i=0; i<phatControl.getParamsCount(); i++){
            std::cout << phatControl.getParamAdress(i) << "\n";
        }
        
        
        
        //for this project, i did not label the "group" within my faust process line
        //Romain said it was ok to not worry about it for this time around
        phatControl.setParamValue("/phat/freq",32.70);
        phatControl.setParamValue("/phat/cutoff",20.0);
        phatControl.setParamValue("/phat/reso",1.0);
        phatControl.setParamValue("/phat/gain",0.0);
    }
    
    
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        audioBuffer[0] = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
        audioBuffer[1] = bufferToFill.buffer->getWritePointer (1, bufferToFill.startSample);
        
        
        phat.compute(blockSize, NULL, audioBuffer); // computing one block with Faust
        
        
    }
    
    void releaseResources() override
    {
        
    
    
//======================================
    void paint (Graphics& g) override
    {
        g.fillAll (Colours::lightblue);
    }
    
    void resized() override
    {
        const int border = 4;
        
        
        Rectangle<int> area = getLocalBounds();
        
        {
            Rectangle<int> dialArea = area.removeFromTop (area.getHeight() );
            dial1.setBounds (dialArea.removeFromLeft (dialArea.getWidth() / 3).reduced (border));
            dial2.setBounds (dialArea.removeFromLeft (dialArea.getWidth() / 3).reduced (border));
            dial3.setBounds (dialArea.removeFromLeft (dialArea.getWidth() / 3).reduced (border));
            dial4.setBounds (dialArea.removeFromLeft (dialArea.getWidth() / 3).reduced (border));
        }
    }

    void sliderValueChanged (Slider* slider) override
    {
        if (currentSampleRate > 0.0){
            if (slider == &dial1)
            {
                phatControl.setParamValue("/phat/freq", dial1.getValue());
            }
            else if (slider == &dial2)
            {
                phatControl.setParamValue("/phat/cutoff", dial2.getValue());
            }
            else if (slider == &dial3)
            {
                phatControl.setParamValue("/phat/reso", dial3.getValue());
            }
            else if (slider == &dial4)
            {
                phatControl.setParamValue("/phat/gain", dial4.getValue());
            }
        }
    }
    
private:
    Phat phat;
    MapUI phatControl;
    
    OtherLookAndFeel otherLookAndFeel; // [2]
    
    Slider dial1;//pitch frequency
    Slider dial2;//cutoff frequency
    Slider dial3;//resonance
    Slider dial4;//gain

    int blockSize, nChans;
    double currentSampleRate;
    
    float** audioBuffer; // multichannel audio buffer used both for input and output
    
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};

// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()     { return new MainContentComponent(); }


#endif  // MAINCOMPONENT_H_INCLUDED
