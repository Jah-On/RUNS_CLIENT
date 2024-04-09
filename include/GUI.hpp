#ifdef _WIN32
    #include <windows.h>
    #include <gl/GL.h>
#endif

#ifdef __APPLE__
    #include <OpenGL/gl.h>
#endif

#include <array>
#include <Robot.hpp>
#include <chrono>
#ifdef __linux__
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#endif
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

typedef std::chrono::high_resolution_clock precise_clock;

class GUI {
private:
    bool                       should_exit = false;
    int                        frame_rate;
    float                      font_scale;
    GLFWwindow                *wm_ctx;
    ImGuiContext              *im_ctx;
    ImGuiViewport             *imgui_vp;
    ImGuiIO                    gui_io;
    ImFont                    *base_font;
    ImFont                    *temp_font;
    ImFont                    *rotation_font;
    RUNS::Robot               *robot;
    int                        bumper_y;
    int                        temps_y;
    int                        speed_slider_value;
    RUNS::Rotation             gui_rotation_value;
    precise_clock::time_point  timer;
    enum {
        BT_OFF, NOT_CONNECTED, CONNECTED
    }                          UI_State  = BT_OFF;

    std::array<ImU32, 2> BUMPER_COLOR_MAP;
    ImU32                      LEFT_COLOR;
    ImU32                      RIGHT_COLOR;

    void createBaseWindow();
    void endBaseWindow();

    void styleButtonWhite();

    void drawError(std::string, std::string, std::function<void()>);
    void drawBumpers();
    void drawTemperatures();
    void drawMovement();
    void updateRobotFromUI();
    void exit();
protected:
public:
    GUI(std::string, int, int);
    void run();
};