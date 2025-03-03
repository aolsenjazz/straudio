#include "Straudio.h"
#include "IPlug_include_in_plug_src.h"
#include "IPlugPaths.h"
#include <iostream>

#include "civetweb.h"

Straudio::Straudio(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kGain)->InitGain("Gain", -70., -70, 0.);
  
//#ifdef DEBUG
  SetCustomUrlScheme("iplug2");
  SetEnableDevTools(true);
//#endif

  mEditorInitFunc = [&]() {
    LoadURL("http://localhost:5173/");
    LoadIndexHtml(__FILE__, GetBundleID());
    EnableScroll(false);
  };

  MakePreset("One", -70.);
  MakePreset("Two", -30.);
  MakePreset("Three", 0.);
  
//  namespace fs = std::filesystem;
//  
//  fs::path mainPath(__FILE__);
//  fs::path indexRelativePath = mainPath.parent_path() / "resources" / "web";
//
//  std::cout << indexRelativePath.string();
  
  const char* htmlPath = "/Users/alex/workspace/straudio/plugin/Straudio/resources/web/";
  
  // In your plugin constructor
  mg_callbacks callbacks{};
  callbacks.init_context = &Straudio::ServerInitCallback;
//  callbacks.begin_request = &Straudio::BeginRequestHandler;

  const char* options[] = {
      "document_root", htmlPath,
      "listening_ports", "8080",
      "index_files", "index.html",
      NULL
  };
  
  struct mg_context *ctx;

  /* Initialize the library */
//  mg_init_library(0);

  ctx = mg_start(NULL, NULL, options);
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
    // const char* mMessage = GetBundleID();
    // Straudio::SendArbitraryMsgFromDelegate(0, strlen(mMessage) + 1, (void*)mMessage);
    
}

// In your .cpp file
void Straudio::ServerInitCallback(const struct mg_context* ctx) {
    printf("Server initialized");
}

int Straudio::BeginRequestHandler(struct mg_connection* conn) {
  printf("Request handler");
}
