#include "Straudio.h"
#include "IPlug_include_in_plug_src.h"
#include "IPlugPaths.h"
#include <windows.h>
#include <stdio.h>

Straudio::Straudio(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    printf("Console attached!\n");
  GetParam(kGain)->InitGain("Gain", -70., -70, 0.);
  
//#ifdef DEBUG
  SetCustomUrlScheme("iplug2");
  SetEnableDevTools(true);
//#endif
  printf("3");
  mEditorInitFunc = [&]() {
//    LoadURL("http://localhost:5173/");
      printf("1");
    LoadIndexHtml(__FILE__, GetBundleID());
    printf(__FILE__);
    EnableScroll(false);
  };
  printf("2");
  MakePreset("One", -70.);
  MakePreset("Two", -30.);
  MakePreset("Three", 0.);
}

void Straudio::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double gain = GetParam(kGain)->DBToAmp();
    
  mOscillator.ProcessBlock(inputs[0], nFrames); // comment for audio in

  for (int s = 0; s < nFrames; s++)
  {
    outputs[0][s] = inputs[0][s] * mGainSmoother.Process(gain);
    outputs[1][s] = outputs[0][s]; // copy left
  }
  
  mSender.ProcessBlock(outputs, nFrames, kCtrlTagMeter);
}

void Straudio::OnReset()
{
  auto sr = GetSampleRate();
  mOscillator.SetSampleRate(sr);
  mGainSmoother.SetSmoothTime(20., sr);
}

void Straudio::OnIdle()
{
//  mSender.TransmitData(*this);
    //const char* mMessage = __FILE__;
    const char* mMessage = GetBundleID();
    Straudio::SendArbitraryMsgFromDelegate(0, strlen(mMessage) + 1, (void*)mMessage);
    
}
