#include "arduinoFFT.h"

#include "FastLED.h"

#define NUM_STRIPS 7
#define NUM_LEDS_PER_STRIP 30

#define SAMPLING_FREQUENCY 40000 

#define MAX_ADC_VAL 4096.0

// Must use GPIO number
#define BAND0_PIN 16
#define BAND1_PIN 17
#define BAND2_PIN 18
#define BAND3_PIN 21
#define BAND4_PIN 22
#define BAND5_PIN 23
#define BAND6_PIN 25

#define MIC_PIN 32

#define SAMPLING_TIME_US 25


arduinoFFT FFT = arduinoFFT(); /* Create FFT object */

CRGB leds[NUM_STRIPS][NUM_LEDS_PER_STRIP];

const int NB_SAMPLES = 512;
int _val;
int _min=MAX_ADC_VAL, _max=0;

double vReal[NB_SAMPLES];
double vImag[NB_SAMPLES];

unsigned int sampling_period_us;
unsigned long newTime, oldTime = 0;

double band_values[NUM_STRIPS];
double band_max = 0.0;

void setup() {
  pinMode(MIC_PIN, INPUT);
  Serial.begin(9600);

  delay(1000);

  FastLED.addLeds<NEOPIXEL, BAND0_PIN>(leds[0], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<NEOPIXEL, BAND1_PIN>(leds[1], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<NEOPIXEL, BAND2_PIN>(leds[2], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<NEOPIXEL, BAND3_PIN>(leds[3], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<NEOPIXEL, BAND4_PIN>(leds[4], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<NEOPIXEL, BAND5_PIN>(leds[5], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<NEOPIXEL, BAND6_PIN>(leds[6], NUM_LEDS_PER_STRIP);
  
  sampling_period_us = round(1000000  * (1.0 / SAMPLING_FREQUENCY));
  Serial.println(sampling_period_us);
}

void loop() {
  // put your main code here, to run repeatedly:
  _min = MAX_ADC_VAL;
  _max = 0; // min and max adc value

  Serial.println("Taking samples..");

  // Take N samples
  for (int i = 0; i < NB_SAMPLES; i++) {
    newTime = micros()-oldTime;
    oldTime = newTime;
    _val = analogRead(MIC_PIN);
    vReal[i] = _val;
    vImag[i] = 0;
    // Capture min and max val
    if(_val>_max) _max = _val;
    if(_val<_min) _min = _val;

    while (micros() < (newTime + SAMPLING_TIME_US)) { 
      /*
      Serial.println(SAMPLING_TIME_US);
      Serial.print("Waiting..: ");
      Serial.print(newTime+SAMPLING_TIME_US);
       Serial.print("   ");
      Serial.println(micros());
      */
    }
  }

  Serial.println("Computing FFT..");
  FFT.Windowing(vReal, NB_SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, NB_SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, NB_SAMPLES);

  /*
  Serial.print("Max: ");
  Serial.print(max);
  Serial.print("  MIN: ");
  Serial.println(min);
  */

  for(int b = 0; b < NUM_STRIPS; b++) {
    band_values[b] = 0.0;
  }
  band_max = 0;
  for (int i = 2; i < (NB_SAMPLES/2); i++){ // Don't use sample 0 and only first SAMPLES/2 are usable. Each array eleement represents a frequency and its value the amplitude.
    if (vReal[i] > 400) { // Add a crude noise filter, 4 x amplitude or more
      if (i<=2 ){ // 125Hz  
        // Take average value for that band
        band_values[0] += vReal[i];
        band_values[0] /= 2.0;
        if(band_values[0] > band_max) band_max = band_values[0];
      }
      else if (i >3 && i<=5 ) { // 250Hz
        band_values[1] += vReal[i];
        band_values[1] /= 2.0;
        if(band_values[1] > band_max) band_max = band_values[1];
      }
      else if (i >5 && i<=7 ){ // 500Hz
        band_values[2] += vReal[i];
        band_values[2] /= 2.0;
        if(band_values[2] > band_max) band_max = band_values[2];
      }
      else if (i >7 && i<=15 ) { // 1000Hz
        band_values[3] += vReal[i];
        band_values[3] /= 2.0;
        if(band_values[3] > band_max) band_max = band_values[3];
      }
     else  if (i >15 && i<=30 ) { // 2000Hz
        band_values[4] += vReal[i];
        band_values[4] /= 2.0;
        if(band_values[4] > band_max) band_max = band_values[4];
      }
      else if (i >30 && i<=53 ) { // 4000Hz
        band_values[5] += vReal[i];
        band_values[5] /= 2.0;
        if(band_values[5] > band_max) band_max = band_values[5];
      }
      else if (i >53) { // 8000Hz +
        band_values[6] += vReal[i];
        band_values[6] /= 2.0;
        if(band_values[6] > band_max) band_max = band_values[6];
      }
      /*
      else if (i >200) { // 16000Hz
        band_values[0] += vReal[i];
        band_values[0] /= 2;
      }
      */
    }
  }

  Serial.println("Displaying bands..");
  for(int i = 0; i < 7; i++) {
    displayBand(i, band_values[i]);
  }
  FastLED.show();
  // Clear all LEDs for next time
  //clearBands();
}

void displayBand(int band, float value) {
    // Make sure the value is between 0 and NUM_LEDS_PER_STRIP
    int led_val = (int)(value * (float)NUM_LEDS_PER_STRIP / band_max);
    //Serial.println(led_val);
    for(int j = 0; j < led_val; j++) leds[band][j] = CRGB::Green;
    for(int j = led_val; j < NUM_LEDS_PER_STRIP; j++) leds[band][j] = CRGB::Black;
}

void clearBands() {
  for(int i = 0; i < NUM_STRIPS; i++){
    // clear this led for the next time around the loop
    for(int j = 0; j < NUM_LEDS_PER_STRIP; j++) leds[i][j] = CRGB::Black;    
  }
 
}
