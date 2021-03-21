// system
#include <cstdio>
#include <cstdint>
#include <Windows.h>

// gl
#include <gl/GL.h>
#include "glext.h"
#include "gldefs.h"

// config
#include "config.h"

// shaders
#include "frag_draw.h"
#undef VAR_IRESOLUTION
#include "frag_present.h"

// requirements for capture mode
#if CAPTURE
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <ctime>
#endif

// shaders
GLuint gShaderDraw;
GLuint gShaderPresent;

// framebuffers
GLuint fbAccumulator;

// uniform bindings
int const kUniformResolution = 0;
int const kUniformFrame = 1;
int const kSamplerAccumulatorTex = 0;

// === resolutions ===
#if WINDOW_AUTO_SIZE
// the ugliest comma operator hack I will ever write
int const kCanvasWidth = (SetProcessDPIAware(), GetSystemMetrics(SM_CXSCREEN));
int const kCanvasHeight = GetSystemMetrics(SM_CYSCREEN);
#else
#define kCanvasWidth CANVAS_WIDTH
#define kCanvasHeight CANVAS_HEIGHT
#endif

#define kWindowWidth kCanvasWidth
#define kWindowHeight kCanvasHeight
// =====================



// capture GL errors
#if _DEBUG
void __stdcall
MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);

	__debugbreak();
}
#endif

GLuint makeFramebuffer()
{
	GLuint name, backing;
	glGenFramebuffers(1, &name);
	glBindFramebuffer(GL_FRAMEBUFFER, name);
	glGenTextures(1, &backing);
	glBindTexture(GL_TEXTURE_2D, backing);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, kWindowWidth, kWindowHeight, 0, GL_RGBA, GL_FLOAT, 0);

	// don't remove these!
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, backing, 0);
	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);
	return name;
}

GLuint makeShader(const char* source)
{
#if _DEBUG
	GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(shader, 1, &source, 0);
	glCompileShader(shader);

	// shader compiler errors
	GLint isCompiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		const int maxLength = 1024;
		GLchar errorLog[maxLength];
		glGetShaderInfoLog(shader, maxLength, 0, errorLog);
		puts(errorLog);
		glDeleteShader(shader);
		__debugbreak();
	}

	// link shader
	GLuint m_program = glCreateProgram();
	glAttachShader(m_program, shader);
	glLinkProgram(m_program);

	GLint isLinked = 0;
	glGetProgramiv(m_program, GL_LINK_STATUS, &isLinked);
	if (isLinked == GL_FALSE)
	{
		const int maxLength = 1024;
		GLchar errorLog[maxLength];
		glGetProgramInfoLog(m_program, maxLength, 0, errorLog);
		puts(errorLog);
		glDeleteProgram(m_program);
		__debugbreak();
	}
	
	return m_program;
#else
	return glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &source);
#endif

}

void bindSharedUniforms()
{
	glUniform4f(
		kUniformResolution,
		(float)kCanvasWidth,
		(float)kCanvasHeight,
		(float)kCanvasWidth / (float)kCanvasHeight,
		(float)kCanvasHeight / (float)kCanvasWidth);
}

static inline void accumulatorSetup()
{
	glUseProgram(gShaderDraw);
	glBindFramebuffer(GL_FRAMEBUFFER, fbAccumulator);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	bindSharedUniforms();
}

static inline void presentSetup(int destFb)
{
	glUseProgram(gShaderPresent);
	glBindFramebuffer(GL_FRAMEBUFFER, destFb);

	glDisable(GL_BLEND);

	bindSharedUniforms();
	glBindTexture(GL_TEXTURE_2D, fbAccumulator);
}

static inline void accumulatorRender(int sampleCount)
{
	glUniform1i(kUniformFrame, sampleCount);
	glRecti(-1, -1, 1, 1);

	// deliberately block so we don't queue up more work than we have time for
	glFinish();
}

static inline void presentRender(HDC hDC)
{
	glRecti(-1, -1, 1, 1);
	SwapBuffers(hDC);
}

