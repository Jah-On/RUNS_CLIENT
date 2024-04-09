#include <GUI.hpp>

#define   BUMPER_PADDING_X    30 // Pixels
#define   BUMPER_PADDING_Y    30
#define   BUMPER_GAP          30
const int BUMPER_TOTAL{5*BUMPER_GAP+2*BUMPER_PADDING_X};

#define   TEMP_PADDING_X      60
#define   TEMP_PADDING_Y      30
#define   TEMP_GAP            30

#define   SPEED_WIDTH         0.1f // %
#define   SPEED_HEIGHT        0.4f
#define   SPEED_BOTTOM_OFFSET 40

GUI::GUI(std::string title, int width, int height){
    if (!glfwInit()){
        throw std::runtime_error(
            "GLFW could not init!"
        );
    }
    if ((width <= 0) || (height <= 0)){
        wm_ctx = glfwCreateWindow(800, 640, title.c_str(), nullptr, nullptr);
        glfwMaximizeWindow(wm_ctx);
    } else {
        wm_ctx = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    }
    frame_rate = glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate;

#if defined(IMGUI_IMPL_OPENGL_ES2)
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
#endif
    glfwSwapInterval(1);
    glfwMakeContextCurrent(wm_ctx);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    this->gui_io = ImGui::GetIO();
    base_font = gui_io.Fonts->AddFontFromFileTTF("assets/RobotoMono-VariableFont_wght.ttf", 16);
    base_font->Scale = 1;
    
    temp_font = gui_io.Fonts->AddFontFromFileTTF("assets/RobotoMono-VariableFont_wght.ttf", 32);
    temp_font->Scale = 1;

    gui_io.FontDefault = base_font;

    ImGui_ImplGlfw_InitForOpenGL(wm_ctx, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    ImGui::StyleColorsDark();

    BUMPER_COLOR_MAP = {
        ImGui::GetColorU32(IM_COL32(255, 255, 255, 200)), 
        ImGui::GetColorU32(IM_COL32(  0, 255,   0, 255))
    };

    LEFT_COLOR  = ImGui::GetColorU32(IM_COL32(  0, 0, 255, 255));
    RIGHT_COLOR = ImGui::GetColorU32(IM_COL32(255, 0,   0, 255));

    robot = new RUNS::Robot;
}

void GUI::createBaseWindow(){
    imgui_vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(imgui_vp->Pos);
    ImGui::SetNextWindowSize(imgui_vp->Size);

    ImGui::Begin(
        "#", 
        nullptr, 
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove
    );
    ImGui::GetStyle().WindowBorderSize = 0.0f;
}

void GUI::endBaseWindow(){
    ImGui::End();
}

void GUI::drawError(std::string msg, std::string button, std::function<void()> lambda){
    ImGui::OpenPopup("##error");
    if (ImGui::BeginPopupModal(
        "##error",
        nullptr,
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoTitleBar
    )){
        ImGui::PushFont(temp_font);
        ImGui::Text(msg.c_str());
        if (button.length() > 0){
            if (ImGui::Button(button.c_str())){
                lambda();
            }
        } else {
            lambda();
        }
        ImGui::PopFont();
        ImGui::SetWindowPos(ImVec2(
            imgui_vp->GetWorkCenter().x - (ImGui::GetWindowSize().x*0.5),
            imgui_vp->GetWorkCenter().y - (ImGui::GetWindowSize().y*0.5)
        ));
        ImGui::EndPopup();
    }    
}

void GUI::drawBumpers(){
    int8_t bumpers = this->robot->getBumpers();
    int    size    = (imgui_vp->Size.x - BUMPER_TOTAL)/6;
    ImGui::SetCursorPosX(((int)(imgui_vp->Size.x - ImGui::CalcTextSize("Bumpers").x))>>1);
    ImGui::Text("Bumpers");
    ImVec2 start = ImVec2(BUMPER_PADDING_X, BUMPER_PADDING_Y);
    ImVec2 end   = ImVec2(start.x + size, start.y + size);
    bumper_y     = end.y + BUMPER_PADDING_Y;

    ImDrawList *draw = ImGui::GetWindowDrawList();
    for (int bit = 5; bit >= 0; --bit){
        draw->AddRectFilled(start, end, BUMPER_COLOR_MAP[(bumpers>>bit)&1], 10.0f, 0);
        start.x += size + BUMPER_GAP;
        end.x   += size + BUMPER_GAP;
    }

    if ((speed_slider_value > 0) && bumpers){
        speed_slider_value = 0;
    }
}

void GUI::drawTemperatures(){
    ImVec2 start = ImVec2(TEMP_PADDING_X, bumper_y + TEMP_PADDING_Y);
    ImVec2 end   = start;
    ImGui::SetCursorPos(start);
    ImGui::PushFont(temp_font);
    ImGui::Text("MCU Temperature: %d°C", robot->getMicroprocessorTemp());
    end.y += ImGui::CalcTextSize("MCU Temperature: %d°C").y;
    ImGui::PopFont();

    start.y = end.y + TEMP_GAP;
    ImGui::SetCursorPos(start);
    ImGui::PushFont(temp_font);
    ImGui::Text("Ambient Temperature: %d°C", robot->getEnvironmentTemp());
    end.y = start.y + ImGui::CalcTextSize("MCU Temperature: %d°C").y + TEMP_PADDING_Y;
    ImGui::PopFont();
}

void GUI::drawMovement(){
    ImDrawList *draw = ImGui::GetForegroundDrawList();
    ImVec2 sliderSize = {
        ImGui::CalcTextSize("Speed: -100%").x + 10,
        imgui_vp->Size.y * SPEED_HEIGHT,
    };
    ImVec2 sliderPos = {
        imgui_vp->Size.x/2 - sliderSize.x/2,
        imgui_vp->Size.y - sliderSize.y - SPEED_BOTTOM_OFFSET
    };

    ImVec2 leftPoints[3] = {
        {sliderPos.x - 200, sliderPos.y + sliderSize.y/2},
        {sliderPos.x - 100, sliderPos.y + sliderSize.y/4},
        {sliderPos.x - 100, sliderPos.y + (3*sliderSize.y)/4},
    };

    ImVec2 rightPoints[3] = {
        {sliderPos.x + sliderSize.x + 200, sliderPos.y + sliderSize.y/2},
        {sliderPos.x + sliderSize.x + 100, sliderPos.y + sliderSize.y/4},
        {sliderPos.x + sliderSize.x + 100, sliderPos.y + (3*sliderSize.y)/4},
    };

    if (ImGui::IsKeyDown(ImGuiKey_A)){
        draw->AddTriangleFilled(leftPoints[0], leftPoints[1], leftPoints[2], LEFT_COLOR);
    } else {
        draw->AddTriangle(leftPoints[0], leftPoints[1], leftPoints[2], LEFT_COLOR);
    }

    ImGui::SetCursorPos(sliderPos);
    ImGui::VSliderInt("##speedSlider", sliderSize, &speed_slider_value, -100, 100, "Speed: %d%%");

    if (ImGui::IsKeyDown(ImGuiKey_D)){
        draw->AddTriangleFilled(rightPoints[0], rightPoints[1], rightPoints[2], RIGHT_COLOR);
    } else {
        draw->AddTriangle(rightPoints[0], rightPoints[1], rightPoints[2], RIGHT_COLOR);
    }
}

void GUI::updateRobotFromUI(){
    if (
        ImGui::IsKeyDown(ImGuiKey_W) && 
        !ImGui::IsKeyDown(ImGuiKey_S) && 
        !ImGui::IsKeyDown(ImGuiKey_Space) && 
        speed_slider_value < 100
    ){
        speed_slider_value++;
    } else if (
        ImGui::IsKeyDown(ImGuiKey_S) && 
        !ImGui::IsKeyDown(ImGuiKey_W) && 
        !ImGui::IsKeyDown(ImGuiKey_Space) && 
        speed_slider_value > -100
    ){
        speed_slider_value--;
    }

    speed_slider_value = !ImGui::IsKeyDown(ImGuiKey_Space);

    if (ImGui::IsKeyDown(ImGuiKey_A) && !ImGui::IsKeyDown(ImGuiKey_D)){
        gui_rotation_value = RUNS::Rotation::LEFT;
    } else if (ImGui::IsKeyDown(ImGuiKey_D) && !ImGui::IsKeyDown(ImGuiKey_A)){
        gui_rotation_value = RUNS::Rotation::RIGHT;
    } else {
        gui_rotation_value = RUNS::Rotation::NONE;
    }
    
    if (gui_rotation_value != robot->getRotation()){
        robot->setRotation(gui_rotation_value);
    }

    if (speed_slider_value != robot->getVelocity()){
        robot->setVelocity(speed_slider_value);
    }
}

void GUI::run(){
    ImVec4 clear_color = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    int display_w, display_h;

    while (!glfwWindowShouldClose(wm_ctx) && !should_exit){
        glfwPollEvents();
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        glfwGetFramebufferSize(wm_ctx, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (
            ImGui::IsKeyDown(ImGuiKey_Escape)
            // ImGui::IsKeyDown(ImGuiKey_Q) && 
            // (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl))
        ){
            break;
        }

        createBaseWindow();
        switch (UI_State){
        case (BT_OFF):
            drawError(
                "Bluetooth off, please turn back on", 
                "", 
                [this](){
                    if (RUNS::Robot::bluetoothEnabled()){
                        this->UI_State = NOT_CONNECTED;
                    }
                }
            );
            break;
        case (NOT_CONNECTED):
            drawError(
                "Waiting for a connected robot...", 
                "", 
                [this](){
                    if (this->robot->isConnected()){
                        this->UI_State = CONNECTED;
                    } else if (!RUNS::Robot::bluetoothEnabled()){
                        this->UI_State = BT_OFF;
                    }
                }
            );
            break;
        default:
            if (!this->robot->isConnected()){
                this->UI_State = NOT_CONNECTED;
                break;
            } else if (!RUNS::Robot::bluetoothEnabled()){
                this->UI_State = BT_OFF;
                break;
            }
            drawBumpers();
            drawTemperatures();
            drawMovement();
            updateRobotFromUI();
            break;
        }
        // drawBumpers();
        // drawTemperatures();
        // drawMovement();
        endBaseWindow();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(wm_ctx);
        std::this_thread::sleep_for(
            std::chrono::milliseconds(int(1000/frame_rate))
        );
    }

    exit();
}

void GUI::exit(){
    delete robot;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
