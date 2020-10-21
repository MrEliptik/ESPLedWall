#include "arduinoFFT.h"

#define SAMPLING_FREQUENCY 10000

arduinoFFT FFT = arduinoFFT(); /* Create FFT object */

const int NB_SAMPLES = 128;
int val;

double vReal[NB_SAMPLES];
double vImag[NB_SAMPLES];

unsigned int sampling_period_us;
unsigned long newTime, oldTime;

void setup() {
  analogReference(DEFAULT); // Use the default (5v) reference
  pinMode(A0, INPUT);
  Serial.begin(9600);

  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
}

void loop() {
  // put your main code here, to run repeatedly:
  int min=1024, max=0; // min and max adc value

  // Take 128 samples
  for (int i = 0; i < NB_SAMPLES; i++) {
    newTime = micros()-oldTime;
    oldTime = newTime;
    val = analogRead(A0);
    vReal[i] = val;
    vImag[i] = 0;
    // Capture min and max val
    if(val>max) max = val;
    if(val<min) min = val;

    while (micros() < (newTime + sampling_period_us)) { /* do nothing to wait */ }
  }

  FFT.Windowing(vReal, NB_SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, NB_SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, NB_SAMPLES);

  for(int i = 0; i < NB_SAMPLES; i++){ 
    //Serial.println(vReal[i]);
  }

  for (int i = 2; i < (NB_SAMPLES/2); i++){ // Don't use sample 0 and only first SAMPLES/2 are usable. Each array eleement represents a frequency and its value the amplitude.
    
    if (vReal[i] > 100) { // Add a crude noise filter, 4 x amplitude or more
      if (i<=5 ){
        Serial.print((int)vReal[i]); //displayBand(0,(int)vReal[i]/amplitude); // 125Hz
        Serial.print("  ");
      }
      if (i >5   && i<=12 ){
        Serial.print((int)vReal[i]);  //displayBand(1,(int)vReal[i]/amplitude); // 250Hz
        Serial.print("  ");
      }
      if (i >12  && i<=32 ){
        Serial.print((int)vReal[i]);  //displayBand(2,(int)vReal[i]/amplitude); // 500Hz
        Serial.print("  ");
      }
      if (i >32  && i<=62 ){
        Serial.print((int)vReal[i]);  //displayBand(3,(int)vReal[i]/amplitude); // 1000Hz
        Serial.print("  ");
      }
    }
    //for (byte band = 0; band <= 6; band++) display.drawHorizontalLine(18*band,64-peak[band],14);
  }
  Serial.println("");

}
