LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := external.sh

LOCAL_CFLAGS := -std=c++17 -fvisibility=hidden -DUSE_OPENGL
LOCAL_CPPFLAGS := -std=c++17 -fvisibility=hidden -DUSE_OPENGL -Wno-error=format-security -fexceptions -fno-rtti

LOCAL_C_INCLUDES += \
$(LOCAL_PATH)/include \
$(LOCAL_PATH)/include/ImGui \
$(LOCAL_PATH)/include/ImGui/backends \
$(LOCAL_PATH)/src/sdk \
$(LOCAL_PATH)/src
    
LOCAL_SRC_FILES := \ src/main.cpp \ src/window/Android_draw/draw.cpp \ src/window/Android_touch/Touch.cpp \ src/window/ImGui/imgui.cpp \ src/window/ImGui/imgui_demo.cpp \ src/window/ImGui/imgui_draw.cpp \ src/window/ImGui/imgui_tables.cpp \ src/window/ImGui/imgui_widgets.cpp \ src/window/ImGui/backends/imgui_impl_android.cpp \ src/window/ImGui/backends/imgui_impl_opengl3.cpp

LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv3

include $(BUILD_EXECUTABLE) #可执行文件
