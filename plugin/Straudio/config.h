#define PLUG_NAME "Straudio"
#define PLUG_MFR "AOFX"
#define PLUG_VERSION_HEX 0x00000100
#define PLUG_VERSION_STR "0.0.1"
#define PLUG_UNIQUE_ID '1337'
#define PLUG_MFR_ID 'AOFX'
#define PLUG_URL_STR "https://www.github.com/aolsenjazz"
#define PLUG_EMAIL_STR "aolsenjazz@gmail.com"
#define PLUG_COPYRIGHT_STR "Copyright 2024 AOFX Inc"
#define PLUG_CLASS_NAME Straudio

#define BUNDLE_NAME "Straudio"
#define BUNDLE_MFR "AOFX"
#define BUNDLE_DOMAIN "com"

#define SHARED_RESOURCES_SUBPATH "Straudio"

#ifdef APP_API
#define PLUG_CHANNEL_IO "1-2"
#else
#define PLUG_CHANNEL_IO "1-1 1-2 2-2"
#endif

#define PLUG_LATENCY 0
#define PLUG_TYPE 0
#define PLUG_DOES_MIDI_IN 0
#define PLUG_DOES_MIDI_OUT 0
#define PLUG_DOES_MPE 0
#define PLUG_DOES_STATE_CHUNKS 0
#define PLUG_HAS_UI 1
#define PLUG_WIDTH 459
#define PLUG_HEIGHT 644
#define PLUG_FPS 60
#define PLUG_SHARED_RESOURCES 0
#define PLUG_HOST_RESIZE 1
#define PLUG_MIN_WIDTH 459
#define PLUG_MIN_HEIGHT 644
#define PLUG_MAX_WIDTH 459
#define PLUG_MAX_HEIGHT 644

#define AUV2_ENTRY Straudio_Entry
#define AUV2_ENTRY_STR "Straudio_Entry"
#define AUV2_FACTORY Straudio_Factory
#define AUV2_VIEW_CLASS Straudio_View
#define AUV2_VIEW_CLASS_STR "Straudio_View"

#define AAX_TYPE_IDS '1337'
#define AAX_TYPE_IDS_AUDIOSUITE '1337'
#define AAX_PLUG_MFR_STR "AOFX"
#define AAX_PLUG_NAME_STR "Straudio\nIPEF"
#define AAX_PLUG_CATEGORY_STR "Effect"
#define AAX_DOES_AUDIOSUITE 1

#define VST3_SUBCATEGORY "Fx"

#define APP_NUM_CHANNELS 2
#define APP_N_VECTOR_WAIT 0
#define APP_MULT 1
#define APP_COPY_AUV3 0
#define APP_SIGNAL_VECTOR_SIZE 64

#define ROBOTO_FN "Roboto-Regular.ttf"
