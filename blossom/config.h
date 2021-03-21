#pragma once

// like Leviathan, we have some risky tricks to shave the last few bytes
#define DESPERATE 0

// releasing at Revision? here's a handy toggle for some compo-safe config presets
#define REVISION_RULESET 0


#if _DEBUG
	#define WINDOW_FULLSCREEN 0
	#define WINDOW_AUTO_SIZE 0
	#define CANVAS_WIDTH 1280
	#define CANVAS_HEIGHT 720

	#define RENDER_MAX_TIME_MS 30000
	//#define RENDER_MIN_SAMPLES 1
	//#define RENDER_MAX_SAMPLES 256

	#define RENDER_PROGRESSIVE 1
#endif

#if RELEASE
	// party release config, based on Revision ruleset
	#if REVISION_RULESET
		#define WINDOW_FULLSCREEN 1
		#define WINDOW_AUTO_SIZE 0
		#define CANVAS_WIDTH  1920
		#define CANVAS_HEIGHT 1080

		#define RENDER_MAX_TIME_MS 30000 // Revision Rules: don't exceed 30 seconds

		#define RENDER_MAX_SAMPLES 256 // customizable, optional

		#define RENDER_PROGRESSIVE 0 // Revision Rules: no progressive rendering.
	#else
		#define WINDOW_FULLSCREEN 1
		#define WINDOW_AUTO_SIZE 1

		#define RENDER_EXACT_SAMPLES 256 // without the constraints of party rules, we can set an exact quality bar, if we want.

		#define RENDER_PROGRESSIVE 0
	#endif
#endif

#if CAPTURE
	#define WINDOW_AUTO_SIZE 0
	#define CANVAS_WIDTH 3840
	#define CANVAS_HEIGHT 2160

	#define RENDER_EXACT_SAMPLES 1024

	// which formats to save
	#define CAPTURE_SAVE_PNG 1
	#define CAPTURE_SAVE_JPG 0
	#define CAPTURE_SAVE_U8_BIN 0
	#define CAPTURE_SAVE_F32_BIN 0
#endif



// ==== housekeeping stuff, you don't need to touch this ==== //

#if CAPTURE
	#define WINDOW_FULLSCREEN 0

	#define RENDER_PROGRESSIVE 1

	#if !CAPTURE_SAVE_PNG && !CAPTURE_SAVE_JPG && !CAPTURE_SAVE_U8_BIN && !CAPTURE_SAVE_F32_BIN
		#define CAPTURE_SAVE_PNG 1
	#endif
#endif
