# Blossom 🌸

Blossom is a small framework for creating 4K Executable Graphics artworks for the [demoscene](https://en.wikipedia.org/wiki/Demoscene).

You are free to use this as the basis for your own demoscene works; credit ("Blossom by yx") is welcome but not required.

![](docs/screenshot.jpg)

## Rendering Model
Blossom is built primarily with accumulative pathtracing in mind. There are two shaders, `draw` and `present`. The `draw` shader will be executed repeatedly up to a configurable sample-count or render duration, and each sample will be _additively_ blended into a framebuffer. Once the samples are all accumulated, the `present` shader will be executed repeatedly until the user exits the program. Typically the `present` shader will sample the result of the accumulated `draw` passes, and perform some sort of post-processing, tonemapping, etc. for display.

In the example shaders provided, the alpha channel of the `draw` buffer is used to keep track of the sample count (by writing an additive `1.0` for each sample).

## Configurations

### Debug
Does what it says on the tin - a debug-friendly mode, no executable compression (although shaders are still minified). By default this will open a smaller 1280x720 window, and a progressive rendering preview will be enabled. A debug console window is also available.

### Release
This builds the executable with Crinkler enabled to shrink it as small as possible.

### Capture
No minification, but the executable will save the rendered image out to a file, and then exit.

## Shader Uniforms

### vec4 iResolution
The target resolution, available in both shaders.  
`x` and `y` components are the width and height respectively.  
`z` and `w` contain the aspect ratio (`width/height`) and inverse aspect ratio (`height/width`) respectively.

### int iFrame
The current sample index, only available in the `draw` shader.

### sampler2D accumulatorTex
The accumulation framebuffer, only available in the `present` shader.

## Custom Options
In `config.h`, there are a number of customization options for you to set how you see fit:

### WINDOW\_FULLSCREEN
If enabled, Blossom opens in exclusive-fullscreen, otherwise it opens a borderless window.

### WINDOW\_AUTO\_SIZE
If enabled, Blossom will target the primary display's native resolution. Otherwise, the target resolution must be set explicitly with `CANVAS_WIDTH` and `CANVAS_HEIGHT`.

_Note: If you enable `WINDOW_AUTO_SIZE` but your artwork needs to be presented in a certain aspect ratio, it's up to you to handle appropriate letterboxing/pillarboxing manually in your shader code - Blossom will not handle that for you._

### CANVAS\_WIDTH, CANVAS\_HEIGHT
Specify the target resolution in pixels. Only used if `WINDOW_AUTO_SIZE` is disabled.

### RENDER\_MAX\_TIME\_MS
Specify the maximum rendering time in milliseconds. Recommended for compo use.

### RENDER\_MIN\_SAMPLES
Optional. Specify the minimum number of samples to accumulate.

### RENDER\_MAX\_SAMPLES
Optional. Specify the maximum number of samples to accumulate. Recommended for compo use, if you don't need a high sample count to converge your artwork.

### RENDER\_EXACT\_SAMPLES
Specify an exact number of samples to accumulate. If defined, the values of `RENDER_MAX_TIME_MS`, `RENDER_MIN_SAMPLES`, and `RENDER_MAX_SAMPLES` will all be ignored.

### RENDER\_PROGRESSIVE
If enabled, the window will alternate between `draw` and `present` passes, so the current state of the rendering is always visible. If disabled, the window will appear black until the accumulation phase is complete, as required by Revision rules.

### CAPTURE\_SAVE\_PNG
In Capture mode, the render will be saved as a PNG.

### CAPTURE\_SAVE\_JPG
In Capture mode, the render will be saved as a JPG.

### CAPTURE\_SAVE\_U8\_BIN
In Capture mode, the render will be saved as a binary file containing 8-bit-per-channel RGB triplets.

### CAPTURE\_SAVE\_F32\_BIN
In Capture mode, the render will be saved as a binary file containing float32 RGBA data.

### DESPERATE
If enabled, uses a couple of risky tricks to shave a few bytes.

### REVISION_RULESET
If enabled, sets up a party-safe configuration based on the [Revision](https://revision-party.net/competitions/graphics-competitions) rules.

## As Seen In...
* [Submit](https://demozoo.org/graphics/292417/) by yx
* [Tesseract](https://demozoo.org/graphics/292406/) by Sinmix
* [You Are Here](https://demozoo.org/graphics/292426/) by tdhooper

...and [more](https://demozoo.org/productions/tagged/blossom/)!

## Special Thanks
* LLB, for [Shader Minifier](https://github.com/laurentlb/Shader_Minifier)
* Blueberry & Mentor, for [Crinkler](https://github.com/runestubbe/Crinkler)
* noby, for [Leviathan](https://github.com/armak/Leviathan-2.0), which served as an invaluable reference.