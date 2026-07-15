#include "Android_draw/draw.h"

#include "../Fonts/robotoRegular.h"
#include "../Fonts/bold.h"
#include "../Fonts/icons.h"
#include "../Fonts/verdana.h"

#include "../Fonts/article.h"

#include "../Fonts/ovnir.h"
#include "../Fonts/saveicon.h"
#include "../Fonts/logotutef.h"
#include "../Fonts/text_font.h"
#include "../Fonts/esp_font.h"
#include "../Fonts/granades_font.h"
#include "../Fonts/knife_font.h"
#include "../Fonts/weapons_font.h"
#include "../Fonts/pistols_font.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_android.h"

ImFont* boldFont;
ImFont* verdanaFont;
ImFont* ovnirr;
ImFont* saveiconf;
ImFont* text_fonts;
ImFont* text_fonts2;
ImFont* logotypefont;
ImFont* esp_fonts;
ImFont* weapons_fonts;
ImFont* knife_fonts;
ImFont* granades_fonts;
ImFont* pistols_fonts;
ImFont* a1;
ImFont* a2;

EGLDisplay display = EGL_NO_DISPLAY;
EGLConfig config;
EGLSurface surface = EGL_NO_SURFACE;
EGLContext context = EGL_NO_CONTEXT;

ANativeWindow *native_window;

int native_window_screen_x = 0;
int native_window_screen_y = 0;
android::ANativeWindowCreator::DisplayInfo displayInfo{0};
uint32_t orientation = 0;
bool g_Initialized = false;
ImGuiWindow *g_window = nullptr;

bool initGUI_draw(uint32_t _screen_x, uint32_t _screen_y, bool log) {
    orientation = displayInfo.orientation;

    #if defined(USE_OPENGL)
        if (!init_egl(_screen_x, _screen_y, log)) {
            return false;
        }
    #else
        InitVulkan();
        SetupVulkan();
        ::native_window = android::ANativeWindowCreator::Create("AImGui", _screen_x, _screen_y, false);
        SetupVulkanWindow(::native_window, (int) _screen_x, (int) _screen_y);
    #endif
    if (!ImGui_init()) {
        return false;
    }   

    #ifndef USE_OPENGL
        UploadFonts();
    #endif
     
    return true;
}

bool init_egl(uint32_t _screen_x, uint32_t _screen_y, bool log) {
    ::native_window = android::ANativeWindowCreator::Create("AImGui", _screen_x, _screen_y, false);
    if (!native_window) {
        return false;
    }
    ANativeWindow_acquire(native_window);
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        return false;
    }
    if (log) {
    }
    if (eglInitialize(display, 0, 0) != EGL_TRUE) {
        return false;
    }
    if (log) {
    }
    EGLint num_config = 0;
    const EGLint attribList[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_BLUE_SIZE, 5,   //-->delete
            EGL_GREEN_SIZE, 6,  //-->delete
            EGL_RED_SIZE, 5,    //-->delete
            EGL_BUFFER_SIZE, 32,  //-->new field
            EGL_DEPTH_SIZE, 16,
            EGL_STENCIL_SIZE, 8,
            EGL_NONE
    };
    const EGLint attrib_list[] = {
            EGL_CONTEXT_CLIENT_VERSION,
            3,
            EGL_NONE
    };

    if (log) {
    }
    if (eglChooseConfig(display, attribList, &config, 1, &num_config) != EGL_TRUE) {
        return false;
    }
    if (log) {
    }
    EGLint egl_format;
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &egl_format);
    ANativeWindow_setBuffersGeometry(native_window, 0, 0, egl_format);
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, attrib_list);
    if (context == EGL_NO_CONTEXT) {
        return false;
    }
    if (log) {
    }
    surface = eglCreateWindowSurface(display, config, native_window, nullptr);
    if (surface == EGL_NO_SURFACE) {
        return false;
    }
    if (log) {
    }
    if (!eglMakeCurrent(display, surface, surface, context)) {
        return false;
    }
    if (log) {
    }
    return true;
}

