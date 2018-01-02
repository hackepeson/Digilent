#include <stdbool.h>
#include "sample.h"
#include <iostream>
#include <fstream>


using namespace std;

    HDWF hdwf;
    double rgdSamples[1024];
    int idxChannel = 0;
    char szError[512] = {0};

    int cSamples;
    double ADCSamples[8192];
    STS sts;

void createDACData()
{
    // generate custom samples normalized to +-1
    for(int i = 0; i < 1024; i++)
   {
    //  ramp
//    rgdSamples[i] = 2.0*i/1024;
    //printf("%f\n", rgdSamples[i]);

    //custom

    if (i < 100)
    {
        rgdSamples[i] = 0.0;
    }
    else if (i < 201)
    {
        rgdSamples[i] = 2.0;
    }
    else
    {
        rgdSamples[i] = 0.0;
    }



   }


}

void initADC()
{


    // enable channel
    FDwfAnalogInChannelEnableSet(hdwf, 0, true);

    // set 5V pk2pk input range for all channels
    FDwfAnalogInChannelRangeSet(hdwf, -1, 5);

    // 20MHz sample rate
    FDwfAnalogInFrequencySet(hdwf, 1000000.0);

    // get the maximum buffer size
    FDwfAnalogInBufferSizeInfo(hdwf, NULL, &cSamples);
    FDwfAnalogInBufferSizeSet(hdwf, cSamples);

    // configure trigger
    FDwfAnalogInTriggerSourceSet(hdwf, trigsrcDetectorAnalogIn);
    FDwfAnalogInTriggerAutoTimeoutSet(hdwf, 10.0);
    FDwfAnalogInTriggerChannelSet(hdwf, 0);
    FDwfAnalogInTriggerTypeSet(hdwf, trigtypeEdge);
    FDwfAnalogInTriggerLevelSet(hdwf, 1.0);
    FDwfAnalogInTriggerConditionSet(hdwf, trigcondRisingPositive);

    // wait at least 2 seconds with Analog Discovery for the offset to stabilize,
    // before the first reading after device open or offset/range change
    Wait(2);

    // start
    FDwfAnalogInConfigure(hdwf, 0, true);

}

void initDAC()
{
    // enable first channel
    FDwfAnalogOutNodeEnableSet(hdwf, idxChannel, AnalogOutNodeCarrier, true);
    // set custom function
    FDwfAnalogOutNodeFunctionSet(hdwf, idxChannel, AnalogOutNodeCarrier, funcCustom);
    // set custom waveform samples
    // normalized to ±1 values
    FDwfAnalogOutNodeDataSet(hdwf, idxChannel, AnalogOutNodeCarrier, rgdSamples, 1024);
    // 10kHz waveform frequency
   double freq = 1000;
    FDwfAnalogOutNodeFrequencySet(hdwf, idxChannel, AnalogOutNodeCarrier, freq);
    // 2V amplitude, 4V pk2pk, for sample value -1 will output -2V, for 1 +2V
    FDwfAnalogOutNodeAmplitudeSet(hdwf, idxChannel, AnalogOutNodeCarrier, 5);

   FDwfAnalogOutIdleSet(hdwf, idxChannel, DwfAnalogOutIdleInitial);

  //Wait(1);
   // One shot
   FDwfAnalogOutRepeatTriggerSet(hdwf, idxChannel, 0);
   FDwfAnalogOutRepeatSet(hdwf, 0, 1);
   FDwfAnalogOutRunSet(hdwf, idxChannel, 1/freq);

}

int main(int carg, char **szarg)
{

  printf("Open automatically the first available device\n");
  if(!FDwfDeviceOpen(-1, &hdwf))
  {
    FDwfGetLastErrorMsg(szError);
    printf("Device open failed\n\t%s", szError);
    return 0;
  }



  createDACData();
  initADC();
  initDAC();


  // Start DAC
  FDwfAnalogOutConfigure(hdwf, idxChannel, true);

  // Wait 1 secone
  Wait(1);

  printf("Waiting for triggered or auto acquisition\n");
  do
  {
    FDwfAnalogInStatus(hdwf, true, &sts);
  }while(sts != stsDone);

    // get the samples for each channel
    FDwfAnalogInStatusData(hdwf, 0, ADCSamples, cSamples);




  // Stop DAC
  FDwfAnalogOutConfigure(hdwf, 0, false);
  FDwfAnalogInChannelEnableSet(hdwf, 0, false);

  ofstream outFile;
  outFile.open("adc.txt");

  for (int i = 0; i < 8192; i++)
  {
      outFile << ADCSamples[i];
      outFile << "\n";
  }

  outFile.close();
  // on close device is stopped and configuration lost
  FDwfDeviceCloseAll();
}