#if defined(RELEASE)
int WinMainCRTStartup()
#else
int main()
#endif
{
	unsigned int startTime = timeGetTime();

	DEVMODE screenSettings = {
		{0}, 0, 0, sizeof(screenSettings), 0, DM_PELSWIDTH | DM_PELSHEIGHT,
		{0}, 0, 0, 0, 0, 0, {0}, 0, 0, (DWORD)kWindowWidth, (DWORD)kWindowHeight, 0, 0,
		#if(WINVER >= 0x0400)
			0, 0, 0, 0, 0, 0,
			#if (WINVER >= 0x0500) || (_WIN32_WINNT >= 0x0400)
				0, 0
			#endif
		#endif
	};
	const PIXELFORMATDESCRIPTOR pfd = {
		sizeof(pfd), 1, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA,
		32, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 32, 0, 0, PFD_MAIN_PLANE, 0, 0, 0, 0
	};
	
	#if WINDOW_FULLSCREEN
		ChangeDisplaySettings(&screenSettings, CDS_FULLSCREEN);
		ShowCursor(0);
		HDC hDC = GetDC(CreateWindow((LPCSTR)0xC018, 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0));
	#else
		HDC hDC = GetDC(CreateWindow((LPCSTR)0xC018, 0, WS_POPUP | WS_VISIBLE, 0, 0, kWindowWidth, kWindowHeight, 0, 0, 0, 0));
	#endif

	// set pixel format and make opengl context
	SetPixelFormat(hDC, ChoosePixelFormat(hDC, &pfd), &pfd);
	wglMakeCurrent(hDC, wglCreateContext(hDC));
	SwapBuffers(hDC);

	// enable opengl debug messages
#if _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);
#endif

	// make framebuffer
	fbAccumulator = makeFramebuffer();

	// optional extra buffers/textures
#if CAPTURE
	GLuint const fbCapture = makeFramebuffer();
	float* cpuFramebufferFloat = new float[kCanvasWidth * kCanvasHeight * 4];
	uint8_t* cpuFramebufferU8 = new uint8_t[kCanvasWidth * kCanvasHeight * 3];
#endif

	// make shaders
	gShaderDraw = makeShader(draw_frag);
	gShaderPresent = makeShader(present_frag);

	// main accumulator loop
	accumulatorSetup();
#if !DESPERATE
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
#endif
	for (
		int sampleCount = 0;
#ifdef RENDER_EXACT_SAMPLES
		sampleCount < RENDER_EXACT_SAMPLES;
#else
	#ifdef RENDER_MIN_SAMPLES
			(sampleCount < RENDER_MIN_SAMPLES) ||
	#endif
	#ifdef RENDER_MAX_SAMPLES
			(sampleCount < RENDER_MAX_SAMPLES) &&
	#endif
			(timeGetTime() < startTime + RENDER_MAX_TIME_MS);
#endif
		++sampleCount
	)
	{
#if _DEBUG
		printf("accumulate sample %d\n", sampleCount);
#endif

		#if !DESPERATE
		PeekMessage(0, 0, 0, 0, PM_REMOVE);
		#endif
		accumulatorRender(sampleCount);

		// To prevent accidentally hitting esc during long renders. Use Alt+F4 instead.
		#if !CAPTURE
		if (GetAsyncKeyState(VK_ESCAPE))
			goto abort;
		#endif


		#if RENDER_PROGRESSIVE
			#if CAPTURE
			if ((sampleCount&(sampleCount-1))==0)
			#endif
			{
				presentSetup(0);
				presentRender(hDC);

				accumulatorSetup();
			}
		#endif
	}

#if CAPTURE
	presentSetup(fbCapture);
	presentRender(hDC);
	glFinish();
	glReadPixels(0, 0, kCanvasWidth, kCanvasHeight, GL_RGBA, GL_FLOAT, cpuFramebufferFloat);

	for (int y = 0; y < kCanvasHeight; ++y) {
		int invy = kCanvasHeight - y - 1;
		for (int x = 0; x < kCanvasWidth; ++x) {
			for (int i = 0; i < 3; ++i) {
				float const fval = cpuFramebufferFloat[(invy*kCanvasWidth + x) * 4 + i];
				uint8_t const u8val = fval < 0 ? 0 : (fval > 1 ? 255 : (uint8_t)(fval * 255));
				cpuFramebufferU8[(y*kCanvasWidth + x) * 3 + i] = u8val;
			}
		}
	}
	delete[] cpuFramebufferFloat;

	char name[256];
	sprintf(name, "%llu_%dx%d_%dspp", time(NULL), kCanvasWidth, kCanvasHeight, RENDER_EXACT_SAMPLES);

#if CAPTURE_SAVE_U8_BIN
	{
		OutputDebugString("saving u8 bin\n");
		char binname[256];
		sprintf(binname, "%s.u8.bin", name);
		FILE* fh = fopen(binname, "wb");
		fwrite(cpuFramebufferU8, 1, kCanvasWidth * kCanvasHeight * 3, fh);
		fclose(fh);
	}
#endif

#if CAPTURE_SAVE_F32_BIN
	{
		OutputDebugString("saving f32 bin\n");
		char binname[256];
		sprintf(binname, "%s.f32.bin", name);
		FILE* fh = fopen(binname, "wb");
		fwrite(cpuFramebufferFloat, 4, kCanvasWidth * kCanvasHeight * 4, fh);
		fclose(fh);
	}
#endif

#if CAPTURE_SAVE_JPG
	{
		OutputDebugString("saving jpg\n");
		char jpgname[256];
		sprintf(jpgname, "%s.jpg", name);
		stbi_write_jpg(jpgname, kCanvasWidth, kCanvasHeight, 3, cpuFramebufferU8, 100);
	}
#endif

#if CAPTURE_SAVE_PNG
	{
		OutputDebugString("saving png\n");
		char pngname[256];
		sprintf(pngname, "%s.png", name);
		stbi_write_png(pngname, kCanvasWidth, kCanvasHeight, 3, cpuFramebufferU8, kCanvasWidth * 3);
	}
#endif

	delete[] cpuFramebufferU8;
#else
	presentSetup(0);
	while (!GetAsyncKeyState(VK_ESCAPE))
	{
		PeekMessage(0, 0, 0, 0, PM_REMOVE);
		presentRender(hDC);
	}
#endif

abort:
	ExitProcess(0);
	return 0;
}