void screen_config() {
    displayInfo = android::ANativeWindowCreator::GetDisplayInfo();
}
bool ImGui_init() {
    if (g_Initialized) {
        return true;
    }
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplAndroid_Init(native_window);
    #if defined(USE_OPENGL)
        ImGui_ImplOpenGL3_Init("#version 300 es");
    #endif
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = NULL;
    static ImFontConfig standardFont;
    standardFont.SizePixels = 22.0f;
	io.Fonts->AddFontFromMemoryTTF(&robotoRegular, sizeof(robotoRegular), 38.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
	io.Fonts->AddFontDefault(&standardFont);
	boldFont = io.Fonts->AddFontFromMemoryTTF(&bold, sizeof(bold), 40.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
	font::icomoon = io.Fonts->AddFontFromMemoryTTF(&iconsFont, sizeof(iconsFont), 28.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
	verdanaFont = io.Fonts->AddFontFromMemoryTTF(&verdana, sizeof(verdana), 40.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
	
	a2 = io.Fonts->AddFontFromMemoryTTF(&articleicon, sizeof articleicon, 155.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
	a1 = io.Fonts->AddFontFromMemoryTTF(&articleicon, sizeof articleicon, 60.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
	ovnirr = io.Fonts->AddFontFromMemoryTTF(&ovnir, sizeof ovnir, 50.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
	saveiconf = io.Fonts->AddFontFromMemoryTTF(&saveicon, sizeof saveicon, 50.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
	text_fonts = io.Fonts->AddFontFromMemoryTTF(&text_font, sizeof text_font, 46.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
	text_fonts2 = io.Fonts->AddFontFromMemoryTTF(&text_font, sizeof text_font, 35.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
logotypefont = io.Fonts->AddFontFromMemoryTTF(&logotype, sizeof logotype, 38.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
esp_fonts = io.Fonts->AddFontFromMemoryTTF(&esp_font, sizeof esp_font, 25.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
granades_fonts = io.Fonts->AddFontFromMemoryTTF(&granades_font, sizeof granades_font, 25.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
knife_fonts = io.Fonts->AddFontFromMemoryTTF(&knife_font, sizeof knife_font, 25.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
weapons_fonts = io.Fonts->AddFontFromMemoryTTF(&weapons_font, sizeof weapons_font, 25.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
pistols_fonts = io.Fonts->AddFontFromMemoryTTF(&pistols_font, sizeof pistols_font, 25.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
	::g_Initialized = true;
    return true;
}

void drawBegin() {
    screen_config();
    if (::orientation != displayInfo.orientation) {
        ::orientation = displayInfo.orientation;
        touch::update(displayInfo.width, displayInfo.height, displayInfo.orientation);
        g_window->Pos.x = 100;
        g_window->Pos.y = 125;
    }

    #ifdef USE_OPENGL
        ImGui_ImplOpenGL3_NewFrame();
    #else
        ImGui_ImplVulkan_NewFrame();
    #endif        
    ImGui_ImplAndroid_NewFrame(native_window_screen_x, native_window_screen_y);
    ImGui::NewFrame();
}

void drawEnd() {
    ImGui::Render();
    
    #ifdef USE_OPENGL
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        eglSwapBuffers(display, surface);
    #else
        FrameRender(ImGui::GetDrawData());
        FramePresent();
    #endif
}



void shutdown() {
    if (!g_Initialized) {
        return;
    }
    // Cleanup
    #ifdef USE_OPENGL
        ImGui_ImplOpenGL3_Shutdown();
    #else
        DeviceWait();
        ImGui_ImplVulkan_Shutdown();
    #endif
    ImGui_ImplAndroid_Shutdown();
    ImGui::DestroyContext();
    
    
    #ifdef USE_OPENGL
        if (display != EGL_NO_DISPLAY) {
            eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            if (context != EGL_NO_CONTEXT) {
                eglDestroyContext(display, context);
            }
            if (surface != EGL_NO_SURFACE) {
                eglDestroySurface(display, surface);
            }
            eglTerminate(display);
        }
        display = EGL_NO_DISPLAY;
        context = EGL_NO_CONTEXT;
        surface = EGL_NO_SURFACE;
    #else    
        CleanupVulkanWindow();
        CleanupVulkan();
    #endif
    
    ANativeWindow_release(native_window);
    android::ANativeWindowCreator::Destroy(native_window);
    ::g_Initialized = false;
}
