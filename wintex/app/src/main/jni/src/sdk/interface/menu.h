using namespace ImGui;

extern ImFont* boldFont;
extern ImFont* a1;
extern ImFont* a2;
extern ImFont* ovnirr;

struct Menu {
        bool isMenuOpen = true;
} menu;

void huinarnad() {
   SetNextWindowPos(ImVec2{10000, 10000});
   g_window = GetCurrentWindow();
   End();
}

struct Snowflake
{
    ImVec2 position;
    ImVec2 velocity;
    float size;
    float rotation;
    float rotationSpeed;
    int type;
};

std::vector<Snowflake> snowflakes;

void InitializeSnowflakes()
{
    snowflakes.clear();
    ImVec2 screen = ImGui::GetIO().DisplaySize;
    for (int i = 0; i < 350; i++)
    {
        Snowflake flake;
        flake.position = ImVec2(screen.x * ((float)rand() / RAND_MAX),
                                screen.y * ((float)rand() / RAND_MAX));
        flake.size = 2.0f + ((float)rand() / RAND_MAX) * 4.0f;
        float speed = 20.0f + ((float)rand() / RAND_MAX) * 50.0f;
        flake.velocity = ImVec2(-15.0f + ((float)rand() / RAND_MAX) * 30.0f, speed);
        flake.rotation = ((float)rand() / RAND_MAX) * 6.28318530718f;
        flake.rotationSpeed = -1.5f + ((float)rand() / RAND_MAX) * 3.0f;
        flake.type = rand() % 3; // разные формы
        snowflakes.push_back(flake);
    }
}

void drawMenu() {
    static bool watermark;
    menuConfig::accent = ImVec4(accentColor[0], accentColor[1], accentColor[2], 1.0f);
    {
    SetNextWindowPos(ImVec2(200, 0));
    Begin("##watermarnk", nullptr,
    ImGuiWindowFlags_NoBackground |
    ImGuiWindowFlags_NoDecoration |
    ImGuiWindowFlags_NoBringToFrontOnFocus |
    ImGuiWindowFlags_AlwaysAutoResize);
    if (InvisibleButton(("##regulator"), ImVec2(140, 40)))
    menu.isMenuOpen = !menu.isMenuOpen;
    End();
    }
    
    {
    SetNextWindowPos(ImVec2(120, 20));
    SetNextWindowSize(ImVec2(615, 70));    
    Begin("##watermark", nullptr,
    ImGuiWindowFlags_NoDecoration |
    ImGuiWindowFlags_NoBringToFrontOnFocus |
    ImGuiWindowFlags_AlwaysAutoResize);
    
    ImVec2 windowSize = GetWindowSize();
    ImVec2 cursorPos = GetCursorPos();
    
    SetCursorPos(ImVec2(0, 0));
    if (InvisibleButton("##watermark_click_area", windowSize))
    menu.isMenuOpen = !menu.isMenuOpen;
    SetCursorPos(cursorPos);
    
    float windowHeight = windowSize.y;
    float totalTextHeight = GetTextLineHeight();
    float centerY = (windowHeight - totalTextHeight) * 0.5f;
    
    SetCursorPosY(centerY);
    
    PushFont(ovnirr);
    TextGradient(obf("xxxstuxxx"));
    PopFont();
    SameLine(); Text(obf(" | "));
    SameLine(); TextGradient(obf("external"));
    SameLine(); Text(obf(" | "));
    SameLine(); Text(obf("0.38.2"));
    
    End();
}

if (!menu.isMenuOpen)
    return;

if (background)
{
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;

    float alpha = backgroundOpacity / 100.0f;
    if (alpha > 0.75f) alpha = 0.75f;

    ImGui::GetBackgroundDrawList()->AddRectFilled(
        ImVec2(0, 0),
        displaySize,
        ImGui::GetColorU32(ImVec4(0.f, 0.f, 0.f, alpha))
    );
}

if (snowflake)
{
    if (snowflakes.empty())
        InitializeSnowflakes();

    ImDrawList* draw = ImGui::GetBackgroundDrawList();
    ImVec2 screen = ImGui::GetIO().DisplaySize;
    float dt = ImGui::GetIO().DeltaTime;

    for (auto& flake : snowflakes)
    {
        flake.position.x += flake.velocity.x * dt;
        flake.position.y += flake.velocity.y * dt;
        flake.rotation += flake.rotationSpeed * dt;

        if (flake.position.y > screen.y)
        {
            flake.position.y = 0;
            flake.position.x = screen.x * ((float)rand() / RAND_MAX);
        }
        if (flake.position.x < 0) flake.position.x = screen.x;
        if (flake.position.x > screen.x) flake.position.x = 0;

        ImU32 color = IM_COL32(255, 255, 255, 220);

        float half = flake.size * 0.5f;
        float cosR = cosf(flake.rotation);
        float sinR = sinf(flake.rotation);

        auto rotate = [&](float x, float y) -> ImVec2 {
            return ImVec2(x * cosR - y * sinR, x * sinR + y * cosR);
        };

        ImVec2 c = flake.position;

        if (flake.type == 0)
        {
            ImVec2 p1 = ImVec2(c.x + rotate(half, 0).x, c.y + rotate(half, 0).y);
            ImVec2 p2 = ImVec2(c.x + rotate(-half, 0).x, c.y + rotate(-half, 0).y);
            ImVec2 p3 = ImVec2(c.x + rotate(0, half).x, c.y + rotate(0, half).y);
            ImVec2 p4 = ImVec2(c.x + rotate(0, -half).x, c.y + rotate(0, -half).y);
            draw->AddLine(p1, p2, color, 1.0f);
            draw->AddLine(p3, p4, color, 1.0f);
        }
        else if (flake.type == 1)
        {
            ImVec2 p1 = ImVec2(c.x + rotate(half, half).x, c.y + rotate(half, half).y);
            ImVec2 p2 = ImVec2(c.x + rotate(-half, -half).x, c.y + rotate(-half, -half).y);
            ImVec2 p3 = ImVec2(c.x + rotate(half, -half).x, c.y + rotate(half, -half).y);
            ImVec2 p4 = ImVec2(c.x + rotate(-half, half).x, c.y + rotate(-half, half).y);
            draw->AddLine(p1, p2, color, 1.0f);
            draw->AddLine(p3, p4, color, 1.0f);
        }
        else
        {
            draw->AddCircleFilled(c, flake.size * 0.5f, color, 6);
            draw->AddLine(ImVec2(c.x - flake.size * 0.5f, c.y), ImVec2(c.x + flake.size * 0.5f, c.y), color, 1.0f);
            draw->AddLine(ImVec2(c.x, c.y - flake.size * 0.5f), ImVec2(c.x, c.y + flake.size * 0.5f), color, 1.0f);
        }
    }
}

    
    SetNextWindowSize(ImVec2(1120, 800));
    Begin("##mainMenu", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
    ImGuiStyle* style = &GetStyle();
    style->ItemSpacing = ImVec2(13, 23);
    style->WindowBorderSize = 1;
    style->ScrollbarSize = scrollBarWidth * 1.1f;
    style->WindowRounding = 3.0f;
    style->Colors[ImGuiCol_ScrollbarBg] = ImColor(16, 16, 16);
    style->Colors[ImGuiCol_ScrollbarGrab] = ImColor(menuConfig::accent);
    style->Colors[ImGuiCol_ScrollbarGrabActive] = ImColor(menuConfig::accent);
    style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImColor(menuConfig::accent);
    
    float windowWidth = ImGui::GetWindowSize().x;
std::string _t = obf("WINTYX CHEAT");
const char* txt = _t.c_str();

ImGui::SetWindowFontScale(1.7f);
ImGui::SetCursorPos(ImVec2(15.0f, ImGui::GetCursorPosY()));
ImDrawList* draw = ImGui::GetWindowDrawList();
ImVec2 pos = ImGui::GetCursorScreenPos();
ImVec2 size = ImGui::CalcTextSize(txt);

    
    ImVec4 base = menuConfig::accent;
    
    ImVec4 glow_col(
    base.x * 0.55f,
    base.y * 0.55f,
    base.z * 0.55f,
    1.0f
    );
    
    ImVec4 grad_l(
    base.x * 0.75f,
    base.y * 0.75f,
    base.z * 0.75f,
    1.0f
    );
    
    ImVec4 grad_r(
    ImMin(base.x + 0.25f, 1.0f),
    ImMin(base.y + 0.25f, 1.0f),
    ImMin(base.z + 0.25f, 1.0f),
    1.0f
    );
    
    for (int layer = 1; layer <= 10; layer++)
    {
    float radius = layer * 1.1f;
    float alpha = 0.045f * (1.0f - layer / 10.0f);
    ImU32 col = ImGui::GetColorU32(ImVec4(
    glow_col.x,
    glow_col.y,
    glow_col.z,
    alpha
    ));

    for (int x = -1; x <= 1; x++)
    for (int y = -1; y <= 1; y++)
    draw->AddText(
    ImVec2(pos.x + x * radius, pos.y + y * radius),
    col,
    txt
    );
    }
    
    float advance = 0.0f;
    const char* p = txt;
    
    while (*p)
    {
    char c[2] = { *p, 0 };
    ImVec2 cs = ImGui::CalcTextSize(c);

    float t = advance / size.x;
    ImVec4 col = ImLerp(grad_l, grad_r, t);

    draw->AddText(
    ImVec2(pos.x + advance, pos.y),
    ImGui::GetColorU32(col),
    c
    );

    advance += cs.x;
    p++;
    }
    
    ImGui::Dummy(size);
    ImGui::SetWindowFontScale(1.0f);
    
    static int textPageId = 0;
    
    if (pageId == 0) {
    if (textPageId < 0 || textPageId > 1)
    textPageId = 0;
    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 506.0f);

    if (ImGui::TextTab(textPageId == 0, "Esp", 0))
    textPageId = 0;

    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);

    if (ImGui::TextTab(textPageId == 1, "Others", 1))
    textPageId = 1;
    }
    
    if (pageId == 1) {
    if (textPageId < 2 || textPageId > 3)
    textPageId = 2;
    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 401.0f);

    if (ImGui::TextTab(textPageId == 2, "AimBot", 2))
    textPageId = 2;

    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);

    if (ImGui::TextTab(textPageId == 3, "TriggerBot", 3))
    textPageId = 3;
    }
    
    if (pageId == 2) {
    if (textPageId < 4 || textPageId > 6)
    textPageId = 4;
    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 343.0f);

    if (ImGui::TextTab(textPageId == 4, "Guns", 4))
    textPageId = 4;

    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);

    if (ImGui::TextTab(textPageId == 5, "Granades", 5))
    textPageId = 5;
    
    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);

    if (ImGui::TextTab(textPageId == 6, "Bomb", 6))
    textPageId = 6;
    }
    
    if (pageId == 3) {
    if (textPageId < 7 || textPageId > 8)
    textPageId = 7;
    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 491.0f);

    if (ImGui::TextTab(textPageId == 7, "Player", 7))
    textPageId = 7;

    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);

    if (ImGui::TextTab(textPageId == 8, "Props", 8))
    textPageId = 8;
    }
    
    if (pageId == 4) {
    if (textPageId < 9 || textPageId > 10)
    textPageId = 9;
    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 474.0f);

    if (ImGui::TextTab(textPageId == 9, "Game", 9))
    textPageId = 9;

    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);

    if (ImGui::TextTab(textPageId == 10, "Others", 10))
    textPageId = 10;
    }
    
    if (pageId == 5) {
    if (textPageId < 11 || textPageId > 12)
    textPageId = 11;
    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 477.0f);

    if (ImGui::TextTab(textPageId == 11, "Skin", 11))
    textPageId = 11;

    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);

    if (ImGui::TextTab(textPageId == 12, "Markups", 12))
    textPageId = 12;
    }
    
    if (pageId == 6) {
    textPageId = 13;
    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 480.0f);

    if (ImGui::TextTab(textPageId == 13, "Configuration", 13))
    textPageId = 13;
    }
    
    if (pageId == 7) {
    if (textPageId < 14 || textPageId > 15)
    textPageId = 14;
    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 450.0f);

    if (ImGui::TextTab(textPageId == 14, "Menu", 14))
    textPageId = 14;

    ImGui::SameLine();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);

    if (ImGui::TextTab(textPageId == 15, "Recorder", 15))
    textPageId = 15;
    }
    
    ImVec2 menuPosition = ImGui::GetWindowPos();
    
    SetCursorPos(ImVec2(10 * 1.1f, 120 * 1.1f));
    BeginGroup();
    style->ItemSpacing = ImVec2(2, 2);
    EndGroup();
    
    tabAlpha = ImLerp(tabAlpha, (pageId == activeTab) ? 1.0f : 0.0f, 15.0f * GetIO().DeltaTime);
    if (tabAlpha < 0.01f) activeTab = pageId;
    tabAlpha = ImLerp(tabAlpha, (pageId == activeTab) ? 1.0f : 0.0f, 15.0f * GetIO().DeltaTime);
    if (tabAlpha < 0.01f) activeTab = pageId;
    
    SetCursorPos(ImVec2(10 * 1.1f, 75 * 1.1f));
    BeginGroup();
    style->ItemSpacing = ImVec2(2, 2);
    EndGroup();
    
    style->ItemSpacing = ImVec2(6.0f, 0.0f);
    float base_tab_width = (GetContentRegionAvail().x - 27.0f) / 9.0f;
    float tab_width = base_tab_width * 0.98f;
    
    ImGui::CustomTab(pageId == 0, "a", "Visuals", tab_width) && (pageId = 0);
    ImGui::CustomTab(pageId == 1, "b", "Combat", tab_width) && (pageId = 1);
    ImGui::CustomTab(pageId == 2, "c", "Explosives", tab_width) && (pageId = 2);
    ImGui::CustomTab(pageId == 3, "d", "Movement", tab_width) && (pageId = 3);
    ImGui::CustomTab(pageId == 4, "e", "Misc", tab_width) && (pageId = 4);
    ImGui::CustomTab(pageId == 5, "f", "Skinchanger", tab_width) && (pageId = 5);
    ImGui::CustomTab(pageId == 6, "g", "Configuration", tab_width) && (pageId = 6);
    ImGui::CustomTab(pageId == 7, "h", "Settings", tab_width) && (pageId = 7);
    EndGroup();
    
    SetCursorPos(ImVec2(10 * 1.1f, 660 * 1.1f));
    if (ImGui::CustomButton("EXIT CHEAT", ImVec2(275, 55))) {
    exit(0);
    }
    BeginGroup();
    style->ItemSpacing = ImVec2(2, 2);
    EndGroup();
    
    SetCursorPos(ImVec2(10 * 1.1f, 67 * 1.1f));
    BeginGroup();
    style->ItemSpacing = ImVec2(2, 2);
    ImGui::Separator();
    EndGroup();
    
    SetCursorPos(ImVec2(270 * 1.1f, 160 * 1.1f - (tabAlpha * 100)));
    PushStyleVar(ImGuiStyleVar_Alpha, tabAlpha * style->Alpha);
    
    if (textPageId == 0) {
    const float childW = (menuConfig::background::size.x - 390.0f) / 1.64f * 1.1f;
    const float childH = menuConfig::background::size.y + 69.0f;
    ImGui::BeginChild("##ESP_CHILD", ImVec2(childW, childH), true);
    {
    SectionHeader("Esp");
    CustomSwitch("Esp", &enablespi);
    if (enablespi)
    {
    CustomSwitch("Line", &esp.line);
    CustomSwitch("Arrow", &esp.offscreenArrow);
    CustomSwitch("Box", &esp.box);
    if (esp.box)
    {
    CustomSwitch("3D Box", &esp.box3d);
    CustomSwitch("Box Fill", &boxf);
    CustomSwitch("Foto", &esp.foto);
    }
    CustomSwitch("Skeleton", &esp.skeleton);
    CustomSwitch("Health", &esp.hpBar);
    if (esp.hpBar) {
    CustomSwitch("HP Gradient", &esp.hpGradient);
    }
    CustomSwitch("Armor", &esp.armor);
    if (esp.armor) {
    CustomSwitch("Armor Gradient", &esp.armorGradient);
    }
    CustomSwitch("Weapon", &esp.weapon);
    CustomSwitch("Weapon icon", &esp.weaponicon);
    CustomSwitch("Nick", &esp.nickName);
    CustomSwitch("Money", &esp.money);
    CustomSwitch("Distance", &esp.distance);
    CustomSwitch("Ping", &esp.ping);
    CustomSwitch("Pulse Effect", &esp.pulseWave);
    CustomSwitch("Visibility", &esp.isvis);
    }
    }
    EndChild();
    SameLine();
    ImGui::BeginChild("##ESP_SETTINGS", ImVec2(childW, childH), true);
    {
    SectionHeader("Esp settings");
    if (enablespi)
    {
    if (esp.line) {
    CustomSliderFloat("Line glow", &esp.lineGlowStrength, 0.f, 18.f, "%.0f");
    CustomSliderFloat("Line thickness", &esp.espLineThickness, 0.5f, 6.f, "%.1f");
    CustomSliderInt("Snap style", &esp.snaplineStyle, 0, 2);
    CustomSliderInt("End style", &esp.lineEndStyle, 0, 2);
    }
    if (esp.offscreenArrow) {
    CustomColorEdit3("Arrow color", (float*)&esp.offscreenArrowColor);
    }
    if (esp.box) {
    CustomSliderFloat("Box glow", &esp.boxGlowStrength, 0.f, 18.f, "%.0f");
    CustomSliderFloat("Box round", &esp.rounding, 0.f, 20.f);
    CustomSliderInt("Box style", &esp.boxStyle, 0, 1);
    if (esp.boxStyle == 1)
        CustomSliderFloat("Corner len", &esp.cornerLength, 0.05f, 0.45f, "%.2f");
    CustomSliderFloat("Box outline", &esp.boxOutlineThickness, 0.0f, 5.0f, "%.1f");
    }
    if (boxf) {
    CustomSliderFloat("Box fill", &boxfv, 0.f, 100.f);
    }
    if (esp.skeleton) {
    CustomColorEdit3("Skeleton color", esp.skeletonColor);
    CustomSliderFloat("Skel thickness", &esp.skeletonThickness, 0.5f, 5.0f, "%.1f");
    CustomSliderFloat("Skel glow", &esp.skeletonGlow, 0.0f, 8.0f, "%.0f");
    CustomSliderFloat("Skel scale", &esp.skeletonScale, 0.5f, 1.2f, "%.2f");
    CustomSwitch("Skel outline", &esp.skeletonOutline);
    if (esp.skeletonOutline)
        CustomSliderFloat("Outline thick", &esp.skeletonOutlineThickness, 1.0f, 6.0f, "%.1f");
    CustomSwitch("Joint dots", &esp.skeletonJointDots);
    if (esp.skeletonJointDots) {
        CustomSliderFloat("Joint size", &esp.skeletonJointSize, 1.0f, 8.0f, "%.1f");
        CustomColorEdit3("Joint color", esp.skeletonJointColor);
        CustomSliderFloat("Joint glow", &esp.skeletonJointGlow, 0.0f, 8.0f, "%.1f");
    }
    CustomSliderInt("Head circle", &esp.headCircleStyle, 0, 2);
    CustomSliderFloat("Head size", &esp.headCircleSize, 2.0f, 15.0f, "%.1f");
    }
    if (esp.hpBar) {
        CustomSliderFloat("HP glow", &esp.hpGlowStrength, 0.f, 16.f);
        CustomSliderFloat("HP thickness", &esp.hpbarscaless, 0.f, 10.f);
        CustomSliderFloat("HP rounding", &esp.hpBarRounding, 0.0f, 8.0f, "%.1f");
        CustomSwitch("Adaptive HP", &esp.hpAdaptiveColor);
        if (!esp.hpAdaptiveColor) {
        if (!esp.hpGradient)
        CustomColorEdit3("HP color", (float*)&esp.hpColor);
        else {
        CustomColorEdit3("HP color 1", (float*)&esp.hpColor1);
        CustomColorEdit3("HP color 2", (float*)&esp.hpColor2);
        }
        }
    }
    if (esp.armor) {
        CustomSliderFloat("Armor glow", &esp.armorGlowStrength, 0.f, 15.f);
        CustomSliderFloat("Armor thickness", &esp.armorThickness, 0.f, 10.f);
        CustomSliderFloat("Armor rounding", &esp.armorBarRounding, 0.0f, 8.0f, "%.1f");
        CustomSwitch("Adaptive armor", &esp.armorAdaptiveColor);
        if (!esp.armorAdaptiveColor) {
        if (!esp.armorGradient)
        CustomColorEdit3("Armor color", (float*)&esp.armorColor);
        else {
        CustomColorEdit3("Armor color 1", (float*)&esp.armorColor1);
        CustomColorEdit3("Armor color 2", (float*)&esp.armorColor2);
        }
        }
    }
    if (esp.weapon) {
    CustomColorEdit3("Weapon Color", esp.weaponColor);
    }
    if (esp.nickName){
    CustomSliderFloat("Nick glow", &esp.nameGlowStrength, 0.0f, 15.0f);
    CustomColorEdit3("Nick color", esp.nickNameColor);
    CustomSliderFloat("Nick scale", &esp.nameScale, 0.4f, 1.5f, "%.2f");
    CustomSwitch("Nick outline", &esp.nameOutline);
    if (esp.nameOutline)
        CustomColorEdit3("Outline color", esp.nameOutlineColor);
    }
    if (esp.money){
    CustomColorEdit3("Money Color", esp.moneyColor);
    }
    if (esp.distance){
    CustomColorEdit3("Distance Color", esp.distanceColor);
    }
    if (esp.ping){
    CustomColorEdit3("Ping Color", esp.pingColor);
    }    
    if (esp.pulseWave) {
    CustomColorEdit3("Pulse Color", esp.pulseColor);
    CustomSliderFloat("Pulse Speed", &esp.pulseSpeed, 1.0f, 15.0f);
    }
    if (esp.isvis)
    {
        CustomColorEdit3("Visible", esp.espVColor);
        CustomColorEdit3("Invisible", esp.espIColor);
    }
    }
    }
    EndChild();
    }
    else
    if (textPageId == 1) {
    textPageId = 1;
    const float childW = (menuConfig::background::size.x - 390.0f) / 1.64f * 1.1f;
    const float childH = menuConfig::background::size.y + 69.0f;
    ImGui::BeginChild("##OTHERS_CHILD", ImVec2(childW, childH), true);
    {
    SectionHeader("Others");
    CustomSwitch("Bullet trace", &esp.bullettrace);
    CustomSwitch("Sky Color", &sky);
    CustomSwitch("Camera Fov", &camerafov);
    CustomSwitch("Aspect Ratio", &aspect);
    CustomSwitch("Head Scale", &head_scale);
    CustomSwitch("Player Scale", &player_scale);
    CustomSwitch("Hitlogs", &esp.hitlogs);
        CustomSwitch("Hand Position", &arms_position);
        CustomSwitch("Third Person", &thirdperson);
    }
    EndChild();
    SameLine();
    ImGui::BeginChild("##OTHERS_SETTINGS", ImVec2(childW, childH), true);
    {
    SectionHeader("Ohers settings");
    if (esp.bullettrace) {
    CustomColorEdit3("Bullet color", esp.bullettraceColor);
    CustomColorEdit3("Glow color", esp.bullettraceGlowColor);
    CustomSliderInt("Trace style", &esp.bullettraceStyle, 0, 2);
    CustomSliderInt("Hit style", &esp.bullettraceHitStyle, 0, 2);
    CustomSwitch("From crosshair", &esp.bullettraceFromCrosshair);
    CustomSliderFloat("Trace time", &esp.bullettraceTime, 0.3f, 5.0f, "%.1f");
    CustomSliderFloat("Trace thick", &esp.bullettraceThickness, 0.5f, 6.0f, "%.1f");
    CustomSliderFloat("Hit size", &esp.bullettraceHitSize, 1.0f, 14.0f, "%.1f");
    CustomSliderFloat("Glow", &esp.bullettraceGlow, 0.0f, 12.0f, "%.0f");
    if (esp.bullettraceStyle == 1)
        CustomSwitch("Neon core", &esp.bullettraceNeonCore);
    }
    if (sky){
    CustomColorEdit3("Sky Color", sky_color);
    }
    if (camerafov){
    CustomSliderFloat("Fov Value", &fov_value, 60.0f, 150.0f);
    }
    if (aspect){
    CustomSliderFloat("Fov Value", &aspect_value, 0.5f, 2.5f);           
        }
        if (head_scale) {
        CustomSliderFloat("Sacle head", &headScaleValue, -10.0f, 10.0f);
        }       
        if (player_scale) {
        CustomSliderFloat("Player scale", &playerscaleval, -2.0f, 1.3f);
        }
        if (esp.hitlogs){
        CustomColorEdit3("Hit color", esp.hitlogColor);
        }
        if (thirdperson) {
        CustomSliderFloat("Camera Distance", &thirdperson_distance, 0.5f, 8.0f, "%.1f");
        }
        if (arms_position) {
        CustomSliderFloat("Hand Pos X", &ARMSX, -10.0f, 10.0f);
        CustomSliderFloat("Hand Pos Y", &ARMSY, -10.0f, 10.0f);
        CustomSliderFloat("Hand Pos Z", &ARMSZ, -10.0f, 10.0f);
        }
    }
    EndChild();
    }
    else
    if (textPageId == 2) {
    const float childW = (menuConfig::background::size.x - 390.0f) / 1.64f * 1.1f;
    const float childH = menuConfig::background::size.y + 69.0f;
    BeginChild("##AIM_CHILD", ImVec2(childW, childH), true);
    {
    SectionHeader("Aimbot");
    CustomSwitch("Enable", &aimbot.enable);
    CustomSwitch("Silent HvH", &silentaim);
    CustomSwitch("Silent autofire", &silent_autofire);
    CustomSwitch("Fov fill", &aimbot.fovFillEnabled);
    CustomSwitch("Visible check", &aimbot.visibleCheck);
    }
    EndChild();
    SameLine();
    ImGui::BeginChild("##AIM_SETTINGS", ImVec2(childW, childH), true);
    {
    SectionHeader("Aimbot settings");
    if (silentaim) {
    CustomCombo("Silent mode", &silent_field, silentFieldItems, IM_ARRAYSIZE(silentFieldItems));
    CustomSliderFloat("Silent Radius", &aimbot.fovRadius, 20.0f, 360.0f);
    CustomCombo("Silent Bone", &aimbot.aimTarget, aimTargetItems, IM_ARRAYSIZE(aimTargetItems));
    }
    if (aimbot.enable) {
    CustomSliderFloat("Radius", &aimbot.fovRadius, 20.0f, 360.0f);
    CustomCombo("Bone", &aimbot.aimTarget, aimTargetItems, IM_ARRAYSIZE(aimTargetItems));
        CustomSliderFloat("Smoothing", &aimbot.smoothing, 0.01f, 1.0f, "%.2f");
        if (!aimbot.visibleCheck)
        CustomColorEdit3("FOV Visible", aimbot.fovVisibleColor);
        else {
        CustomColorEdit3("FOV Visible", aimbot.fovVisibleColor);
        CustomColorEdit3("FOV Invisible", aimbot.fovInvisibleColor);
        }
        if (aimbot.fovFillEnabled) {
        CustomSliderFloat("Fov fill", &aimbot.fovFillOpacity, 0.01f, 1.0f);
        }
    }
    }
    EndChild();
    }
    else
    if (textPageId == 3) {
    const float childW = (menuConfig::background::size.x - 390.0f) / 1.64f * 1.1f;
    const float childH = menuConfig::background::size.y + 69.0f;
    BeginChild("##TRIGGER", ImVec2(childW, childH), true);
    {
    SectionHeader("Trigger bot");
    CustomSwitch("Trigger bot", &tg);
    } 
    EndChild();
    SameLine();
    ImGui::BeginChild("##TRIGGER_SETTINGS", ImVec2(childW, childH), true);
    {
    SectionHeader("Trigger settings");
        if (tg) {
        CustomSliderFloat("Trigger Delay", &aimbot.triggerDelay, 0.01f, 1.0f, "%.2f sec");
        }
    }
    EndChild();
    }
    else
        if (textPageId == 4) {
    const float childW = (menuConfig::background::size.x - 403.0f) / 1.64f * 1.1f;
    const float childH = menuConfig::background::size.y + 69.0f;

    ImGui::BeginChild("##WEAPON_CHILD", ImVec2(childW * 2.05f, childH), true);
    {
    SectionHeader("Guns");
    CustomSwitch("Reverse ammo", &weapon.reversammo);
        CustomSwitch("Infinity ammo", &weapon.infinityammo);
        CustomSwitch("Wallshoot", &weapon.wallshoot);
        CustomSwitch("No recoil", &weapon.norecoil);
        CustomSwitch("Fire rate", &weapon.firerate);
        CustomSwitch("One hit kill", &weapon.onehitkill);
    CustomSwitch("Fast knife", &weapon.fastknife);
    }
    ImGui::EndChild();
    }
        else
        if (textPageId == 5) {
    const float childW = (menuConfig::background::size.x - 403.0f) / 1.64f * 1.1f;
    const float childH = menuConfig::background::size.y + 69.0f;

    ImGui::BeginChild("##GRANADE_CHILD", ImVec2(childW * 2.05f, childH), true);
    {
    SectionHeader("Granades");
    CustomSwitch("Fast detonate", &gdetonate);
    CustomSwitch("No damage", &gnodamage);
    CustomSwitch("Infinity safe", &gUnlimitedGrenades);
    CustomSwitch("Damage hack", &dmgh);
    }
    ImGui::EndChild();
    }
        else
        if (textPageId == 6) {
    const float childW = (menuConfig::background::size.x - 403.0f) / 1.64f * 1.1f;
    const float childH = menuConfig::background::size.y + 69.0f;

    ImGui::BeginChild("##BOMB_CHILD", ImVec2(childW * 2.05f, childH), true);
    {
    SectionHeader("Bomb");
    CustomSwitch("Explos", &bexplos);
    CustomSwitch("Fast plant", &bplant);
    CustomSwitch("Impulse hack", &bimpulse);
    CustomSwitch("Radius hack", &bradius);
    CustomSwitch("Damage hack", &bdamage);
    }
    ImGui::EndChild();
    }
    else
    if (textPageId == 7) {
    const float childW = (menuConfig::background::size.x - 390.0f) / 1.64f * 1.1f;
    const float childH = menuConfig::background::size.y + 69.0f;
    BeginChild("##PLAYER_CHILD", ImVec2(childW, childH), true);
    {
    SectionHeader("Player");
    CustomSwitch("Speedhack", &speedhack);
    CustomSwitch("Crouch speed", &crouch);
    CustomSwitch("Bunny Hope", &bunnyhope);
    CustomSwitch("Air strafe", &airstafe);
    CustomSwitch("Strafe", &strafe);
    CustomSwitch("Air jump", &airjump);
    CustomSwitch("God mode", &sethl9998);
    CustomSwitch("Invismode", &invismode);
    CustomSwitch("Anti afk", &aafk);
    CustomSwitch("High jump", &high_jump); 
    CustomSwitch("Anti-aim", &antiaim);
        CustomSwitch("Skiptable", &skiptable);
        CustomSwitch("Enable clumsy", &clumsy);
    }
    EndChild();
        SameLine();
        ImGui::BeginChild("##PLAYER_SETTINGS", ImVec2(childW, childH), true);
    {
    SectionHeader("Settings");
        if (bunnyhope){
    CustomSliderFloat("BunnyHope Value", &hope_value, 0.0f, 10.0f);
    }
    if (strafe) {
    CustomSliderFloat("Strafe Speed", &strafe_speed, 0.1f, 5.0f, "%.1f");
    }
    if (high_jump){
    ImGui::CustomSliderFloat("High Value", &high_value, 5.0f, 50.0f);
    }
    if (antiaim){
    CustomCombo("AA Yaw", &aa_yaw_mode, aaYawItems, IM_ARRAYSIZE(aaYawItems));
    if (aa_yaw_mode == 1 || aa_yaw_mode == 3)
        CustomSliderFloat("Yaw Value", &aa_yaw_value, 0.0f, 180.0f, "%.0f");
    if (aa_yaw_mode == 2)
        CustomSliderFloat("Spin Speed", &aa_spin_speed, 1.0f, 60.0f, "%.0f");
    CustomCombo("AA Pitch", &aa_pitch_mode, aaPitchItems, IM_ARRAYSIZE(aaPitchItems));
    if (aa_pitch_mode == 1 || aa_pitch_mode == 2)
        CustomSliderFloat("Pitch Value", &aa_pitch_value, 0.0f, 89.0f, "%.0f");
    CustomSwitch("AA Rotate Body", &aa_body);
    }
    if (clumsy) {
    if (ImGui::CustomButton(islag ? "UnLag" : "Lag")) {
    islag = !islag;
    }
    }
        }
        EndChild();
        }
        else
        if (textPageId == 8) {
    const float childW = (menuConfig::background::size.x - 390.0f) / 1.64f * 1.1f;
    const float childH = menuConfig::background::size.y + 69.0f;
    BeginChild("##PROPS_CILD", ImVec2(childW, childH), true);
    {
    SectionHeader("Props");
    CustomSwitch("Set Score", &set_score);
    CustomSwitch("Set Score all", &set_score_all);
    CustomSwitch("Set money all", &set_money_all);
        CustomSwitch("Set Death", &setdeath);
        CustomSwitch("Set Death all", &set_death_all);
        CustomSwitch("Set assists", &setassists);
        CustomSwitch("Set kills", &setkills);
        CustomSwitch("Set kills all", &set_kills_all);
        CustomSwitch("MVP", &set_mvp);
        CustomSwitch("Set ping", &set_ping);
        CustomSwitch("Set ping all", &set_ping_all);
        CustomSwitch("Hide id", &spoofers.hideid);
        CustomSwitch("Hide id all", &hide_id_all);
        CustomSwitch("Hide clan", &spoofers.hideclantag);
        CustomSwitch("Hide clan all", &hide_clan_tag_all);
        CustomSwitch("Fake frame", &set_avatar);
        CustomSwitch("Fake frame all", &fake_avatar_all);
        CustomSwitch("Fake medal", &set_medalsc);
        CustomSwitch("Fake medal all", &fake_medal_all);
    }
    EndChild();
        SameLine();
        ImGui::BeginChild("##PROPS_SETTINGS", ImVec2(childW, childH), true);
    {
    SectionHeader("Settings");
    if (set_money_all) {
    CustomSliderInt("Money all", &money_all_v, 1, 10000);
    }
        if (set_score) {        
        CustomSliderInt("Score", &score_value, 1, 999);
        }
        if (set_score_all) {
        CustomSliderInt("Score all", &score_all_v, 1, 999);
        }
        if (setdeath) {
        CustomSliderInt("Death", &deathval, 1, 10000);
        }
        if (set_death_all) {
        CustomSliderInt("Death all", &death_all_v, 1, 10000);
        }
        if (setassists) {
        CustomSliderInt("Assist", &assistsval, 1, 10000);
        }
        if (setkills) {
        CustomSliderInt("Kills", &killsval, 1, 10000);
        }
        if (set_kills_all) {
        CustomSliderInt("Kills all", &kills_all_v, 1, 10000);
        }
        if (set_ping) {
        CustomSliderInt("Ping", &pingval, 1, 999);
        }
        if (set_ping_all) {
        CustomSliderInt("Ping all", &ping_all_vl, 1, 999);
        }
        }
        EndChild();
        }
        else
        if (textPageId == 9) {
    const float childW = (menuConfig::background::size.x - 403.0f) / 1.64f * 1.1f;
    const float childH = menuConfig::background::size.y + 69.0f;
    ImGui::BeginChild("##GAME_CHILD", ImVec2(childW * 2.05f, childH), true);
    {
    SectionHeader("Game");
    CustomSwitch("Move before timer", &mbf);
    CustomSwitch("Cencel match", &cencmatchmm);
    CustomSwitch("Skip warmup", &skip_warmup);
        CustomSwitch("Autowin", &autowin);
        CustomSwitch("Autowin ct", &autowin_ct);
        CustomSwitch("Autowin tr", &autowin_tr);
        CustomSwitch("Kick players", &kickplayers);
        bool current_host = is_host();
    if (current_host)
    {
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "YOU ARE HOST (Master Client)");
    }
    else
    {
    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "NOT HOST");
    }
    }
    ImGui::EndChild();
    }
        else
        if (textPageId == 10) {
    const float childW = (menuConfig::background::size.x - 403.0f) / 1.64f * 1.1f;
    const float childH = menuConfig::background::size.y + 69.0f;
    ImGui::BeginChild("##OTHERS_CHILD", ImVec2(childW * 2.05f, childH), true);
    {
    SectionHeader("Others");
        CustomSwitch("Set ct team", &force_team_ct);
        CustomSwitch("Set tr team", &force_team_tr);
        CustomSwitch("Money hack", &moneyhack_a);
        CustomSwitch("Infinity buy time", &infinity_buy_a);
        CustomSwitch("Buy anywhere", &buy_anywhere_a);
    }
    ImGui::EndChild();
    }
        else
        if (textPageId == 11) {
    const float childW = (menuConfig::background::size.x - 390.0f) / 1.64f * 1.1f;
    const float childH = menuConfig::background::size.y + 69.0f;
    BeginChild("##SKINCHANGER", ImVec2(childW, childH), true);
    {
    SectionHeader("Skinchanger");
    MenuText("Knife");
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 10.0f));
    CustomSwitch("Set m9", &setm9s);
    CustomSwitch("Set karambit", &kerambitset);
    CustomSwitch("Set jCommando", &jcomandoset);
    CustomSwitch("Set butterfly", &butterflyset);
    CustomSwitch("Set flip knife", &flipset);
    CustomSwitch("Set kunai", &kunaiset);
    CustomSwitch("Set scorpion", &scorpionset);
    CustomSwitch("Set tanto", &tantoset);
    CustomSwitch("Set dugger", &duggerset);
    CustomSwitch("Set kukri", &kukriset);
    CustomSwitch("Set stilletto", &stilletoset);
    CustomSwitch("Set mantis", &mantisset);
    CustomSwitch("Set fang", &fangset);
    CustomSwitch("Set sting", &stingset);
    ImGui::Separator();
    MenuText("Glove");
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 10.0f));
    CustomSwitch("Set glove", &gloveset);
    ImGui::Separator();
    MenuText("Weapon");
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 10.0f));    
    CustomSwitch("Set G22", &g22set);
    CustomSwitch("Set USP", &uspset);
    CustomSwitch("Set P350", &p350set);
    CustomSwitch("Set berettas", &beretasset);
    CustomSwitch("Set tec-9", &tec9set);
    CustomSwitch("Set FiveSeven", &fivesevenset);
    CustomSwitch("Set Deagle", &deagleset);
    CustomSwitch("Set UMP45", &ump45set);
    CustomSwitch("Set MP7", &mp7set);
    CustomSwitch("Set P90", &p90set);
    CustomSwitch("Set MP5", &mp5set);
    CustomSwitch("Set AKR", &akrset);
    CustomSwitch("Set AKR12", &akr12set);
    CustomSwitch("Set M4", &m4set);
    CustomSwitch("Set M4A1", &m4a1set);
    CustomSwitch("Set M16", &m16set);
    CustomSwitch("Set FAMAS", &famasset);
    CustomSwitch("Set FnFal", &fnfalset);
    CustomSwitch("Set AWM", &awmset);
    CustomSwitch("Set M40", &m40set);
    CustomSwitch("Set M110", &m110set);
    CustomSwitch("Set Mallard", &mallardset);
    CustomSwitch("Set SM1014", &sm1014set);
    CustomSwitch("Set FabM", &fabmset);
    CustomSwitch("Set M60", &m60set);
    CustomSwitch("Set VAL", &valset);
    CustomSwitch("Set MAC10", &mac10set);
    CustomSwitch("Set SPAS", &spasset);
    CustomSwitch("Set AkimboUzi", &akimbouziset);      
    ImGui::Separator();
    MenuText("Granade");
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 10.0f));
    CustomSwitch("Set grenade", &grenadeset);
    ImGui::Separator();
    MenuText("Other");
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 10.0f));
    CustomSwitch("Set medal", &medalset);
    CustomSwitch("Set avatar frame", &avatarset);
    }
    EndChild();
        SameLine();
        ImGui::BeginChild("##SKINS", ImVec2(childW, childH), true);
    {
    SectionHeader("Set skin");
    ImGui::Dummy(ImVec2(0, 20.0f));
    if(setm9s){
    MenuText("M9");
    if (ImGui::CustomButton("Blue Blood")) {ChangeSkin(71001);}
    if (ImGui::CustomButton("Ancient")) {ChangeSkin(71002);}
    if (ImGui::CustomButton("Scratch")) {ChangeSkin(71003);}
    if (ImGui::CustomButton("Universe")) {ChangeSkin(71004);}
    if (ImGui::CustomButton("Dragon Glass")) {ChangeSkin(71005);}
    if (ImGui::CustomButton("Frozen")) {ChangeSkin(97100);}
    if (ImGui::CustomButton("Kumo")) {ChangeSkin(157100);}
    if (ImGui::CustomButton("Digital Burst")) {ChangeSkin(200012);}
    if (ImGui::CustomButton("Dark Shiver")) {ChangeSkin(240063);}
    if (ImGui::CustomButton("Amber Koi")) {ChangeSkin(240154);}
    if (ImGui::CustomButton("Moonstone")) {ChangeSkin(240228);}
    if (ImGui::CustomButton("M9 Bayonet Sketch")) { ChangeSkin(240293); }
    if (ImGui::CustomButton("M9 Bayonet Citrine")) { ChangeSkin(240294); }
    if (ImGui::CustomButton("M9 Bayonet Poison")) { ChangeSkin(240295); }
    if (ImGui::CustomButton("M9 Bayonet Twilight")) { ChangeSkin(240296); }
    if (ImGui::CustomButton("agent")) { ChangeSkin(1660); }
    }
    if (kerambitset){
    MenuText("Karambit");
    if (ImGui::CustomButton("Claw")) {ChangeSkin(72002);}
    if (ImGui::CustomButton("Gold")) {ChangeSkin(72003);}
    if (ImGui::CustomButton("IceDragon")) {ChangeSkin(72004);}
    if (ImGui::CustomButton("Scratch")) {ChangeSkin(72006);}
    if (ImGui::CustomButton("Universe")) {ChangeSkin(72007);}
    if (ImGui::CustomButton("Cold Flame")) {ChangeSkin(97200);}
    if (ImGui::CustomButton("Frozen")) {ChangeSkin(97201);}
    if (ImGui::CustomButton("Snow Camo")) {ChangeSkin(97203);}
    if (ImGui::CustomButton("YearOfTheTiger")) {ChangeSkin(157200);}
    if (ImGui::CustomButton("Purple Camo")) {ChangeSkin(200011);}
    if (ImGui::CustomButton("Nebula")) {ChangeSkin(240014);}
    if (ImGui::CustomButton("Pelagia")) {ChangeSkin(240111);}
    if (ImGui::CustomButton("Widow's Weave")) {ChangeSkin(240131);}
    if (ImGui::CustomButton("Vermilion")) {ChangeSkin(240164);}
    if (ImGui::CustomButton("Hologram")) { ChangeSkin(240290); }
    }
    if (jcomandoset){
    MenuText("jKommando");
    if (ImGui::CustomButton("Ancient")) {ChangeSkin(73002);}
    if (ImGui::CustomButton("Reaper")) {ChangeSkin(73003);}
    if (ImGui::CustomButton("Floral")) {ChangeSkin(73004);}
    if (ImGui::CustomButton("Luxury")) {ChangeSkin(73006);}
    if (ImGui::CustomButton("Frozen")) {ChangeSkin(97300);}
    if (ImGui::CustomButton("Prism")) {ChangeSkin(200015);}
    if (ImGui::CustomButton("Augustite")) {ChangeSkin(157300);}
    if (ImGui::CustomButton("Shatter")) {ChangeSkin(240056);}
    if (ImGui::CustomButton("Plasma")) {ChangeSkin(240132);}
    }
    if (butterflyset){
    MenuText("Butterfly");
    if (ImGui::CustomButton("Gold")) {ChangeSkin(47502);}
    if (ImGui::CustomButton("Dragon Glass")) {ChangeSkin(47503);}
    if (ImGui::CustomButton("Red")) {ChangeSkin(47504);}
    if (ImGui::CustomButton("Starfall")) {ChangeSkin(47505);}
    if (ImGui::CustomButton("New1")) {ChangeSkin(57501);}
    if (ImGui::CustomButton("Cold Flame")) {ChangeSkin(97500);}
    if (ImGui::CustomButton("Kumo")) {ChangeSkin(157500);}
    if (ImGui::CustomButton("Glitch")) {ChangeSkin(240007);}
    if (ImGui::CustomButton("Saphira")) {ChangeSkin(240059);}
    if (ImGui::CustomButton("Ripple")) {ChangeSkin(240130);}
    if (ImGui::CustomButton("Gambit")) {ChangeSkin(240225);}
    if (ImGui::CustomButton("Wild West")) { ChangeSkin(240319); }
    if (ImGui::CustomButton("JadeStone")) { ChangeSkin(240331); }
    }
    if (flipset){
    MenuText("FlipKnife");
    if (ImGui::CustomButton("New1")) {ChangeSkin(67701);}
    if (ImGui::CustomButton("New2")) {ChangeSkin(67702);}
    if (ImGui::CustomButton("New3")) {ChangeSkin(67703);}
    if (ImGui::CustomButton("New4")) {ChangeSkin(67704);}
    if (ImGui::CustomButton("New5")) {ChangeSkin(67705);}
    if (ImGui::CustomButton("Snow Camo")) {ChangeSkin(97700);}
    if (ImGui::CustomButton("Holiday Frost")) {ChangeSkin(157700);}
    if (ImGui::CustomButton("Magnalium")) {ChangeSkin(240011);}
    }
    if (kunaiset){
    MenuText("Kunai");
    if (ImGui::CustomButton("Bone")) {ChangeSkin(77813);}
    if (ImGui::CustomButton("Luxury")) {ChangeSkin(77814);}
    if (ImGui::CustomButton("Poison")) {ChangeSkin(77815);}
    if (ImGui::CustomButton("Radiation")) {ChangeSkin(77816);}
    if (ImGui::CustomButton("Reaper")) {ChangeSkin(77817);}
    if (ImGui::CustomButton("Cold Flame")) {ChangeSkin(97800);}
    if (ImGui::CustomButton("Snow Camo")) {ChangeSkin(97801);}
    if (ImGui::CustomButton("Augustite")) {ChangeSkin(157800);}
    if (ImGui::CustomButton("Prism")) {ChangeSkin(200014);}
    if (ImGui::CustomButton("Glitch")) {ChangeSkin(240008);}
    if (ImGui::CustomButton("Wavesong")) {ChangeSkin(240156);}
    if (ImGui::CustomButton("Diffusion")) {ChangeSkin(240231);}
    if (ImGui::CustomButton("Fire Horse")) { ChangeSkin(240318); }
    }
    if (scorpionset){
    MenuText("Scorpion");
    if (ImGui::CustomButton("Camouflage")) {ChangeSkin(87919);}
    if (ImGui::CustomButton("Green")) {ChangeSkin(87920);}
    if (ImGui::CustomButton("Scratch")) {ChangeSkin(87921);}
    if (ImGui::CustomButton("Sky")) {ChangeSkin(87922);}
    if (ImGui::CustomButton("Cold Flame")) {ChangeSkin(97900);}
    if (ImGui::CustomButton("Holiday Frost")) {ChangeSkin(157900);}
    if (ImGui::CustomButton("Magnalium")) {ChangeSkin(240010);}
    if (ImGui::CustomButton("Ahau")) {ChangeSkin(240112);}
    if (ImGui::CustomButton("Neko Joy")) {ChangeSkin(240152);}
    }
    if (tantoset){
    MenuText("Tanto");
    if (ImGui::CustomButton("Dojo")) {ChangeSkin(138000);}
    if (ImGui::CustomButton("Mafia")) {ChangeSkin(138001);}
    if (ImGui::CustomButton("Malachite")) {ChangeSkin(138002);}
    if (ImGui::CustomButton("Pearl Abyss")) {ChangeSkin(138003);}
    if (ImGui::CustomButton("Transistor")) {ChangeSkin(138004);}
    if (ImGui::CustomButton("Flow")) {ChangeSkin(138005);}
    if (ImGui::CustomButton("Restless")) {ChangeSkin(148000);}
    if (ImGui::CustomButton("Year of the Tiger")) {ChangeSkin(158000);}
    if (ImGui::CustomButton("Retro Arcade")) {ChangeSkin(200016);}
    if (ImGui::CustomButton("Glitch")) {ChangeSkin(240009);}
    if (ImGui::CustomButton("Frost Nova")) {ChangeSkin(240061);}
    if (ImGui::CustomButton("Solstice")) {ChangeSkin(240114);}
    if (ImGui::CustomButton("Hanami")) {ChangeSkin(240151);}
    if (ImGui::CustomButton("CelestialTiger")) {ChangeSkin(240226);}
    }
    if (duggerset){
    MenuText("Dagger");
    if (ImGui::CustomButton("Acid")) {ChangeSkin(170021);}
    if (ImGui::CustomButton("Demonic Steel")) {ChangeSkin(170022);}
    if (ImGui::CustomButton("Grunge")) {ChangeSkin(170023);}
    if (ImGui::CustomButton("Molten")) {ChangeSkin(170024);}
    if (ImGui::CustomButton("Harmony")) {ChangeSkin(180000);}
    if (ImGui::CustomButton("Jaws")) {ChangeSkin(180028);}
    if (ImGui::CustomButton("Retro Arcade")) {ChangeSkin(200017);}
    if (ImGui::CustomButton("Frost Nova")) {ChangeSkin(240060);}
    if (ImGui::CustomButton("Tiki")) {ChangeSkin(240109);}
    if (ImGui::CustomButton("Contagion")) {ChangeSkin(240129);}
    }
    if (kukriset){
    MenuText("Kukri");
    if (ImGui::CustomButton("Ares")) {ChangeSkin(180022);}
    if (ImGui::CustomButton("Eagle")) {ChangeSkin(180023);}
    if (ImGui::CustomButton("Gold")) {ChangeSkin(180024);}
    if (ImGui::CustomButton("Silver")) {ChangeSkin(180025);}
    if (ImGui::CustomButton("Stars")) {ChangeSkin(180026);}
    if (ImGui::CustomButton("Stone")) {ChangeSkin(180027);}
    if (ImGui::CustomButton("Digital Burst")) {ChangeSkin(200013);}
    if (ImGui::CustomButton("Sea Glint")) {ChangeSkin(240110);}
    if (ImGui::CustomButton("Decay")) {ChangeSkin(240128);}
    if (ImGui::CustomButton("Cascade")) {ChangeSkin(240227);}
    if (ImGui::CustomButton("Pixel Glass")) { ChangeSkin(240320); }
    }
    if (stilletoset){
    MenuText("Stiletto");
    if (ImGui::CustomButton("Soul Devourer")) {ChangeSkin(220021);}
    if (ImGui::CustomButton("Viper")) {ChangeSkin(220022);}
    if (ImGui::CustomButton("Tie Dye")) {ChangeSkin(220023);}
    if (ImGui::CustomButton("Damascus")) {ChangeSkin(220024);}
    if (ImGui::CustomButton("Flux")) {ChangeSkin(240012);}
    if (ImGui::CustomButton("Dark Shiver")) {ChangeSkin(240062);}
    if (ImGui::CustomButton("Voidroot")) {ChangeSkin(240153);}
    if (ImGui::CustomButton("Ascend")) {ChangeSkin(240230);}
    if (ImGui::CustomButton("Empyrean")) { ChangeSkin(240322); }
    }
    if (mantisset){
    MenuText("Mantis");
    if (ImGui::CustomButton("Eclipse")) {ChangeSkin(240198);}
    if (ImGui::CustomButton("Citrine")) {ChangeSkin(240199);}
    if (ImGui::CustomButton("Nest")) {ChangeSkin(240200);}
    if (ImGui::CustomButton("Genesis")) {ChangeSkin(240201);}
    if (ImGui::CustomButton("Impact")) {ChangeSkin(240211);}
    if (ImGui::CustomButton("Yokai")) {ChangeSkin(240229);}
    if (ImGui::CustomButton("Glacier")) { ChangeSkin(240317); }
    if (ImGui::CustomButton("Ink_1")) { ChangeSkin(10240327); }
    if (ImGui::CustomButton("Ink_2")) { ChangeSkin(20240327); }
    if (ImGui::CustomButton("Ink_3")) { ChangeSkin(30240327); }
    }
    if (fangset){
    MenuText("Fang");
    if (ImGui::CustomButton("Flare")) {ChangeSkin(240029);}
    if (ImGui::CustomButton("Serpent")) {ChangeSkin(240030);}
    if (ImGui::CustomButton("Obsidian")) {ChangeSkin(240031);}
    if (ImGui::CustomButton("Relic")) {ChangeSkin(240032);}
    if (ImGui::CustomButton("Aureate")) {ChangeSkin(240033);}
    if (ImGui::CustomButton("Damascus")) {ChangeSkin(240034);}
    if (ImGui::CustomButton("Haunt")) {ChangeSkin(240045);}
    if (ImGui::CustomButton("Saphira")) {ChangeSkin(240058);}
    if (ImGui::CustomButton("Wavesong")) {ChangeSkin(240155);}
    if (ImGui::CustomButton("Ice Storm")) { ChangeSkin(240321); }
    }
    if (stingset){
    MenuText("Sting");
    if (ImGui::CustomButton("Mimicry")) {ChangeSkin(240064);}
    if (ImGui::CustomButton("Meld")) {ChangeSkin(240065);}
    if (ImGui::CustomButton("Corrode")) {ChangeSkin(240066);}
    if (ImGui::CustomButton("Fleck")) {ChangeSkin(240067);}
    if (ImGui::CustomButton("Shroud")) {ChangeSkin(240068);}
    if (ImGui::CustomButton("Oil Sheen")) {ChangeSkin(240069);}
    if (ImGui::CustomButton("Temper Shades")) {ChangeSkin(240070);}
    if (ImGui::CustomButton("Pelagia")) {ChangeSkin(240113);}
    if (ImGui::CustomButton("Arcane Surge")) {ChangeSkin(240133);}
    }
    if (gloveset){
    MenuText("Gloves");
    if (ImGui::CustomButton("Phoenix")) {ChangeSkin(3000);}
    if (ImGui::CustomButton("Autumn")) {ChangeSkin(3001);}
    if (ImGui::CustomButton("Geometric")) {ChangeSkin(3002);}
    if (ImGui::CustomButton("RetroWave")) {ChangeSkin(3003);}
    if (ImGui::CustomButton("LivingFlame")) {ChangeSkin(3004);}
    if (ImGui::CustomButton("Neuro")) {ChangeSkin(3005);}
    if (ImGui::CustomButton("BurningFists")) {ChangeSkin(3006);}
    if (ImGui::CustomButton("Pun")) {ChangeSkin(3007);}
    if (ImGui::CustomButton("Champion")) {ChangeSkin(3008);}
    if (ImGui::CustomButton("SteamRider")) {ChangeSkin(3009);}
    if (ImGui::CustomButton("YearOfTheTiger")) {ChangeSkin(3010);}
    if (ImGui::CustomButton("Acid")) {ChangeSkin(3011);}
    if (ImGui::CustomButton("Camo")) {ChangeSkin(3012);}
    if (ImGui::CustomButton("Fossil")) {ChangeSkin(3013);}
    if (ImGui::CustomButton("Handcraft")) {ChangeSkin(3014);}
    if (ImGui::CustomButton("Raider")) {ChangeSkin(3015);}
    if (ImGui::CustomButton("Thug")) {ChangeSkin(3016);}
    if (ImGui::CustomButton("Fireborn1")) {ChangeSkin(3017);}
    if (ImGui::CustomButton("Fireborn2")) {ChangeSkin(3018);}
    if (ImGui::CustomButton("Fireborn3")) {ChangeSkin(3019);}
    if (ImGui::CustomButton("Fireborn4")) {ChangeSkin(3020);}
    if (ImGui::CustomButton("Fireborn5")) {ChangeSkin(3021);}
    if (ImGui::CustomButton("Fireborn6")) {ChangeSkin(3022);}
    if (ImGui::CustomButton("Flux")) {ChangeSkin(3023);}
    if (ImGui::CustomButton("Shatter")) {ChangeSkin(3024);}
    if (ImGui::CustomButton("Haunt")) {ChangeSkin(3025);}
    if (ImGui::CustomButton("Plague")) {ChangeSkin(3026);}
    if (ImGui::CustomButton("AcidVeil")) {ChangeSkin(3027);}
    if (ImGui::CustomButton("Immolation")) {ChangeSkin(3028);}
    if (ImGui::CustomButton("Stream")) {ChangeSkin(3030);}
    if (ImGui::CustomButton("Artificer")) {ChangeSkin(3031);}
    if (ImGui::CustomButton("DragonGlass")) {ChangeSkin(3032);}
    if (ImGui::CustomButton("Mimicry")) {ChangeSkin(3033);}
    if (ImGui::CustomButton("Rebellion")) {ChangeSkin(3034);}
    if (ImGui::CustomButton("Utility")) {ChangeSkin(3035);}
    if (ImGui::CustomButton("Ironclad")) {ChangeSkin(3036);}
    if (ImGui::CustomButton("Spectral")) {ChangeSkin(3037);}
    if (ImGui::CustomButton("Gangster")) {ChangeSkin(3038);}
    if (ImGui::CustomButton("Polymer")) {ChangeSkin(3039);}
    if (ImGui::CustomButton("Hanami")) {ChangeSkin(3040);}
    if (ImGui::CustomButton("Feral")) {ChangeSkin(3041);}
    if (ImGui::CustomButton("Marble")) {ChangeSkin(3042);}
    if (ImGui::CustomButton("Slash")) {ChangeSkin(3043);}
    if (ImGui::CustomButton("Frond")) {ChangeSkin(3044);}
    if (ImGui::CustomButton("Spray")) {ChangeSkin(3045);}
    if (ImGui::CustomButton("Flicker")) {ChangeSkin(3046);}
    if (ImGui::CustomButton("Impulse")) {ChangeSkin(3047);}
    if (ImGui::CustomButton("Ravage")) {ChangeSkin(3048);}
    if (ImGui::CustomButton("Dread")) {ChangeSkin(3049);}
    if (ImGui::CustomButton("Ritual")) {ChangeSkin(3050);}
    if (ImGui::CustomButton("WhiteCammo")) {ChangeSkin(3051);}
    if (ImGui::CustomButton("Gambit")) {ChangeSkin(3052);}
    if (ImGui::CustomButton("Tessera")) {ChangeSkin(3053);}
    if (ImGui::CustomButton("SkeletalHand")) {ChangeSkin(3054);}
    if (ImGui::CustomButton("FirmGrip")) {ChangeSkin(3055);}
    if (ImGui::CustomButton("IceStorm")) {ChangeSkin(3056);}
    if (ImGui::CustomButton("IronWolf")) {ChangeSkin(3057);}
    if (ImGui::CustomButton("ArcticCamo")) {ChangeSkin(3058);}
    if (ImGui::CustomButton("Razorwire")) {ChangeSkin(3059);}
    if (ImGui::CustomButton("JadeStone")) {ChangeSkin(3060);}
    }
    if (g22set){
    MenuText("G22");
    if (ImGui::CustomButton("PixelCamouflage")) {ChangeSkin(11001);}
    if (ImGui::CustomButton("Nest")) {ChangeSkin(11002);}
    if (ImGui::CustomButton("Pattern")) {ChangeSkin(11005);}
    if (ImGui::CustomButton("Inferno")) {ChangeSkin(11006);}
    if (ImGui::CustomButton("FrostWyrm")) {ChangeSkin(11008);}
    if (ImGui::CustomButton("Relic")) {ChangeSkin(41101);}
    if (ImGui::CustomButton("Starfall")) {ChangeSkin(41102);}
    if (ImGui::CustomButton("New1")) {ChangeSkin(61101);}
    if (ImGui::CustomButton("Monster")) {ChangeSkin(71103);}
    if (ImGui::CustomButton("WhiteCarbon")) {ChangeSkin(71104);}
    if (ImGui::CustomButton("Carbon")) {ChangeSkin(81107);}
    if (ImGui::CustomButton("YellowLine")) {ChangeSkin(81108);}
    if (ImGui::CustomButton("Scale")) {ChangeSkin(121100);}
    if (ImGui::CustomButton("Casual")) {ChangeSkin(170008);}
    if (ImGui::CustomButton("SteelGrip")) {ChangeSkin(210005);}
    if (ImGui::CustomButton("LionLord")) {ChangeSkin(230003);}
    if (ImGui::CustomButton("Haunt")) {ChangeSkin(240039);}
    if (ImGui::CustomButton("Flock")) {ChangeSkin(240097);}
    if (ImGui::CustomButton("Briar")) {ChangeSkin(240195);}
    if (ImGui::CustomButton("Impulse")) {ChangeSkin(240207);}
    if (ImGui::CustomButton("FlorDeMuertos")) {ChangeSkin(240253);}
    if (ImGui::CustomButton("Nest ST")) {ChangeSkin(1011002);}
    if (ImGui::CustomButton("FrostWyrm ST")) {ChangeSkin(1011008);}
    if (ImGui::CustomButton("Starfall ST")) {ChangeSkin(1041102);}
    if (ImGui::CustomButton("New1 ST")) {ChangeSkin(1061101);}
    if (ImGui::CustomButton("Monster ST")) {ChangeSkin(1071103);}
    if (ImGui::CustomButton("WhiteCarbon ST")) {ChangeSkin(1071104);}
    if (ImGui::CustomButton("Carbon ST")) {ChangeSkin(1081107);}
    if (ImGui::CustomButton("Casual ST")) {ChangeSkin(1170008);}
    if (ImGui::CustomButton("LionLord ST")) {ChangeSkin(1230003);}
    if (ImGui::CustomButton("Haunt ST")) {ChangeSkin(1240039);}
    if (ImGui::CustomButton("Flock ST")) {ChangeSkin(1240097);}
    if (ImGui::CustomButton("Briar ST")) {ChangeSkin(1240195);}
    if (ImGui::CustomButton("Impulse ST")) {ChangeSkin(1240207);}
    if (ImGui::CustomButton("FlorDeMuertos ST")) {ChangeSkin(1240253);}
    if (ImGui::CustomButton("G22 Twilight")) { ChangeSkin(240316); }
    if (ImGui::CustomButton("G22 Twilight ST")) { ChangeSkin(1240316); }
    if (ImGui::CustomButton("G22 Reindeer Sweater")) { ChangeSkin(240286); }
    if (ImGui::CustomButton("G22 Reindeer Sweater ST")) { ChangeSkin(1240286); }
    }
    if (uspset){
    MenuText("USP");
    if (ImGui::CustomButton("Genesis")) {ChangeSkin(12001);}
    if (ImGui::CustomButton("2Years")) {ChangeSkin(12002);}
    if (ImGui::CustomButton("2YearsRed")) {ChangeSkin(12003);}
    if (ImGui::CustomButton("Fiend")) {ChangeSkin(41201);}
    if (ImGui::CustomButton("Pisces")) {ChangeSkin(41212);}
    if (ImGui::CustomButton("New1")) {ChangeSkin(61201);}
    if (ImGui::CustomButton("Geometric")) {ChangeSkin(81200);}
    if (ImGui::CustomButton("Line")) {ChangeSkin(81214);}
    if (ImGui::CustomButton("Yellow")) {ChangeSkin(81215);}
    if (ImGui::CustomButton("Chameleon")) {ChangeSkin(121200);}
    if (ImGui::CustomButton("Stickerbomb")) {ChangeSkin(161200);}
    if (ImGui::CustomButton("PurpleCamo")) {ChangeSkin(170020);}
    if (ImGui::CustomButton("DigitalBurst")) {ChangeSkin(200010);}
    if (ImGui::CustomButton("Hunter02")) {ChangeSkin(210006);}
    if (ImGui::CustomButton("Ignite")) {ChangeSkin(220009);}
    if (ImGui::CustomButton("Griffin")) {ChangeSkin(230010);}
    if (ImGui::CustomButton("MirageMenace")) {ChangeSkin(240025);}
    if (ImGui::CustomButton("Corrode")) {ChangeSkin(240072);}
    if (ImGui::CustomButton("Ghosts")) {ChangeSkin(240095);}
    if (ImGui::CustomButton("Suture")) {ChangeSkin(240135);}
    if (ImGui::CustomButton("Rainforest")) {ChangeSkin(240173);}
    if (ImGui::CustomButton("CelestialTiger")) {ChangeSkin(240238);}
    if (ImGui::CustomButton("Horror")) {ChangeSkin(1240263);}
    if (ImGui::CustomButton("Pisces ST")) {ChangeSkin(1041212);}
    if (ImGui::CustomButton("New1 ST")) {ChangeSkin(1061201);}
    if (ImGui::CustomButton("Geometric ST")) {ChangeSkin(1081200);}
    if (ImGui::CustomButton("Chameleon ST")) {ChangeSkin(1121200);}
    if (ImGui::CustomButton("PurpleCamo ST")) {ChangeSkin(1170020);}
    if (ImGui::CustomButton("DigitalBurst ST")) {ChangeSkin(1200010);}
    if (ImGui::CustomButton("Ignite ST")) {ChangeSkin(1220009);}
    if (ImGui::CustomButton("Griffin ST")) {ChangeSkin(1230010);}
    if (ImGui::CustomButton("MirageMenace ST")) {ChangeSkin(1240025);}
    if (ImGui::CustomButton("Corrode ST")) {ChangeSkin(1240072);}
    if (ImGui::CustomButton("Ghosts ST")) {ChangeSkin(1240095);}
    if (ImGui::CustomButton("Suture ST")) {ChangeSkin(1240135);}
    if (ImGui::CustomButton("Rainforest ST")) {ChangeSkin(1240173);}
    if (ImGui::CustomButton("Horror ST")) {ChangeSkin(1240263);}
    if (ImGui::CustomButton("Hologram")) { ChangeSkin(240292); }
    if (ImGui::CustomButton("Hologram ST")) { ChangeSkin(1240292); }
    if (ImGui::CustomButton("TopSecret")) { ChangeSkin(240333); }
    if (ImGui::CustomButton("TopSecret ST")) { ChangeSkin(1240333); }
    }
    if (p350set){
    MenuText("P350");
    if (ImGui::CustomButton("Cyber")) {ChangeSkin(13001);}
    if (ImGui::CustomButton("Savannah")) {ChangeSkin(13002);}
    if (ImGui::CustomButton("ForestSpirit")) {ChangeSkin(13003);}
    if (ImGui::CustomButton("Rally")) {ChangeSkin(13004);}
    if (ImGui::CustomButton("Skull")) {ChangeSkin(13005);}
    if (ImGui::CustomButton("Poison")) {ChangeSkin(71306);}
    if (ImGui::CustomButton("Autumn")) {ChangeSkin(81300);}
    if (ImGui::CustomButton("Nano")) {ChangeSkin(81323);}
    if (ImGui::CustomButton("Blizzard")) {ChangeSkin(91300);}
    if (ImGui::CustomButton("4Years")) {ChangeSkin(101300);}
    if (ImGui::CustomButton("Oni")) {ChangeSkin(121300);}
    if (ImGui::CustomButton("TagKing")) {ChangeSkin(170018);}
    if (ImGui::CustomButton("Raider")) {ChangeSkin(190005);}
    if (ImGui::CustomButton("Rhino")) {ChangeSkin(220007);}
    if (ImGui::CustomButton("Sandspirit")) {ChangeSkin(240020);}
    if (ImGui::CustomButton("FestalWrap")) {ChangeSkin(240054);}
    if (ImGui::CustomButton("Vitrum")) {ChangeSkin(240084);}
    if (ImGui::CustomButton("Gearshift")) {ChangeSkin(240120);}
    if (ImGui::CustomButton("InkSpree")) {ChangeSkin(240166);}
    if (ImGui::CustomButton("Ooze")) {ChangeSkin(240191);}
    if (ImGui::CustomButton("Impact")) {ChangeSkin(240209);}
    if (ImGui::CustomButton("SilverWing")) {ChangeSkin(240237);}
    if (ImGui::CustomButton("Cyber ST")) {ChangeSkin(1013001);}
    if (ImGui::CustomButton("ForestSpirit ST")) {ChangeSkin(1013003);}
    if (ImGui::CustomButton("Rally ST")) {ChangeSkin(1013004);}
    if (ImGui::CustomButton("Poison ST")) {ChangeSkin(1071306);}
    if (ImGui::CustomButton("Autumn ST")) {ChangeSkin(1081300);}
    if (ImGui::CustomButton("Blizzard ST")) {ChangeSkin(1091300);}
    if (ImGui::CustomButton("Oni ST")) {ChangeSkin(1121300);}
    if (ImGui::CustomButton("TagKing ST")) {ChangeSkin(1170018);}
    if (ImGui::CustomButton("Raider ST")) {ChangeSkin(1190005);}
    if (ImGui::CustomButton("Sandspirit ST")) {ChangeSkin(1240020);}
    if (ImGui::CustomButton("FestalWrap ST")) {ChangeSkin(1240054);}
    if (ImGui::CustomButton("Gearshift ST")) {ChangeSkin(1240120);}
    if (ImGui::CustomButton("InkSpree ST")) {ChangeSkin(1240166);}
    if (ImGui::CustomButton("Ooze ST")) {ChangeSkin(1240191);}
    if (ImGui::CustomButton("Impact ST")) {ChangeSkin(1240209);}
    if (ImGui::CustomButton("Emberbird")) { ChangeSkin(240312); }
    if (ImGui::CustomButton("Emberbird ST")) { ChangeSkin(1240312); }
    if (ImGui::CustomButton("LabPrototype")) { ChangeSkin(240338); }
    if (ImGui::CustomButton("LabPrototype ST")) { ChangeSkin(1240338); }
    }
    if (beretasset){
    MenuText("Berettas");
    if (ImGui::CustomButton("SoulDevourer")) {ChangeSkin(220015);}
    if (ImGui::CustomButton("Damascus")) {ChangeSkin(220016);}
    if (ImGui::CustomButton("BlazingMaw")) {ChangeSkin(230011);}
    if (ImGui::CustomButton("RoyalRose")) {ChangeSkin(230012);}
    if (ImGui::CustomButton("Hexagon")) {ChangeSkin(240004);}
    if (ImGui::CustomButton("Hybrid")) {ChangeSkin(240037);}
    if (ImGui::CustomButton("LumosShard")) {ChangeSkin(240049);}
    if (ImGui::CustomButton("Artifacts")) {ChangeSkin(240086);}
    if (ImGui::CustomButton("ConstructionSector")) {ChangeSkin(240117);}
    if (ImGui::CustomButton("Verdict")) {ChangeSkin(240184);}
    if (ImGui::CustomButton("Cascade")) {ChangeSkin(240240);}
    if (ImGui::CustomButton("SoulDevourer ST")) {ChangeSkin(1220015);}
    if (ImGui::CustomButton("Damascus ST")) {ChangeSkin(1220016);}
    if (ImGui::CustomButton("BlazingMaw ST")) {ChangeSkin(1230011);}
    if (ImGui::CustomButton("RoyalRose ST")) {ChangeSkin(1230012);}
    if (ImGui::CustomButton("Hexagon ST")) {ChangeSkin(1240004);}
    if (ImGui::CustomButton("Hybrid ST")) {ChangeSkin(1240037);}
    if (ImGui::CustomButton("LumosShard ST")) {ChangeSkin(1240049);}
    if (ImGui::CustomButton("ConstructionSector ST")) {ChangeSkin(1240117);}
    }
    if (tec9set){
    MenuText("Tec9");
    if (ImGui::CustomButton("Aurora")) {ChangeSkin(41601);}
    if (ImGui::CustomButton("Fable")) {ChangeSkin(41605);}
    if (ImGui::CustomButton("New3")) {ChangeSkin(51601);}
    if (ImGui::CustomButton("New1")) {ChangeSkin(61601);}
    if (ImGui::CustomButton("Reactor")) {ChangeSkin(71607);}
    if (ImGui::CustomButton("Spot")) {ChangeSkin(81613);}
    if (ImGui::CustomButton("Tropic")) {ChangeSkin(121600);}
    if (ImGui::CustomButton("Restless")) {ChangeSkin(141600);}
    if (ImGui::CustomButton("HolidayFrost")) {ChangeSkin(151600);}
    if (ImGui::CustomButton("Stickerbomb")) {ChangeSkin(161600);}
    if (ImGui::CustomButton("Needle")) {ChangeSkin(170019);}
    if (ImGui::CustomButton("Splash")) {ChangeSkin(180015);}
    if (ImGui::CustomButton("TieDye")) {ChangeSkin(220013);}
    if (ImGui::CustomButton("Glitch")) {ChangeSkin(240001);}
    if (ImGui::CustomButton("Verglas")) {ChangeSkin(240046);}
    if (ImGui::CustomButton("Disguise")) {ChangeSkin(240098);}
    if (ImGui::CustomButton("Gangster")) {ChangeSkin(240123);}
    if (ImGui::CustomButton("NightSlayer")) {ChangeSkin(240147);}
    if (ImGui::CustomButton("Ultimatum")) {ChangeSkin(240183);}
    if (ImGui::CustomButton("Iris")) {ChangeSkin(240222);}
    if (ImGui::CustomButton("Blossom")) {ChangeSkin(240235);}
    if (ImGui::CustomButton("Fable ST")) {ChangeSkin(1041605);}
    if (ImGui::CustomButton("New1 ST")) {ChangeSkin(1061601);}
    if (ImGui::CustomButton("Reactor ST")) {ChangeSkin(1071607);}
    if (ImGui::CustomButton("Tropic ST")) {ChangeSkin(1121600);}
    if (ImGui::CustomButton("Restless ST")) {ChangeSkin(1141600);}
    if (ImGui::CustomButton("HolidayFrost ST")) {ChangeSkin(1151600);}
    if (ImGui::CustomButton("Needle ST")) {ChangeSkin(1170019);}
    if (ImGui::CustomButton("Splash ST")) {ChangeSkin(1180015);}
    if (ImGui::CustomButton("TieDye ST")) {ChangeSkin(1220013);}
    if (ImGui::CustomButton("Glitch ST")) {ChangeSkin(1240001);}
    if (ImGui::CustomButton("Verglas ST")) {ChangeSkin(1240046);}
    if (ImGui::CustomButton("Disguise ST")) {ChangeSkin(1240098);}
    if (ImGui::CustomButton("Gangster ST")) {ChangeSkin(1240123);}
    if (ImGui::CustomButton("NightSlayer ST")) {ChangeSkin(1240147);}
    if (ImGui::CustomButton("Iris ST")) {ChangeSkin(1240222);}
    if (ImGui::CustomButton(" Gift Wrap")) { ChangeSkin(240287); }
    if (ImGui::CustomButton(" Gift Wrap ST")) { ChangeSkin(1240287); }
    }
    if (fivesevenset){
    MenuText("FiveSeven");
    if (ImGui::CustomButton("Venom")) {ChangeSkin(41701);}
    if (ImGui::CustomButton("Tactical")) {ChangeSkin(41703);}
    if (ImGui::CustomButton("New2")) {ChangeSkin(51701);}
    if (ImGui::CustomButton("Poison")) {ChangeSkin(71701);}
    if (ImGui::CustomButton("Zone")) {ChangeSkin(71702);}
    if (ImGui::CustomButton("Rush")) {ChangeSkin(81700);}
    if (ImGui::CustomButton("SightRed")) {ChangeSkin(81725);}
    if (ImGui::CustomButton("SightGrey")) {ChangeSkin(81726);}
    if (ImGui::CustomButton("Enforcer")) {ChangeSkin(131700);}
    if (ImGui::CustomButton("DemonicFog")) {ChangeSkin(141700);}
    if (ImGui::CustomButton("HolidayFrost")) {ChangeSkin(151700);}
    if (ImGui::CustomButton("Zap")) {ChangeSkin(170007);}
    if (ImGui::CustomButton("Octopus")) {ChangeSkin(180013);}
    if (ImGui::CustomButton("Ophidian")) {ChangeSkin(230002);}
    if (ImGui::CustomButton("Hellspawn")) {ChangeSkin(240041);}
    if (ImGui::CustomButton("Toxic")) {ChangeSkin(240075);}
    if (ImGui::CustomButton("Corrosion")) {ChangeSkin(240090);}
    if (ImGui::CustomButton("Polymer")) {ChangeSkin(240116);}
    if (ImGui::CustomButton("PinkShot")) {ChangeSkin(240148);}
    if (ImGui::CustomButton("Aquamarine")) {ChangeSkin(240181);}
    if (ImGui::CustomButton("Vines")) {ChangeSkin(240221);}
    if (ImGui::CustomButton("Venom ST")) {ChangeSkin(1041701);}
    if (ImGui::CustomButton("Tactical ST")) {ChangeSkin(1041703);}
    if (ImGui::CustomButton("Poison ST")) {ChangeSkin(1071701);}
    if (ImGui::CustomButton("Zone ST")) {ChangeSkin(1071702);}
    if (ImGui::CustomButton("Rush ST")) {ChangeSkin(1081700);}
    if (ImGui::CustomButton("SightGrey ST")) {ChangeSkin(1081726);}
    if (ImGui::CustomButton("Enforcer ST")) {ChangeSkin(1131700);}
    if (ImGui::CustomButton("DemonicFog ST")) {ChangeSkin(1141700);}
    if (ImGui::CustomButton("HolidayFrost ST")) {ChangeSkin(1151700);}
    if (ImGui::CustomButton("Ophidian ST")) {ChangeSkin(1230002);}
    if (ImGui::CustomButton("Hellspawn ST")) {ChangeSkin(1240041);}
    if (ImGui::CustomButton("Toxic ST")) {ChangeSkin(1240075);}
    if (ImGui::CustomButton("Polymer ST")) {ChangeSkin(1240116);}
    if (ImGui::CustomButton("PinkShot ST")) {ChangeSkin(1240148);}
    if (ImGui::CustomButton("Vines ST")) {ChangeSkin(1240221);}
    if (ImGui::CustomButton("FlashingFlame")) { ChangeSkin(240305);}
    if (ImGui::CustomButton("BurningMist_1")) { ChangeSkin(10240324);}
    if (ImGui::CustomButton("BurningMist_2")) { ChangeSkin(20240324);}
    if (ImGui::CustomButton("BurningMist_3")) { ChangeSkin(30240324);}
    if (ImGui::CustomButton("BurningMist_1 ST")) { ChangeSkin(11240324);}
    if (ImGui::CustomButton("BurningMist_2 ST")) { ChangeSkin(21240324);}
    if (ImGui::CustomButton("BurningMist_3 ST")) { ChangeSkin(31240324);}
    }
    if (deagleset){
    MenuText("Deagle");
    if (ImGui::CustomButton("CaptainMorgan")) {ChangeSkin(15001);}
    if (ImGui::CustomButton("Blood")) {ChangeSkin(15002);}
    if (ImGui::CustomButton("Predator")) {ChangeSkin(15003);}
    if (ImGui::CustomButton("RedDragon")) {ChangeSkin(15004);}
    if (ImGui::CustomButton("Winner")) {ChangeSkin(15005);}
    if (ImGui::CustomButton("DragonGlass")) {ChangeSkin(15006);}
    if (ImGui::CustomButton("Thunder")) {ChangeSkin(15007);}
    if (ImGui::CustomButton("Ace")) {ChangeSkin(41502);}
    if (ImGui::CustomButton("Pro")) {ChangeSkin(81505);}
    if (ImGui::CustomButton("Orochi")) {ChangeSkin(121500);}
    if (ImGui::CustomButton("Mafia")) {ChangeSkin(131500);}
    if (ImGui::CustomButton("Piranha")) {ChangeSkin(170003);}
    if (ImGui::CustomButton("Glory")) {ChangeSkin(170004);}
    if (ImGui::CustomButton("Infection")) {ChangeSkin(190002);}
    if (ImGui::CustomButton("Hunter")) {ChangeSkin(210001);}
    if (ImGui::CustomButton("Venator")) {ChangeSkin(220020);}
    if (ImGui::CustomButton("DustDevil")) {ChangeSkin(240024);}
    if (ImGui::CustomButton("Aureate")) {ChangeSkin(240028);}
    if (ImGui::CustomButton("FusionCore")) {ChangeSkin(240081);}
    if (ImGui::CustomButton("VioletFlame")) {ChangeSkin(240092);}
    if (ImGui::CustomButton("Vermilion")) {ChangeSkin(240163);}
    if (ImGui::CustomButton("Eclipse")) {ChangeSkin(240194);}
    if (ImGui::CustomButton("Gambit")) {ChangeSkin(240214);}
    if (ImGui::CustomButton("Blossom")) {ChangeSkin(240249);}
    if (ImGui::CustomButton("Predator ST")) {ChangeSkin(1015003);}
    if (ImGui::CustomButton("RedDragon ST")) {ChangeSkin(1015004);}
    if (ImGui::CustomButton("DragonGlass ST")) {ChangeSkin(1015006);}
    if (ImGui::CustomButton("Ace ST")) {ChangeSkin(1041502);}
    if (ImGui::CustomButton("Orochi ST")) {ChangeSkin(1121500);}
    if (ImGui::CustomButton("Mafia ST")) {ChangeSkin(1131500);}
    if (ImGui::CustomButton("Piranha ST")) {ChangeSkin(1170003);}
    if (ImGui::CustomButton("Infection ST")) {ChangeSkin(1190002);}
    if (ImGui::CustomButton("Venator ST")) {ChangeSkin(1220020);}
    if (ImGui::CustomButton("DustDevil ST")) {ChangeSkin(1240024);}
    if (ImGui::CustomButton("FusionCore ST")) {ChangeSkin(1240081);}
    if (ImGui::CustomButton("VioletFlame ST")) {ChangeSkin(1240092);}
    if (ImGui::CustomButton("Vermilion ST")) {ChangeSkin(1240163);}
    if (ImGui::CustomButton("Eclipse ST")) {ChangeSkin(1240194);}
    if (ImGui::CustomButton("Gambit ST")) {ChangeSkin(1240214);}
    if (ImGui::CustomButton("Blossom ST")) {ChangeSkin(1240249);}
    if (ImGui::CustomButton("Wild Flower")) { ChangeSkin(240283); }
    if (ImGui::CustomButton("Wild Flower ST")) { ChangeSkin(1240283); }
    if (ImGui::CustomButton("JadeStone")) { ChangeSkin(240330); }
    if (ImGui::CustomButton("JadeStone ST")) { ChangeSkin(1240330); }
    }
    if (ump45set){
    MenuText("UMP45");
    if (ImGui::CustomButton("Cyberpunk")) {ChangeSkin(32001);}
    if (ImGui::CustomButton("Pixel")) {ChangeSkin(32002);}
    if (ImGui::CustomButton("Shark")) {ChangeSkin(32003);}
    if (ImGui::CustomButton("Winged")) {ChangeSkin(32004);}
    if (ImGui::CustomButton("Beast")) {ChangeSkin(32005);}
    if (ImGui::CustomButton("Iron")) {ChangeSkin(32006);}
    if (ImGui::CustomButton("PixelV2")) {ChangeSkin(43201);}
    if (ImGui::CustomButton("Cerberus")) {ChangeSkin(43202);}
    if (ImGui::CustomButton("Gas")) {ChangeSkin(73208);}
    if (ImGui::CustomButton("WhiteCarbon")) {ChangeSkin(73209);}
    if (ImGui::CustomButton("Geometric")) {ChangeSkin(83200);}
    if (ImGui::CustomButton("Spirit")) {ChangeSkin(83201);}
    if (ImGui::CustomButton("4Years")) {ChangeSkin(103200);}
    if (ImGui::CustomButton("PeacefulDream")) {ChangeSkin(123200);}
    if (ImGui::CustomButton("Luminous")) {ChangeSkin(200002);}
    if (ImGui::CustomButton("Warchief")) {ChangeSkin(230009);}
    if (ImGui::CustomButton("Arid")) {ChangeSkin(240018);}
    if (ImGui::CustomButton("Industrial")) {ChangeSkin(240076);}
    if (ImGui::CustomButton("VHS")) {ChangeSkin(240087);}
    if (ImGui::CustomButton("Heatwave")) {ChangeSkin(240167);}
    if (ImGui::CustomButton("Smelter")) {ChangeSkin(240220);}
    if (ImGui::CustomButton("Leviathan")) {ChangeSkin(1240264);}
    if (ImGui::CustomButton("Cyberpunk ST")) {ChangeSkin(1032001);}
    if (ImGui::CustomButton("Shark ST")) {ChangeSkin(1032003);}
    if (ImGui::CustomButton("Winged ST")) {ChangeSkin(1032004);}
    if (ImGui::CustomButton("Beast ST")) {ChangeSkin(1032005);}
    if (ImGui::CustomButton("Cerberus ST")) {ChangeSkin(1043202);}
    if (ImGui::CustomButton("Gas ST")) {ChangeSkin(1073208);}
    if (ImGui::CustomButton("WhiteCarbon ST")) {ChangeSkin(1073209);}
    if (ImGui::CustomButton("Geometric ST")) {ChangeSkin(1083200);}
    if (ImGui::CustomButton("Spirit ST")) {ChangeSkin(1083201);}
    if (ImGui::CustomButton("PeacefulDream ST")) {ChangeSkin(1123200);}
    if (ImGui::CustomButton("Luminous ST")) {ChangeSkin(1200002);}
    if (ImGui::CustomButton("Warchief ST")) {ChangeSkin(1230009);}
    if (ImGui::CustomButton("Arid ST")) {ChangeSkin(1240018);}
    if (ImGui::CustomButton("Industrial ST")) {ChangeSkin(1240076);}
    if (ImGui::CustomButton("Heatwave ST")) {ChangeSkin(1240167);}
    if (ImGui::CustomButton("Smelter ST")) {ChangeSkin(1240220);}
    if (ImGui::CustomButton("Leviathan ST")) {ChangeSkin(1240264);}
    }
    if (mp7set){
    MenuText("MP7");
    if (ImGui::CustomButton("Offroad")) {ChangeSkin(34001);}
    if (ImGui::CustomButton("Arcade")) {ChangeSkin(34002);}
    if (ImGui::CustomButton("2Years")) {ChangeSkin(34003);}
    if (ImGui::CustomButton("2YearsRed")) {ChangeSkin(34004);}
    if (ImGui::CustomButton("Thorn")) {ChangeSkin(43401);}
    if (ImGui::CustomButton("Lich")) {ChangeSkin(43402);}
    if (ImGui::CustomButton("New1")) {ChangeSkin(63401);}
    if (ImGui::CustomButton("Revival")) {ChangeSkin(83400);}
    if (ImGui::CustomButton("Girl")) {ChangeSkin(83409);}
    if (ImGui::CustomButton("Graffity")) {ChangeSkin(83410);}
    if (ImGui::CustomButton("Monkey")) {ChangeSkin(83411);}
    if (ImGui::CustomButton("Blizzard")) {ChangeSkin(93400);}
    if (ImGui::CustomButton("Empire")) {ChangeSkin(123400);}
    if (ImGui::CustomButton("Stickerbomb")) {ChangeSkin(163400);}
    if (ImGui::CustomButton("Dawn")) {ChangeSkin(170016);}
    if (ImGui::CustomButton("SpaceBlaster")) {ChangeSkin(200004);}
    if (ImGui::CustomButton("Presicion")) {ChangeSkin(220010);}
    if (ImGui::CustomButton("Ridge")) {ChangeSkin(240016);}
    if (ImGui::CustomButton("FestalWrap")) {ChangeSkin(240052);}
    if (ImGui::CustomButton("Fright")) {ChangeSkin(240099);}
    if (ImGui::CustomButton("Rebellion")) {ChangeSkin(240119);}
    if (ImGui::CustomButton("R1NA")) {ChangeSkin(240232);}
    if (ImGui::CustomButton("Offroad ST")) {ChangeSkin(1034001);}
    if (ImGui::CustomButton("Arcade ST")) {ChangeSkin(1034002);}
    if (ImGui::CustomButton("Lich ST")) {ChangeSkin(1043402);}
    if (ImGui::CustomButton("New1 ST")) {ChangeSkin(1063401);}
    if (ImGui::CustomButton("Revival ST")) {ChangeSkin(1083400);}
    if (ImGui::CustomButton("Graffity ST")) {ChangeSkin(1083410);}
    if (ImGui::CustomButton("Monkey ST")) {ChangeSkin(1083411);}
    if (ImGui::CustomButton("Blizzard ST")) {ChangeSkin(1093400);}
    if (ImGui::CustomButton("Empire ST")) {ChangeSkin(1123400);}
    if (ImGui::CustomButton("SpaceBlaster ST")) {ChangeSkin(1200004);}
    if (ImGui::CustomButton("Ridge ST")) {ChangeSkin(1240016);}
    if (ImGui::CustomButton("FestalWrap ST")) {ChangeSkin(1240052);}
    if (ImGui::CustomButton("Fright ST")) {ChangeSkin(1240099);}
    if (ImGui::CustomButton("Rebellion ST")) {ChangeSkin(1240119);}
    if (ImGui::CustomButton("R1NA ST")) {ChangeSkin(1240232);}
    if (ImGui::CustomButton("MP7 Magma Trail")) { ChangeSkin(240307); }
    if (ImGui::CustomButton("MP7 Magma Trail ST")) { ChangeSkin(1240307); }
    }
    if (p90set){
    MenuText("P90");
    if (ImGui::CustomButton("Radiation")) {ChangeSkin(35001);}
    if (ImGui::CustomButton("Ghoul")) {ChangeSkin(35002);}
    if (ImGui::CustomButton("Fury")) {ChangeSkin(35003);}
    if (ImGui::CustomButton("Pilot")) {ChangeSkin(35004);}
    if (ImGui::CustomButton("Jungle")) {ChangeSkin(43502);}
    if (ImGui::CustomButton("Samurai")) {ChangeSkin(83512);}
    if (ImGui::CustomButton("IronWill")) {ChangeSkin(93500);}
    if (ImGui::CustomButton("RONINmk9")) {ChangeSkin(133500);}
    if (ImGui::CustomButton("Z50FUJIN")) {ChangeSkin(133501);}
    if (ImGui::CustomButton("Oops")) {ChangeSkin(170017);}
    if (ImGui::CustomButton("PurpleMist")) {ChangeSkin(1200006);}
    if (ImGui::CustomButton("Clash")) {ChangeSkin(1220017);}
    if (ImGui::CustomButton("DragonFlame")) {ChangeSkin(1230007);}
    if (ImGui::CustomButton("Nebula")) {ChangeSkin(240015);}
    if (ImGui::CustomButton("Revenant")) {ChangeSkin(240042);}
    if (ImGui::CustomButton("FusionCore")) {ChangeSkin(240079);}
    if (ImGui::CustomButton("ColdLead")) {ChangeSkin(240088);}
    if (ImGui::CustomButton("Noir")) {ChangeSkin(240192);}
    if (ImGui::CustomButton("Oracle")) {ChangeSkin(240213);}
    if (ImGui::CustomButton("Foxfire")) {ChangeSkin(240245);}
    if (ImGui::CustomButton("Ghoul ST")) {ChangeSkin(1035002);}
    if (ImGui::CustomButton("Samurai ST")) {ChangeSkin(1083512);}
    if (ImGui::CustomButton("IronWill ST")) {ChangeSkin(1093500);}
    if (ImGui::CustomButton("RONINmk9 ST")) {ChangeSkin(1133500);}
    if (ImGui::CustomButton("Z50FUJIN ST")) {ChangeSkin(1133501);}
    if (ImGui::CustomButton("Oops ST")) {ChangeSkin(1170017);}
    if (ImGui::CustomButton("PurpleMist ST")) {ChangeSkin(1200006);}
    if (ImGui::CustomButton("Clash ST")) {ChangeSkin(1220017);}
    if (ImGui::CustomButton("DragonFlame ST")) {ChangeSkin(1230007);}
    if (ImGui::CustomButton("Revenant ST")) {ChangeSkin(1240042);}
    if (ImGui::CustomButton("FusionCore ST")) {ChangeSkin(1240079);}
    if (ImGui::CustomButton("Noir ST")) {ChangeSkin(1240192);}
    if (ImGui::CustomButton("Oracle ST")) {ChangeSkin(1240213);}
    if (ImGui::CustomButton("Foxfire ST")) {ChangeSkin(1240245);}
    if (ImGui::CustomButton("Horizon")) { ChangeSkin(240308); }
    if (ImGui::CustomButton("Horizon ST")) { ChangeSkin(1240308); }
    if (ImGui::CustomButton("Hologram")) { ChangeSkin(240291); }
    if (ImGui::CustomButton("Hologram ST")) { ChangeSkin(1240291); }
    }
    if (mp5set){
    MenuText("MP5");
    if (ImGui::CustomButton("ProjectZ9")) {ChangeSkin(73610);}
    if (ImGui::CustomButton("Reactor")) {ChangeSkin(73611);}
    if (ImGui::CustomButton("Zone")) {ChangeSkin(73612);}
    if (ImGui::CustomButton("4Years")) {ChangeSkin(103500);}
    if (ImGui::CustomButton("NorthernFury")) {ChangeSkin(93600);}
    if (ImGui::CustomButton("Dusk")) {ChangeSkin(170015);}
    if (ImGui::CustomButton("SpaceBlaster")) {ChangeSkin(200005);}
    if (ImGui::CustomButton("Vector")) {ChangeSkin(220002);}
    if (ImGui::CustomButton("Gorgon")) {ChangeSkin(180010);}
    if (ImGui::CustomButton("SilverPlated")) {ChangeSkin(240089);}
    if (ImGui::CustomButton("WaterGun")) {ChangeSkin(240105);}
    if (ImGui::CustomButton("Insanity")) {ChangeSkin(240035);}
    if (ImGui::CustomButton("Blueprint")) {ChangeSkin(240206);}
    if (ImGui::CustomButton("Clouds")) {ChangeSkin(240241);}
    if (ImGui::CustomButton("ArcaneSurge")) {ChangeSkin(1240262);}
    if (ImGui::CustomButton("ProjectZ9 ST")) {ChangeSkin(1073610);}
    if (ImGui::CustomButton("Reactor ST")) {ChangeSkin(1073611);}
    if (ImGui::CustomButton("Zone ST")) {ChangeSkin(1073612);}
    if (ImGui::CustomButton("NorthernFury ST")) {ChangeSkin(1093600);}
    if (ImGui::CustomButton("Dusk ST")) {ChangeSkin(1170015);}
    if (ImGui::CustomButton("SpaceBlaster ST")) {ChangeSkin(1200005);}
    if (ImGui::CustomButton("Vector ST")) {ChangeSkin(1220002);}
    if (ImGui::CustomButton("Gorgon ST")) {ChangeSkin(1180010);}
    if (ImGui::CustomButton("Insanity ST")) {ChangeSkin(1240035);}
    if (ImGui::CustomButton("WaterGun ST")) {ChangeSkin(1240105);}
    if (ImGui::CustomButton("Blueprint ST")) {ChangeSkin(1240206);}
    if (ImGui::CustomButton("Clouds ST")) {ChangeSkin(1240241);}
    if (ImGui::CustomButton("ArcaneSurge ST")) {ChangeSkin(1240262);}
    }
    if (akrset){
    MenuText("AKR");
    if (ImGui::CustomButton("TreasureHunter")) {ChangeSkin(44002);}
    if (ImGui::CustomButton("Tiger")) {ChangeSkin(44003);}
    if (ImGui::CustomButton("Sport")) {ChangeSkin(44004);}
    if (ImGui::CustomButton("Necromancer")) {ChangeSkin(44005);}
    if (ImGui::CustomButton("Carbon")) {ChangeSkin(44006);}
    if (ImGui::CustomButton("2Years")) {ChangeSkin(44007);}
    if (ImGui::CustomButton("Worm")) {ChangeSkin(44401);}
    if (ImGui::CustomButton("New2")) {ChangeSkin(54401);}
    if (ImGui::CustomButton("Dragon")) {ChangeSkin(84402);}
    if (ImGui::CustomButton("Nano")) {ChangeSkin(84403);}
    if (ImGui::CustomButton("Scale")) {ChangeSkin(124400);}
    if (ImGui::CustomButton("Noname")) {ChangeSkin(144400);}
    if (ImGui::CustomButton("TagKing")) {ChangeSkin(170001);}
    if (ImGui::CustomButton("Scylla")) {ChangeSkin(180001);}
    if (ImGui::CustomButton("DigitalBurst")) {ChangeSkin(200009);}
    if (ImGui::CustomButton("SteelGrip")) {ChangeSkin(210004);}
    if (ImGui::CustomButton("Evolution")) {ChangeSkin(220011);}
    if (ImGui::CustomButton("MirageMenace")) {ChangeSkin(240027);}
    if (ImGui::CustomButton("Icewing")) {ChangeSkin(240051);}
    if (ImGui::CustomButton("Vermilion")) {ChangeSkin(240162);}
    if (ImGui::CustomButton("Orchid")) {ChangeSkin(240233);}
    if (ImGui::CustomButton("LaReina")) {ChangeSkin(1240278);}
    if (ImGui::CustomButton("TreasureHunter ST")) {ChangeSkin(1044002);}
    if (ImGui::CustomButton("Sport ST")) {ChangeSkin(1044004);}
    if (ImGui::CustomButton("Necromancer ST")) {ChangeSkin(1044005);}
    if (ImGui::CustomButton("Carbon ST")) {ChangeSkin(1044006);}
    if (ImGui::CustomButton("Dragon ST")) {ChangeSkin(1084402);}
    if (ImGui::CustomButton("Nano ST")) {ChangeSkin(1084403);}
    if (ImGui::CustomButton("Scale ST")) {ChangeSkin(1124400);}
    if (ImGui::CustomButton("Noname ST")) {ChangeSkin(1144400);}
    if (ImGui::CustomButton("TagKing ST")) {ChangeSkin(1170001);}
    if (ImGui::CustomButton("Scylla ST")) {ChangeSkin(1180001);}
    if (ImGui::CustomButton("DigitalBurst ST")) {ChangeSkin(1200009);}
    if (ImGui::CustomButton("Evolution ST")) {ChangeSkin(1220011);}
    if (ImGui::CustomButton("MirageMenace ST")) {ChangeSkin(1240027);}
    if (ImGui::CustomButton("Icewing ST")) {ChangeSkin(1240051);}
    if (ImGui::CustomButton("Vermilion ST")) {ChangeSkin(1240162);}
    if (ImGui::CustomButton("Orchid ST")) {ChangeSkin(1240233);}
    if (ImGui::CustomButton("LaReina ST")) {ChangeSkin(1240278);}
    if (ImGui::CustomButton("Sketch")) { ChangeSkin(240313); }
    if (ImGui::CustomButton("Sketch ST")) { ChangeSkin(1240313); }
    if (ImGui::CustomButton("ArcticCamo")) { ChangeSkin(240342); }
    if (ImGui::CustomButton("ArcticCamo ST")) { ChangeSkin(1240342); }
    }
    if (akr12set){
    MenuText("AKR12");
    if (ImGui::CustomButton("Railgun")) {ChangeSkin(45001);}
    if (ImGui::CustomButton("PixelCamouflage")) {ChangeSkin(45002);}
    if (ImGui::CustomButton("Mechanic")) {ChangeSkin(45003);}
    if (ImGui::CustomButton("Aurora")) {ChangeSkin(45004);}
    if (ImGui::CustomButton("Carbon")) {ChangeSkin(84500);}
    if (ImGui::CustomButton("Spring")) {ChangeSkin(84501);}
    if (ImGui::CustomButton("4Years")) {ChangeSkin(104400);}
    if (ImGui::CustomButton("Geometric")) {ChangeSkin(84400);}
    if (ImGui::CustomButton("Flow")) {ChangeSkin(94400);}
    if (ImGui::CustomButton("Roar")) {ChangeSkin(124500);}
    if (ImGui::CustomButton("Armored")) {ChangeSkin(134500);}
    if (ImGui::CustomButton("Transistor")) {ChangeSkin(134501);}
    if (ImGui::CustomButton("Steampunk")) {ChangeSkin(170002);}
    if (ImGui::CustomButton("Carving")) {ChangeSkin(180016);}
    if (ImGui::CustomButton("Riot")) {ChangeSkin(190001);}
    if (ImGui::CustomButton("Ashbringer")) {ChangeSkin(230001);}
    if (ImGui::CustomButton("Haunt")) {ChangeSkin(240043);}
    if (ImGui::CustomButton("Mimicry")) {ChangeSkin(240100);}
    if (ImGui::CustomButton("ColdHearted")) {ChangeSkin(240140);}
    if (ImGui::CustomButton("Hanami")) {ChangeSkin(240150);}
    if (ImGui::CustomButton("Shark")) {ChangeSkin(240212);}
    if (ImGui::CustomButton("Kensei")) {ChangeSkin(240250);}
    if (ImGui::CustomButton("Railgun ST")) {ChangeSkin(1045001);}
    if (ImGui::CustomButton("PixelCamouflage ST")) {ChangeSkin(1045002);}
    if (ImGui::CustomButton("Carbon ST")) {ChangeSkin(1084500);}
    if (ImGui::CustomButton("Geometric ST")) {ChangeSkin(1084400);}
    if (ImGui::CustomButton("Flow ST")) {ChangeSkin(1094400);}
    if (ImGui::CustomButton("Roar ST")) {ChangeSkin(1124500);}
    if (ImGui::CustomButton("Armored ST")) {ChangeSkin(1134500);}
    if (ImGui::CustomButton("Transistor ST")) {ChangeSkin(1134501);}
    if (ImGui::CustomButton("Steampunk ST")) {ChangeSkin(1170002);}
    if (ImGui::CustomButton("Carving ST")) {ChangeSkin(1180016);}
    if (ImGui::CustomButton("Riot ST")) {ChangeSkin(1190001);}
    if (ImGui::CustomButton("Ashbringer ST")) {ChangeSkin(1230001);}
    if (ImGui::CustomButton("Haunt ST")) {ChangeSkin(1240043);}
    if (ImGui::CustomButton("Mimicry ST")) {ChangeSkin(1240100);}
    if (ImGui::CustomButton("Hanami ST")) {ChangeSkin(1240150);}
    if (ImGui::CustomButton("Shark ST")) {ChangeSkin(1240212);}
    if (ImGui::CustomButton("Kensei ST")) {ChangeSkin(1240250);}
    if (ImGui::CustomButton("Ice Storm")) { ChangeSkin(240281); }
    if (ImGui::CustomButton("Ice Storm ST")) { ChangeSkin(1240281); }
    if (ImGui::CustomButton("YearTheHorse_1")) { ChangeSkin(10240326); }
    if (ImGui::CustomButton("YearTheHorse_2")) { ChangeSkin(20240326); }
    if (ImGui::CustomButton("YearTheHorse_3")) { ChangeSkin(30240326); }
    if (ImGui::CustomButton("YearTheHorse_1 ST")) { ChangeSkin(11240326); }
    if (ImGui::CustomButton("YearTheHorse_2 ST")) { ChangeSkin(21240326); }
    if (ImGui::CustomButton("YearTheHorse_3 ST")) { ChangeSkin(31240326); }
    }
    if (m4set){
    MenuText("M4");
    if (ImGui::CustomButton("Predator")) {ChangeSkin(46001);}
    if (ImGui::CustomButton("Necromancer")) {ChangeSkin(46002);}
    if (ImGui::CustomButton("Tiger")) {ChangeSkin(46003);}
    if (ImGui::CustomButton("Pro")) {ChangeSkin(46006);}
    if (ImGui::CustomButton("GrandPrix")) {ChangeSkin(46007);}
    if (ImGui::CustomButton("Lizard")) {ChangeSkin(44601);}
    if (ImGui::CustomButton("Samurai")) {ChangeSkin(44603);}
    if (ImGui::CustomButton("New2")) {ChangeSkin(54601);}
    if (ImGui::CustomButton("Revival")) {ChangeSkin(84600);}
    if (ImGui::CustomButton("Wolf")) {ChangeSkin(84624);}
    if (ImGui::CustomButton("Demon")) {ChangeSkin(124600);}
    if (ImGui::CustomButton("RONINmk56")) {ChangeSkin(134600);}
    if (ImGui::CustomButton("Minotaur")) {ChangeSkin(180004);}
    if (ImGui::CustomButton("Sunset")) {ChangeSkin(200001);}
    if (ImGui::CustomButton("Powergame")) {ChangeSkin(220008);}
    if (ImGui::CustomButton("Paladin")) {ChangeSkin(230004);}
    if (ImGui::CustomButton("Serpent")) {ChangeSkin(240022);}
    if (ImGui::CustomButton("NKai")) {ChangeSkin(240044);}
    if (ImGui::CustomButton("Flock")) {ChangeSkin(240091);}
    if (ImGui::CustomButton("Ironclad")) {ChangeSkin(240124);}
    if (ImGui::CustomButton("FuriousSharpshooter")) {ChangeSkin(240141);}
    if (ImGui::CustomButton("Kachi")) {ChangeSkin(240146);}
    if (ImGui::CustomButton("Flex")) {ChangeSkin(240204);}
    if (ImGui::CustomButton("BulletsAndRoses")) {ChangeSkin(240254);}
    if (ImGui::CustomButton("Necromancer ST")) {ChangeSkin(1046002);}
    if (ImGui::CustomButton("Pro ST")) {ChangeSkin(1046006);}
    if (ImGui::CustomButton("GrandPrix ST")) {ChangeSkin(1046007);}
    if (ImGui::CustomButton("Lizard ST")) {ChangeSkin(1044601);}
    if (ImGui::CustomButton("Samurai ST")) {ChangeSkin(1044603);}
    if (ImGui::CustomButton("Revival ST")) {ChangeSkin(1084600);}
    if (ImGui::CustomButton("Wolf ST")) {ChangeSkin(1084624);}
    if (ImGui::CustomButton("Demon ST")) {ChangeSkin(1124600);}
    if (ImGui::CustomButton("RONINmk56 ST")) {ChangeSkin(1134600);}
    if (ImGui::CustomButton("Minotaur ST")) {ChangeSkin(1180004);}
    if (ImGui::CustomButton("Sunset ST")) {ChangeSkin(1200001);}
    if (ImGui::CustomButton("Paladin ST")) {ChangeSkin(1230004);}
    if (ImGui::CustomButton("Serpent ST")) {ChangeSkin(1240022);}
    if (ImGui::CustomButton("NKai ST")) {ChangeSkin(1240044);}
    if (ImGui::CustomButton("Flock ST")) {ChangeSkin(1240091);}
    if (ImGui::CustomButton("Ironclad ST")) {ChangeSkin(1240124);}
    if (ImGui::CustomButton("Kachi ST")) {ChangeSkin(1240146);}
    if (ImGui::CustomButton("Flex ST")) {ChangeSkin(1240204);}
    if (ImGui::CustomButton("Retro Film")) { ChangeSkin(240309); }
    if (ImGui::CustomButton("Retro Film ST")) { ChangeSkin(1240309); }
    if (ImGui::CustomButton("DireWolf")) { ChangeSkin(240332); }
    if (ImGui::CustomButton("DireWolf ST")) { ChangeSkin(1240332); }
    }
    if (m4a1set){
    MenuText("M4A1");
    if (ImGui::CustomButton("Bubblegum")) {ChangeSkin(124300);}
    if (ImGui::CustomButton("Kitsune")) {ChangeSkin(124301);}
    if (ImGui::CustomButton("KINGv703")) {ChangeSkin(134300);}
    if (ImGui::CustomButton("Noname")) {ChangeSkin(144300);}
    if (ImGui::CustomButton("YearOfTheTiger")) {ChangeSkin(154300);}
    if (ImGui::CustomButton("Sour")) {ChangeSkin(170009);}
    if (ImGui::CustomButton("Mermaid")) {ChangeSkin(180018);}
    if (ImGui::CustomButton("Serpent")) {ChangeSkin(240022);}
    if (ImGui::CustomButton("Ferocity")) {ChangeSkin(240082);}
    if (ImGui::CustomButton("Stainless")) {ChangeSkin(240093);}
    if (ImGui::CustomButton("Overdrive")) {ChangeSkin(240193);}
    if (ImGui::CustomButton("Impact")) {ChangeSkin(240210);}
    if (ImGui::CustomButton("Tempest")) {ChangeSkin(240242);}
    if (ImGui::CustomButton("SparklingGaze")) {ChangeSkin(240159);}
    if (ImGui::CustomButton("PawPaw")) {ChangeSkin(1240277);}
    if (ImGui::CustomButton("Bubblegum ST")) {ChangeSkin(1124300);}
    if (ImGui::CustomButton("Kitsune ST")) {ChangeSkin(1124301);}
    if (ImGui::CustomButton("KINGv703 ST")) {ChangeSkin(1134300);}
    if (ImGui::CustomButton("Noname ST")) {ChangeSkin(1144300);}
    if (ImGui::CustomButton("YearOfTheTiger ST")) {ChangeSkin(1154300);}
    if (ImGui::CustomButton("Sour ST")) {ChangeSkin(1170009);}
    if (ImGui::CustomButton("Mermaid ST")) {ChangeSkin(1180018);}
    if (ImGui::CustomButton("Ferocity ST")) {ChangeSkin(1240082);}
    if (ImGui::CustomButton("Stainless ST")) {ChangeSkin(1240093);}
    if (ImGui::CustomButton("Overdrive ST")) {ChangeSkin(1240193);}
    if (ImGui::CustomButton("Impact ST")) {ChangeSkin(1240210);}
    if (ImGui::CustomButton("Tempest ST")) {ChangeSkin(1240242);}
    if (ImGui::CustomButton("SparklingGaze ST")) {ChangeSkin(1240159);}
    if (ImGui::CustomButton("PawPaw ST")) {ChangeSkin(1240277);}
    if (ImGui::CustomButton("Treasure Hunter")) { ChangeSkin(240315); }
    if (ImGui::CustomButton("Treasure Hunter ST")) { ChangeSkin(1240315); }
    if (ImGui::CustomButton("Pixel Flakes")) { ChangeSkin(240285); }
    if (ImGui::CustomButton("Pixel Flakes ST")) { ChangeSkin(1240285); }
    }
    if (m16set){
    MenuText("M16");
    if (ImGui::CustomButton("Camouflage")) {ChangeSkin(47001);}
    if (ImGui::CustomButton("Winged")) {ChangeSkin(47002);}
    if (ImGui::CustomButton("Facet")) {ChangeSkin(47003);}
    if (ImGui::CustomButton("4Years")) {ChangeSkin(104700);}
    if (ImGui::CustomButton("IronWill")) {ChangeSkin(94700);}
    if (ImGui::CustomButton("Needl")) {ChangeSkin(170010);}
    if (ImGui::CustomButton("Muraena")) {ChangeSkin(180019);}
    if (ImGui::CustomButton("RetroArcade")) {ChangeSkin(200007);}
    if (ImGui::CustomButton("Sharpshooter")) {ChangeSkin(210002);}
    if (ImGui::CustomButton("ShogunStripes")) {ChangeSkin(220019);}
    if (ImGui::CustomButton("DustDevil")) {ChangeSkin(240026);}
    if (ImGui::CustomButton("Bigfoot")) {ChangeSkin(240050);}
    if (ImGui::CustomButton("SeaGlint")) {ChangeSkin(240108);}
    if (ImGui::CustomButton("Mayhem")) {ChangeSkin(240186);}
    if (ImGui::CustomButton("Raiva")) {ChangeSkin(240177);}
    if (ImGui::CustomButton("Challenger")) {ChangeSkin(240223);}
    if (ImGui::CustomButton("Basilisk")) {ChangeSkin(1240261);}
    if (ImGui::CustomButton("Winged ST")) {ChangeSkin(1047002);}
    if (ImGui::CustomButton("IronWill ST")) {ChangeSkin(1094700);}
    if (ImGui::CustomButton("Needl ST")) {ChangeSkin(1170010);}
    if (ImGui::CustomButton("Muraena ST")) {ChangeSkin(1180019);}
    if (ImGui::CustomButton("RetroArcade ST")) {ChangeSkin(1200007);}
    if (ImGui::CustomButton("ShogunStripes ST")) {ChangeSkin(1220019);}
    if (ImGui::CustomButton("DustDevil ST")) {ChangeSkin(1240026);}
    if (ImGui::CustomButton("Bigfoot ST")) {ChangeSkin(1240050);}
    if (ImGui::CustomButton("SeaGlint ST")) {ChangeSkin(1240108);}
    if (ImGui::CustomButton("Mayhem ST")) {ChangeSkin(1240186);}
    if (ImGui::CustomButton("Challenger ST")) {ChangeSkin(1240223);}
    if (ImGui::CustomButton("Basilisk ST")) {ChangeSkin(1240261);}
    if (ImGui::CustomButton("Accuracy")) { ChangeSkin(240310); }
    if (ImGui::CustomButton("Accuracy ST")) { ChangeSkin(1240310); }
    }
    if (famasset){
    MenuText("FAMAS");
    if (ImGui::CustomButton("Beagle")) {ChangeSkin(48001);}
    if (ImGui::CustomButton("Fury")) {ChangeSkin(48002);}
    if (ImGui::CustomButton("Hull")) {ChangeSkin(48003);}
    if (ImGui::CustomButton("Monster")) {ChangeSkin(74800);}
    if (ImGui::CustomButton("Autumn")) {ChangeSkin(84800);}
    if (ImGui::CustomButton("Anger")) {ChangeSkin(124800);}
    if (ImGui::CustomButton("Gunsmoke")) {ChangeSkin(170006);}
    if (ImGui::CustomButton("Handcraft")) {ChangeSkin(190003);}
    if (ImGui::CustomButton("StellarBlitz")) {ChangeSkin(240005);}
    if (ImGui::CustomButton("SnowStorm")) {ChangeSkin(240048);}
    if (ImGui::CustomButton("Oxide")) {ChangeSkin(240118);}
    if (ImGui::CustomButton("BattleSteel")) {ChangeSkin(240085);}
    if (ImGui::CustomButton("Ritual")) {ChangeSkin(240216);}
    if (ImGui::CustomButton("Raider")) {ChangeSkin(1240259);}
    if (ImGui::CustomButton("Beagle ST")) {ChangeSkin(1048001);}
    if (ImGui::CustomButton("Fury ST")) {ChangeSkin(1048002);}
    if (ImGui::CustomButton("Hull ST")) {ChangeSkin(1048003);}
    if (ImGui::CustomButton("Monster ST")) {ChangeSkin(1074800);}
    if (ImGui::CustomButton("Autumn ST")) {ChangeSkin(1084800);}
    if (ImGui::CustomButton("Anger ST")) {ChangeSkin(1124800);}
    if (ImGui::CustomButton("Handcraft ST")) {ChangeSkin(1190003);}
    if (ImGui::CustomButton("StellarBlitz ST")) {ChangeSkin(1240005);}
    if (ImGui::CustomButton("SnowStorm ST")) {ChangeSkin(1240048);}
    if (ImGui::CustomButton("Oxide ST")) {ChangeSkin(1240118);}
    if (ImGui::CustomButton("Ritual ST")) {ChangeSkin(1240216);}
    if (ImGui::CustomButton("Raider ST")) {ChangeSkin(1240259);}
    }
    if (fnfalset){
    MenuText("FnFal");
    if (ImGui::CustomButton("Leather")) {ChangeSkin(44901);}
    if (ImGui::CustomButton("AcidCarbon")) {ChangeSkin(44902);}
    if (ImGui::CustomButton("Tactical")) {ChangeSkin(44903);}
    if (ImGui::CustomButton("Phoenix")) {ChangeSkin(84900);}
    if (ImGui::CustomButton("Green")) {ChangeSkin(84906);}
    if (ImGui::CustomButton("ChristmasSymbol")) {ChangeSkin(154900);}
    if (ImGui::CustomButton("Basilisk")) {ChangeSkin(180017);}
    if (ImGui::CustomButton("RedHot")) {ChangeSkin(220006);}
    if (ImGui::CustomButton("Scaleskitter")) {ChangeSkin(240017);}
    if (ImGui::CustomButton("Verglas")) {ChangeSkin(240047);}
    if (ImGui::CustomButton("DuskTide")) {ChangeSkin(240106);}
    if (ImGui::CustomButton("AstralRift")) {ChangeSkin(240138);}
    if (ImGui::CustomButton("CharredWood")) {ChangeSkin(240168);}
    if (ImGui::CustomButton("Pro")) {ChangeSkin(240188);}
    if (ImGui::CustomButton("Splash")) {ChangeSkin(240224);}
    if (ImGui::CustomButton("Tactical ST")) {ChangeSkin(1044903);}
    if (ImGui::CustomButton("Phoenix ST")) {ChangeSkin(1084900);}
    if (ImGui::CustomButton("ChristmasSymbol ST")) {ChangeSkin(1154900);}
    if (ImGui::CustomButton("Basilisk ST")) {ChangeSkin(1180017);}
    if (ImGui::CustomButton("Scaleskitter ST")) {ChangeSkin(1240017);}
    if (ImGui::CustomButton("Verglas ST")) {ChangeSkin(1240047);}
    if (ImGui::CustomButton("DuskTide ST")) {ChangeSkin(1240106);}
    if (ImGui::CustomButton("AstralRift ST")) {ChangeSkin(1240138);}
    if (ImGui::CustomButton("CharredWood ST")) {ChangeSkin(1240168);}
    if (ImGui::CustomButton("Pro ST")) {ChangeSkin(1240188);}
    if (ImGui::CustomButton("Splash ST")) {ChangeSkin(1240224);}
    if (ImGui::CustomButton("Empyrean")) { ChangeSkin(240280); }
    if (ImGui::CustomButton("Empyrean ST")) { ChangeSkin(1240280); }
    }
    if (awmset){
    MenuText("AWM");
    if (ImGui::CustomButton("Sport")) {ChangeSkin(51001);}
    if (ImGui::CustomButton("Phoenix")) {ChangeSkin(51002);}
    if (ImGui::CustomButton("Gear")) {ChangeSkin(51003);}
    if (ImGui::CustomButton("Scratch")) {ChangeSkin(51004);}
    if (ImGui::CustomButton("SportV2")) {ChangeSkin(51006);}
    if (ImGui::CustomButton("Genesis")) {ChangeSkin(51007);}
    if (ImGui::CustomButton("2YearsRed")) {ChangeSkin(51008);}
    if (ImGui::CustomButton("TreasureHunter")) {ChangeSkin(85104);}
    if (ImGui::CustomButton("Dragon")) {ChangeSkin(85127);}
    if (ImGui::CustomButton("PolarNight")) {ChangeSkin(95100);}
    if (ImGui::CustomButton("BOOM")) {ChangeSkin(125100);}
    if (ImGui::CustomButton("Elevation")) {ChangeSkin(125101);}
    if (ImGui::CustomButton("Stickerbomb")) {ChangeSkin(165100);}
    if (ImGui::CustomButton("Poseidon")) {ChangeSkin(180002);}
    if (ImGui::CustomButton("Kings")) {ChangeSkin(220012);}
    if (ImGui::CustomButton("Xenoguard")) {ChangeSkin(240006);}
    if (ImGui::CustomButton("Nebula")) {ChangeSkin(240013);}
    if (ImGui::CustomButton("FestalWrap")) {ChangeSkin(240053);}
    if (ImGui::CustomButton("HoheiTaisho")) {ChangeSkin(240102);}
    if (ImGui::CustomButton("Spectral")) {ChangeSkin(240126);}
    if (ImGui::CustomButton("Sylvan")) {ChangeSkin(240190);}
    if (ImGui::CustomButton("Ravage")) {ChangeSkin(240219);}
    if (ImGui::CustomButton("Moonstone")) {ChangeSkin(240244);}
    if (ImGui::CustomButton("Vampire")) {ChangeSkin(1240260);}
    if (ImGui::CustomButton("Phoenix ST")) {ChangeSkin(1051002);}
    if (ImGui::CustomButton("Gear ST")) {ChangeSkin(1051003);}
    if (ImGui::CustomButton("Scratch ST")) {ChangeSkin(1051004);}
    if (ImGui::CustomButton("Genesis ST")) {ChangeSkin(1051007);}
    if (ImGui::CustomButton("Dragon ST")) {ChangeSkin(1085127);}
    if (ImGui::CustomButton("PolarNight ST")) {ChangeSkin(1095100);}
    if (ImGui::CustomButton("BOOM ST")) {ChangeSkin(1125100);}
    if (ImGui::CustomButton("Poseidon ST")) {ChangeSkin(1180002);}
    if (ImGui::CustomButton("Kings ST")) {ChangeSkin(1220012);}
    if (ImGui::CustomButton("Xenogard ST")) {ChangeSkin(1240006);}
    if (ImGui::CustomButton("FestalWrap ST")) {ChangeSkin(1240053);}
    if (ImGui::CustomButton("HoheiTaisho ST")) {ChangeSkin(1240102);}
    if (ImGui::CustomButton("Spectral ST")) {ChangeSkin(1240126);}
    if (ImGui::CustomButton("Sylvan ST")) {ChangeSkin(1240190);}
    if (ImGui::CustomButton("Ravage ST")) {ChangeSkin(1240219);}
    if (ImGui::CustomButton("Moonstone ST")) {ChangeSkin(1240244);}
    if (ImGui::CustomButton("Vampire ST")) {ChangeSkin(1240260);}
    if (ImGui::CustomButton("JadeStone")) {ChangeSkin(240328);}
    if (ImGui::CustomButton("JadeStone ST")) {ChangeSkin(1240328);}
    }
    if (m40set){
    MenuText("M40");
    if (ImGui::CustomButton("Quake")) {ChangeSkin(52001);}
    if (ImGui::CustomButton("Pro")) {ChangeSkin(52002);}
    if (ImGui::CustomButton("Beagle")) {ChangeSkin(52003);}
    if (ImGui::CustomButton("New1")) {ChangeSkin(65201);}
    if (ImGui::CustomButton("New2")) {ChangeSkin(65202);}
    if (ImGui::CustomButton("Monster")) {ChangeSkin(75205);}
    if (ImGui::CustomButton("Stickerbomb")) {ChangeSkin(165200);}
    if (ImGui::CustomButton("Grip")) {ChangeSkin(170011);}
    if (ImGui::CustomButton("Constellations")) {ChangeSkin(180005);}
    if (ImGui::CustomButton("Impale")) {ChangeSkin(220003);}
    if (ImGui::CustomButton("Wyvern")) {ChangeSkin(1230005);}
    if (ImGui::CustomButton("VenomShade")) {ChangeSkin(240040);}
    if (ImGui::CustomButton("Disguise")) {ChangeSkin(240096);}
    if (ImGui::CustomButton("Daybreak")) {ChangeSkin(240103);}
    if (ImGui::CustomButton("Skyfall")) {ChangeSkin(240121);}
    if (ImGui::CustomButton("Smolder")) {ChangeSkin(240178);}
    if (ImGui::CustomButton("NorthernLights")) {ChangeSkin(240149);}
    if (ImGui::CustomButton("WhiteAsh")) {ChangeSkin(240248);}
    if (ImGui::CustomButton("Quake ST")) {ChangeSkin(1052001);}
    if (ImGui::CustomButton("Beagle ST")) {ChangeSkin(1052003);}
    if (ImGui::CustomButton("New1 ST")) {ChangeSkin(1065201);}
    if (ImGui::CustomButton("New2 ST")) {ChangeSkin(1065202);}
    if (ImGui::CustomButton("Monster ST")) {ChangeSkin(1075205);}
    if (ImGui::CustomButton("Grip ST")) {ChangeSkin(1170011);}
    if (ImGui::CustomButton("Constellations ST")) {ChangeSkin(1180005);}
    if (ImGui::CustomButton("Wyvern ST")) {ChangeSkin(1230005);}
    if (ImGui::CustomButton("VenomShade ST")) {ChangeSkin(1240040);}
    if (ImGui::CustomButton("Disguise ST")) {ChangeSkin(1240096);}
    if (ImGui::CustomButton("Daybreak ST")) {ChangeSkin(1240103);}
    if (ImGui::CustomButton("Skyfall ST")) {ChangeSkin(1240121);}
    if (ImGui::CustomButton("NorthernLights ST")) {ChangeSkin(1240149);}
    if (ImGui::CustomButton("WhiteAsh ST")) {ChangeSkin(1240248);}
    if (ImGui::CustomButton("Auric Bloom")) { ChangeSkin(240279); }
    if (ImGui::CustomButton("Auric Bloom ST")) { ChangeSkin(1240279); }
    }
    if (m110set){
    MenuText("M110");
    if (ImGui::CustomButton("Cyber")) {ChangeSkin(45301);}
    if (ImGui::CustomButton("Transition")) {ChangeSkin(85300);}
    if (ImGui::CustomButton("Flow")) {ChangeSkin(95300);}
    if (ImGui::CustomButton("Stickerbomb")) {ChangeSkin(165300);}
    if (ImGui::CustomButton("Pursuit")) {ChangeSkin(170013);}
    if (ImGui::CustomButton("Themis")) {ChangeSkin(180007);}
    if (ImGui::CustomButton("TechShard")) {ChangeSkin(220004);}
    if (ImGui::CustomButton("Harbinger")) {ChangeSkin(240023);}
    if (ImGui::CustomButton("ReachKeeper")) {ChangeSkin(240073);}
    if (ImGui::CustomButton("DarkPact")) {ChangeSkin(240134);}
    if (ImGui::CustomButton("Grid")) {ChangeSkin(240170);}
    if (ImGui::CustomButton("Centipede")) {ChangeSkin(1240265);}
    if (ImGui::CustomButton("Cyber ST")) {ChangeSkin(1045301);}
    if (ImGui::CustomButton("Transition ST")) {ChangeSkin(1085300);}
    if (ImGui::CustomButton("Flow ST")) {ChangeSkin(1095300);}
    if (ImGui::CustomButton("Themis ST")) {ChangeSkin(1180007);}
    if (ImGui::CustomButton("Harbinger ST")) {ChangeSkin(1240023);}
    if (ImGui::CustomButton("ReachKeeper ST")) {ChangeSkin(1240073);}
    if (ImGui::CustomButton("DarkPact ST")) {ChangeSkin(1240134);}
    if (ImGui::CustomButton("Grid ST")) {ChangeSkin(1240170);}
    if (ImGui::CustomButton("Centipede ST")) {ChangeSkin(1240265);}
    }
    if (mallardset){
    MenuText("Mallard");
    if (ImGui::CustomButton("Ascend")) { ChangeSkin(240236); }
    if (ImGui::CustomButton("Ascend ST")) { ChangeSkin(1240236); }
    if (ImGui::CustomButton("Falling Leaves")) { ChangeSkin(240243); }
    if (ImGui::CustomButton("Falling Leaves ST")) { ChangeSkin(1240243); }
    if (ImGui::CustomButton("Phantom Pack")) { ChangeSkin(240252); }
    if (ImGui::CustomButton("Phantom Pack ST")) { ChangeSkin(1240252); }
    if (ImGui::CustomButton("PRO Gold")) { ChangeSkin(240276); }
    if (ImGui::CustomButton("Wild West")) { ChangeSkin(240282); }
    if (ImGui::CustomButton("Wild West ST")) { ChangeSkin(1240282); }
    if (ImGui::CustomButton("Paint Scrawl")) { ChangeSkin(240303); }
    if (ImGui::CustomButton("Snake Scales")) { ChangeSkin(240314); }
    if (ImGui::CustomButton("Snake Scales ST")) { ChangeSkin(1240314); }
    if (ImGui::CustomButton("Ink_1")) { ChangeSkin(10240325); }
    if (ImGui::CustomButton("Ink_2")) { ChangeSkin(20240325); }
    if (ImGui::CustomButton("Ink_3")) { ChangeSkin(30240325); }
    if (ImGui::CustomButton("Ink_1 ST")) { ChangeSkin(11240325); }
    if (ImGui::CustomButton("Ink_2 ST")) { ChangeSkin(21240325); }
    if (ImGui::CustomButton("Ink_3 ST")) { ChangeSkin(31240325); }
    if (ImGui::CustomButton("PolarBear")) { ChangeSkin(240341); }
    if (ImGui::CustomButton("PolarBear ST")) { ChangeSkin(1240341); }
    }
    if (sm1014set){
    MenuText("SM1014");
    if (ImGui::CustomButton("Facet")) {ChangeSkin(62001);}
    if (ImGui::CustomButton("Pathfinder")) {ChangeSkin(62002);}
    if (ImGui::CustomButton("Necromancer")) {ChangeSkin(62003);}
    if (ImGui::CustomButton("NorthernCamouflage")) {ChangeSkin(62004);}
    if (ImGui::CustomButton("Quake")) {ChangeSkin(62005);}
    if (ImGui::CustomButton("Branches")) {ChangeSkin(62006);}
    if (ImGui::CustomButton("Blaster")) {ChangeSkin(45302);}
    if (ImGui::CustomButton("New1")) {ChangeSkin(66201);}
    if (ImGui::CustomButton("Shark")) {ChangeSkin(86216);}
    if (ImGui::CustomButton("Bolt")) {ChangeSkin(170014);}
    if (ImGui::CustomButton("Shark2")) {ChangeSkin(180014);}
    if (ImGui::CustomButton("Tropic")) {ChangeSkin(126201);}
    if (ImGui::CustomButton("Serpent")) {ChangeSkin(240021);}
    if (ImGui::CustomButton("FreezingGasp")) {ChangeSkin(240055);}
    if (ImGui::CustomButton("Triton")) {ChangeSkin(240104);}
    if (ImGui::CustomButton("Wasp")) {ChangeSkin(240189);}
    if (ImGui::CustomButton("Piranha")) {ChangeSkin(240215);}
    if (ImGui::CustomButton("FallingLeaves")) {ChangeSkin(240246);}
    if (ImGui::CustomButton("Pathfinder ST")) {ChangeSkin(1062002);}
    if (ImGui::CustomButton("Necromancer ST")) {ChangeSkin(1062003);}
    if (ImGui::CustomButton("New1 ST")) {ChangeSkin(1066201);}
    if (ImGui::CustomButton("Shark ST")) {ChangeSkin(1086216);}
    if (ImGui::CustomButton("Bolt ST")) {ChangeSkin(1170014);}
    if (ImGui::CustomButton("Shark2 ST")) {ChangeSkin(1180014);}
    if (ImGui::CustomButton("Serpent ST")) {ChangeSkin(1240021);}
    if (ImGui::CustomButton("FreezingGasp ST")) {ChangeSkin(1240055);}
    if (ImGui::CustomButton("Triton ST")) {ChangeSkin(1240104);}
    if (ImGui::CustomButton("Wasp ST")) {ChangeSkin(1240189);}
    if (ImGui::CustomButton("Piranha ST")) {ChangeSkin(1240215);}
    if (ImGui::CustomButton("FallingLeaves ST")) {ChangeSkin(1240246);}
    }
    if (fabmset){
    MenuText("FabM");
    if (ImGui::CustomButton("Reactor")) {ChangeSkin(76218);}
    if (ImGui::CustomButton("Flight")) {ChangeSkin(86300);}
    if (ImGui::CustomButton("Green")) {ChangeSkin(86317);}
    if (ImGui::CustomButton("Parrot")) {ChangeSkin(86318);}
    if (ImGui::CustomButton("BOOM")) {ChangeSkin(170005);}
    if (ImGui::CustomButton("Hercules")) {ChangeSkin(180003);}
    if (ImGui::CustomButton("Cranium")) {ChangeSkin(240002);}
    if (ImGui::CustomButton("Tindalos")) {ChangeSkin(240038);}
    if (ImGui::CustomButton("Vein")) {ChangeSkin(240083);}
    if (ImGui::CustomButton("Gangster")) {ChangeSkin(240125);}
    if (ImGui::CustomButton("Mayhem")) {ChangeSkin(240182);}
    if (ImGui::CustomButton("Streak")) {ChangeSkin(240203);}
    if (ImGui::CustomButton("Undertow")) {ChangeSkin(240239);}
    if (ImGui::CustomButton("Fiend")) {ChangeSkin(1240266);}
    if (ImGui::CustomButton("Reactor ST")) {ChangeSkin(1076218);}
    if (ImGui::CustomButton("Flight ST")) {ChangeSkin(1086300);}
    if (ImGui::CustomButton("Parrot ST")) {ChangeSkin(1086318);}
    if (ImGui::CustomButton("BOOM ST")) {ChangeSkin(1170005);}
    if (ImGui::CustomButton("Hercules ST")) {ChangeSkin(1180003);}
    if (ImGui::CustomButton("Cranium ST")) {ChangeSkin(1240002);}
    if (ImGui::CustomButton("Tindalos ST")) {ChangeSkin(1240038);}
    if (ImGui::CustomButton("Gangster ST")) {ChangeSkin(1240125);}
    if (ImGui::CustomButton("Streak ST")) {ChangeSkin(1240203);}
    if (ImGui::CustomButton("Fiend ST")) {ChangeSkin(1240266);}
    if (ImGui::CustomButton("CloudLily_1")) {ChangeSkin(10240323);}
    if (ImGui::CustomButton("CloudLily_2")) {ChangeSkin(20240323);}
    if (ImGui::CustomButton("CloudLily_3")) {ChangeSkin(30240323);}
    if (ImGui::CustomButton("CloudLily_1 ST")) {ChangeSkin(11240323);}
    if (ImGui::CustomButton("CloudLily_2 ST")) {ChangeSkin(21240323);}
    if (ImGui::CustomButton("CloudLily_3 ST")) {ChangeSkin(31240323);}
    }
    if (m60set){
    MenuText("M60");
    if (ImGui::CustomButton("Grunge")) {ChangeSkin(126400);}
    if (ImGui::CustomButton("SteamBeast")) {ChangeSkin(126401);}
    if (ImGui::CustomButton("Mecha")) {ChangeSkin(136400);}
    if (ImGui::CustomButton("Y20RAIJIN")) {ChangeSkin(136401);}
    if (ImGui::CustomButton("Turret")) {ChangeSkin(170012);}
    if (ImGui::CustomButton("Ares")) {ChangeSkin(180006);}
    if (ImGui::CustomButton("Spaceware")) {ChangeSkin(240003);}
    if (ImGui::CustomButton("Quantum")) {ChangeSkin(240036);}
    if (ImGui::CustomButton("Fatal")) {ChangeSkin(240077);}
    if (ImGui::CustomButton("Warp")) {ChangeSkin(240169);}
    if (ImGui::CustomButton("Copper")) {ChangeSkin(240185);}
    if (ImGui::CustomButton("Flex")) {ChangeSkin(240205);}
    if (ImGui::CustomButton("Horror")) {ChangeSkin(240258);}
    if (ImGui::CustomButton("Grunge ST")) {ChangeSkin(1126400);}
    if (ImGui::CustomButton("SteamBeast ST")) {ChangeSkin(1126401);}
    if (ImGui::CustomButton("Mecha ST")) {ChangeSkin(1136400);}
    if (ImGui::CustomButton("Y20RAIJIN ST")) {ChangeSkin(1136401);}
    if (ImGui::CustomButton("Turret ST")) {ChangeSkin(1170012);}
    if (ImGui::CustomButton("Ares ST")) {ChangeSkin(1180006);}
    if (ImGui::CustomButton("Spaceware ST")) {ChangeSkin(1240003);}
    if (ImGui::CustomButton("Quantum ST")) {ChangeSkin(1240036);}
    if (ImGui::CustomButton("Fatal ST")) {ChangeSkin(1240077);}
    if (ImGui::CustomButton("Warp ST")) {ChangeSkin(1240169);}
    if (ImGui::CustomButton("Flex ST")) {ChangeSkin(1240205);}
    if (ImGui::CustomButton("Horror ST")) {ChangeSkin(1240258);}
    }
    if (valset){
    MenuText("VAL");
    if (ImGui::CustomButton("Forge")) {ChangeSkin(240074);}
    if (ImGui::CustomButton("OilSheen")) {ChangeSkin(240080);}
    if (ImGui::CustomButton("Joker")) {ChangeSkin(240094);}
    if (ImGui::CustomButton("GildedGale")) {ChangeSkin(240101);}
    if (ImGui::CustomButton("Solstice")) {ChangeSkin(240107);}
    if (ImGui::CustomButton("CityNight")) {ChangeSkin(240122);}
    if (ImGui::CustomButton("WidowsWeave")) {ChangeSkin(240139);}
    if (ImGui::CustomButton("Ronin")) {ChangeSkin(240158);}
    if (ImGui::CustomButton("Orbit")) {ChangeSkin(240218);}
    if (ImGui::CustomButton("RosaMortal")) {ChangeSkin(240256);}
    if (ImGui::CustomButton("PhantomPack")) {ChangeSkin(240267);}
    if (ImGui::CustomButton("Forge ST")) {ChangeSkin(1240074);}
    if (ImGui::CustomButton("OilSheen ST")) {ChangeSkin(1240080);}
    if (ImGui::CustomButton("Joker ST")) {ChangeSkin(1240094);}
    if (ImGui::CustomButton("GildedGale ST")) {ChangeSkin(1240101);}
    if (ImGui::CustomButton("Solstice ST")) {ChangeSkin(1240107);}
    if (ImGui::CustomButton("CityNight ST")) {ChangeSkin(1240122);}
    if (ImGui::CustomButton("WidowsWeave ST")) {ChangeSkin(1240139);}
    if (ImGui::CustomButton("Ronin ST")) {ChangeSkin(1240158);}
    if (ImGui::CustomButton("Orbit ST")) {ChangeSkin(1240218);}
    if (ImGui::CustomButton("RosaMortal ST")) {ChangeSkin(1240256);}
    if (ImGui::CustomButton("PhantomPack ST")) {ChangeSkin(1240267);}
    if (ImGui::CustomButton("Vertex")) { ChangeSkin(240306); }
    if (ImGui::CustomButton("Vertex ST")) { ChangeSkin(1240306); }
    if (ImGui::CustomButton("IronWolf")) { ChangeSkin(240337); }
    if (ImGui::CustomButton("IronWolf ST")) { ChangeSkin(1240337); }
    }
    if (mac10set){
    MenuText("MAC10");
    if (ImGui::CustomButton("Argo")) {ChangeSkin(180008);}
    if (ImGui::CustomButton("Constellations")) {ChangeSkin(180009);}
    if (ImGui::CustomButton("Shark")) {ChangeSkin(180020);}
    if (ImGui::CustomButton("MeltAway")) {ChangeSkin(190004);}
    if (ImGui::CustomButton("PurpleMist")) {ChangeSkin(200008);}
    if (ImGui::CustomButton("Noxious")) {ChangeSkin(220001);}
    if (ImGui::CustomButton("WildRage")) {ChangeSkin(230006);}
    if (ImGui::CustomButton("Arid")) {ChangeSkin(240019);}
    if (ImGui::CustomButton("Corrode")) {ChangeSkin(240071);}
    if (ImGui::CustomButton("RubyShadow")) {ChangeSkin(240115);}
    if (ImGui::CustomButton("ArcaneSurge")) {ChangeSkin(240137);}
    if (ImGui::CustomButton("Tempest")) {ChangeSkin(240247);}
    if (ImGui::CustomButton("PixelCamouflage")) {ChangeSkin(240179);}
    if (ImGui::CustomButton("Argo ST")) {ChangeSkin(1180008);}
    if (ImGui::CustomButton("Constellations ST")) {ChangeSkin(1180009);}
    if (ImGui::CustomButton("Shark ST")) {ChangeSkin(1180020);}
    if (ImGui::CustomButton("MeltAway ST")) {ChangeSkin(1190004);}
    if (ImGui::CustomButton("PurpleMist ST")) {ChangeSkin(1200008);}
    if (ImGui::CustomButton("Noxious ST")) {ChangeSkin(1220001);}
    if (ImGui::CustomButton("WildRage ST")) {ChangeSkin(1230006);}
    if (ImGui::CustomButton("Arid ST")) {ChangeSkin(1240019);}
    if (ImGui::CustomButton("Corrode ST")) {ChangeSkin(1240071);}
    if (ImGui::CustomButton("ArcaneSurge ST")) {ChangeSkin(1240137);}
    if (ImGui::CustomButton("Tempest ST")) {ChangeSkin(1240247);}
    if (ImGui::CustomButton("TopSecret")) {ChangeSkin(240336);}
    if (ImGui::CustomButton("Tempest ST")) {ChangeSkin(1240336);}
    }
    if (spasset){
    MenuText("SPAS");
    if (ImGui::CustomButton("Griffin")) {ChangeSkin(180011);}
    if (ImGui::CustomButton("Zeus")) {ChangeSkin(180012);}
    if (ImGui::CustomButton("Octopus")) {ChangeSkin(180021);}
    if (ImGui::CustomButton("Raider")) {ChangeSkin(190006);}
    if (ImGui::CustomButton("Vibe")) {ChangeSkin(200003);}
    if (ImGui::CustomButton("Taint")) {ChangeSkin(1220018);}
    if (ImGui::CustomButton("Unicorn")) {ChangeSkin(1230008);}
    if (ImGui::CustomButton("VesperHaze")) {ChangeSkin(220005);}
    if (ImGui::CustomButton("Dangerous")) {ChangeSkin(240078);}
    if (ImGui::CustomButton("Tensai")) {ChangeSkin(240157);}
    if (ImGui::CustomButton("Glide")) {ChangeSkin(240176);}
    if (ImGui::CustomButton("EternalFlow")) {ChangeSkin(1240257);}
    if (ImGui::CustomButton("Griffin ST")) {ChangeSkin(1180011);}
    if (ImGui::CustomButton("Zeus ST")) {ChangeSkin(1180012);}
    if (ImGui::CustomButton("Octopus ST")) {ChangeSkin(1180021);}
    if (ImGui::CustomButton("Raider ST")) {ChangeSkin(1190006);}
    if (ImGui::CustomButton("Vibe ST")) {ChangeSkin(1200003);}
    if (ImGui::CustomButton("Taint ST")) {ChangeSkin(1220018);}
    if (ImGui::CustomButton("Unicorn ST")) {ChangeSkin(1230008);}
    if (ImGui::CustomButton("VesperHaze ST")) {ChangeSkin(1220005);}
    if (ImGui::CustomButton("Dangerous ST")) {ChangeSkin(1240078);}
    if (ImGui::CustomButton("Tensai ST")) {ChangeSkin(1240157);}
    if (ImGui::CustomButton("Glide ST")) {ChangeSkin(1240176);}
    if (ImGui::CustomButton("EternalFlow ST")) {ChangeSkin(1240257);}
    if (ImGui::CustomButton("Waypoint")) {ChangeSkin(240340);}
    if (ImGui::CustomButton("Waypoint ST")) {ChangeSkin(1240340);}
    }
    if (akimbouziset){
    MenuText("AkimboUzi");
    if (ImGui::CustomButton("Spray")) {ChangeSkin(240172);}
    if (ImGui::CustomButton("Toco")) {ChangeSkin(240175);}
    if (ImGui::CustomButton("Skull")) {ChangeSkin(240187);}
    if (ImGui::CustomButton("OverdriveRed")) {ChangeSkin(240196);}
    if (ImGui::CustomButton("Streak")) {ChangeSkin(240202);}
    if (ImGui::CustomButton("Ravage")) {ChangeSkin(240217);}
    if (ImGui::CustomButton("Yokai")) {ChangeSkin(240234);}
    if (ImGui::CustomButton("HowlingGhost")) {ChangeSkin(240255);}
    if (ImGui::CustomButton("Spray ST")) {ChangeSkin(1240172);}
    if (ImGui::CustomButton("Toco ST")) {ChangeSkin(1240175);}
    if (ImGui::CustomButton("Skull ST")) {ChangeSkin(1240187);}
    if (ImGui::CustomButton("OverdriveRed ST")) {ChangeSkin(1240196);}
    if (ImGui::CustomButton("Streak ST")) {ChangeSkin(1240202);}
    if (ImGui::CustomButton("Ravage ST")) {ChangeSkin(1240217);}
    if (ImGui::CustomButton("Yokai ST")) {ChangeSkin(1240234);}
    if (ImGui::CustomButton("HowlingGhost ST")) {ChangeSkin(1240255);}
    if (ImGui::CustomButton("Zenith")) { ChangeSkin(240311); }
    if (ImGui::CustomButton("Zenith ST")) { ChangeSkin(1240311); }
    if (ImGui::CustomButton("Fire Horse")) { ChangeSkin(240289); }
    if (ImGui::CustomButton("Fire Horse ST")) { ChangeSkin(1240289); }
    if (ImGui::CustomButton("NoEscape")) { ChangeSkin(240343); }
    if (ImGui::CustomButton("NoEscape ST")) { ChangeSkin(1240343); }
    }
    if (grenadeset){
    MenuText("Grenades");
    if (ImGui::CustomButton("Flash_Solar")) {ChangeSkin(240144);}
    if (ImGui::CustomButton("Smoke_Kiri")) {ChangeSkin(240160);}
    if (ImGui::CustomButton("HE_Hanami")) {ChangeSkin(240161);}
    if (ImGui::CustomButton("Thermite_ScorchedEarth")) {ChangeSkin(240268);}
    if (ImGui::CustomButton("Molotov_Drive")) {ChangeSkin(240208);}
    }
    if (medalset){
    MenuText("Medals");
    if (ImGui::CustomButton("AssistanceBronze")) {ChangeSkin(100);}
    if (ImGui::CustomButton("AssistanceSilver")) {ChangeSkin(101);}
    if (ImGui::CustomButton("AssistanceGold")) {ChangeSkin(102);}
    if (ImGui::CustomButton("AssistancePlatinum")) {ChangeSkin(103);}
    if (ImGui::CustomButton("AssistanceBrilliant")) {ChangeSkin(104);}
    if (ImGui::CustomButton("Veteran2018Bronze")) {ChangeSkin(105);}
    if (ImGui::CustomButton("Veteran2018Silver")) {ChangeSkin(106);}
    if (ImGui::CustomButton("Veteran2018Gold")) {ChangeSkin(107);}
    if (ImGui::CustomButton("Veteran2018Platinum")) {ChangeSkin(108);}
    if (ImGui::CustomButton("Veteran2019Bronze")) {ChangeSkin(109);}
    if (ImGui::CustomButton("Veteran2019Silver")) {ChangeSkin(110);}
    if (ImGui::CustomButton("Veteran2019Gold")) {ChangeSkin(111);}
    if (ImGui::CustomButton("Veteran2019Platinum")) {ChangeSkin(112);}
    if (ImGui::CustomButton("2YearsSilver")) {ChangeSkin(113);}
    if (ImGui::CustomButton("2YearsGold")) {ChangeSkin(114);}
    if (ImGui::CustomButton("CompetitiveBronze")) {ChangeSkin(115);}
    if (ImGui::CustomButton("CompetitiveSilver")) {ChangeSkin(116);}
    if (ImGui::CustomButton("CompetitiveGold")) {ChangeSkin(117);}
    if (ImGui::CustomButton("CompetitivePlatinum")) {ChangeSkin(118);}
    if (ImGui::CustomButton("CompetitiveBrilliant")) {ChangeSkin(119);}
    if (ImGui::CustomButton("NewYearMadness2020Bronze")) {ChangeSkin(120);}
    if (ImGui::CustomButton("NewYearMadness2020Silver")) {ChangeSkin(121);}
    if (ImGui::CustomButton("NewYearMadness2020Gold")) {ChangeSkin(122);}
    if (ImGui::CustomButton("NewYearMadness2020Platinum")) {ChangeSkin(123);}
    if (ImGui::CustomButton("NewYearMadness2020Brilliant")) {ChangeSkin(124);}
    if (ImGui::CustomButton("Veteran2020Bronze")) {ChangeSkin(125);}
    if (ImGui::CustomButton("Veteran2020Silver")) {ChangeSkin(126);}
    if (ImGui::CustomButton("Veteran2020Gold")) {ChangeSkin(127);}
    if (ImGui::CustomButton("Veteran2020Platinum")) {ChangeSkin(128);}
    if (ImGui::CustomButton("ProjectZ9Bronze")) {ChangeSkin(129);}
    if (ImGui::CustomButton("ProjectZ9Silver")) {ChangeSkin(130);}
    if (ImGui::CustomButton("ProjectZ9Gold")) {ChangeSkin(131);}
    if (ImGui::CustomButton("ProjectZ9Platinum")) {ChangeSkin(132);}
    if (ImGui::CustomButton("ProjectZ9Brilliant")) {ChangeSkin(133);}
    if (ImGui::CustomButton("BadgeRevivalBronze")) {ChangeSkin(134);}
    if (ImGui::CustomButton("BadgeRevivalSilver")) {ChangeSkin(135);}
    if (ImGui::CustomButton("BadgeRevivalGold")) {ChangeSkin(136);}
    if (ImGui::CustomButton("BadgeRevivalEliteGold")) {ChangeSkin(137);}
    if (ImGui::CustomButton("BadgeRevivalPlatinum")) {ChangeSkin(138);}
    if (ImGui::CustomButton("BadgeRevivalDiamond")) {ChangeSkin(139);}
    if (ImGui::CustomButton("BadgeNewYear2021Bronze")) {ChangeSkin(140);}
    if (ImGui::CustomButton("BadgeNewYear2021Silver")) {ChangeSkin(141);}
    if (ImGui::CustomButton("BadgeNewYear2021Gold")) {ChangeSkin(142);}
    if (ImGui::CustomButton("BadgeNewYear2021EliteGold")) {ChangeSkin(143);}
    if (ImGui::CustomButton("BadgeNewYear2021Platinum")) {ChangeSkin(144);}
    if (ImGui::CustomButton("BadgeNewYear2021Diamond")) {ChangeSkin(145);}
    if (ImGui::CustomButton("Veteran2021Bronze")) {ChangeSkin(146);}
    if (ImGui::CustomButton("Veteran2021Silver")) {ChangeSkin(147);}
    if (ImGui::CustomButton("Veteran2021Gold")) {ChangeSkin(148);}
    if (ImGui::CustomButton("Veteran2021Platinum")) {ChangeSkin(149);}
    if (ImGui::CustomButton("Veteran2021Diamond")) {ChangeSkin(150);}
    if (ImGui::CustomButton("4YearsSilver")) {ChangeSkin(151);}
    if (ImGui::CustomButton("4YearsGold")) {ChangeSkin(152);}
    if (ImGui::CustomButton("BadgeDragonRiseBronze")) {ChangeSkin(153);}
    if (ImGui::CustomButton("BadgeDragonRiseSilver")) {ChangeSkin(154);}
    if (ImGui::CustomButton("BadgeDragonRiseGold")) {ChangeSkin(155);}
    if (ImGui::CustomButton("BadgeDragonRiseEliteGold")) {ChangeSkin(156);}
    if (ImGui::CustomButton("BadgeDragonRisePlatinum")) {ChangeSkin(157);}
    if (ImGui::CustomButton("BadgeDragonRiseRubin")) {ChangeSkin(158);}
    if (ImGui::CustomButton("BadgeCursedSoulsBronze")) {ChangeSkin(159);}
    if (ImGui::CustomButton("BadgeCursedSoulsSilver")) {ChangeSkin(160);}
    if (ImGui::CustomButton("BadgeCursedSoulsGold")) {ChangeSkin(161);}
    if (ImGui::CustomButton("BadgeCursedSoulsEliteGold")) {ChangeSkin(162);}
    if (ImGui::CustomButton("BadgeCursedSoulsPlatinum")) {ChangeSkin(163);}
    if (ImGui::CustomButton("BadgeCursedSoulsDiamond")) {ChangeSkin(164);}
    if (ImGui::CustomButton("BadgeWinterFun2022Bronze")) {ChangeSkin(165);}
    if (ImGui::CustomButton("BadgeWinterFun2022Silver")) {ChangeSkin(166);}
    if (ImGui::CustomButton("BadgeWinterFun2022Gold")) {ChangeSkin(167);}
    if (ImGui::CustomButton("BadgeWinterFun2022EliteGold")) {ChangeSkin(168);}
    if (ImGui::CustomButton("BadgeWinterFun2022Platinum")) {ChangeSkin(169);}
    if (ImGui::CustomButton("BadgeWinterFun2022Diamond")) {ChangeSkin(170);}
    if (ImGui::CustomButton("Veteran2022Bronze")) {ChangeSkin(171);}
    if (ImGui::CustomButton("Veteran2022Silver")) {ChangeSkin(172);}
    if (ImGui::CustomButton("Veteran2022Gold")) {ChangeSkin(173);}
    if (ImGui::CustomButton("Veteran2022Platinum")) {ChangeSkin(174);}
    if (ImGui::CustomButton("Veteran2022Diamond")) {ChangeSkin(175);}
    if (ImGui::CustomButton("FiveYearsSilver")) {ChangeSkin(176);}
    if (ImGui::CustomButton("FiveYearsGold")) {ChangeSkin(177);}
    if (ImGui::CustomButton("LegendsBronze")) {ChangeSkin(178);}
    if (ImGui::CustomButton("LegendsSilver")) {ChangeSkin(179);}
    if (ImGui::CustomButton("LegendsGold")) {ChangeSkin(180);}
    if (ImGui::CustomButton("LegendsEliteGold")) {ChangeSkin(181);}
    if (ImGui::CustomButton("LegendsPlatinum")) {ChangeSkin(182);}
    if (ImGui::CustomButton("LegendsDiamond")) {ChangeSkin(183);}
    if (ImGui::CustomButton("PandoraBronze")) {ChangeSkin(184);}
    if (ImGui::CustomButton("PandoraSilver")) {ChangeSkin(185);}
    if (ImGui::CustomButton("PandoraGold")) {ChangeSkin(186);}
    if (ImGui::CustomButton("PandoraEliteGold")) {ChangeSkin(187);}
    if (ImGui::CustomButton("PandoraPlatinum")) {ChangeSkin(188);}
    if (ImGui::CustomButton("PandoraDiamond")) {ChangeSkin(189);}
    if (ImGui::CustomButton("HotWinterPartyBronze")) {ChangeSkin(190);}
    if (ImGui::CustomButton("HotWinterPartySilver")) {ChangeSkin(191);}
    if (ImGui::CustomButton("HotWinterPartyGold")) {ChangeSkin(192);}
    if (ImGui::CustomButton("HotWinterPartyEliteGold")) {ChangeSkin(193);}
    if (ImGui::CustomButton("HotWinterPartyPlatinum")) {ChangeSkin(194);}
    if (ImGui::CustomButton("HotWinterPartyDiamond")) {ChangeSkin(195);}
    if (ImGui::CustomButton("Veteran2023Bronze")) {ChangeSkin(196);}
    if (ImGui::CustomButton("Veteran2023Silver")) {ChangeSkin(197);}
    if (ImGui::CustomButton("Veteran2023Gold")) {ChangeSkin(198);}
    if (ImGui::CustomButton("Veteran2023EliteGold")) {ChangeSkin(199);}
    if (ImGui::CustomButton("Veteran2023Platinum")) {ChangeSkin(250);}
    if (ImGui::CustomButton("Veteran2023Diamond")) {ChangeSkin(251);}
    if (ImGui::CustomButton("FirebornBronze")) {ChangeSkin(252);}
    if (ImGui::CustomButton("FirebornSilver")) {ChangeSkin(253);}
    if (ImGui::CustomButton("FirebornGold")) {ChangeSkin(254);}
    if (ImGui::CustomButton("FirebornEliteGold")) {ChangeSkin(255);}
    if (ImGui::CustomButton("FirebornPlatinum")) {ChangeSkin(256);}
    if (ImGui::CustomButton("FirebornDiamond")) {ChangeSkin(257);}
    if (ImGui::CustomButton("SpaceVisionBronze")) {ChangeSkin(258);}
    if (ImGui::CustomButton("SpaceVisionSilver")) {ChangeSkin(259);}
    if (ImGui::CustomButton("SpaceVisionGold")) {ChangeSkin(260);}
    if (ImGui::CustomButton("SpaceVisionEliteGold")) {ChangeSkin(261);}
    if (ImGui::CustomButton("SpaceVisionPlatinum")) {ChangeSkin(262);}
    if (ImGui::CustomButton("SpaceVisionDiamond")) {ChangeSkin(263);}
    if (ImGui::CustomButton("SunstrikeBronze")) {ChangeSkin(264);}
    if (ImGui::CustomButton("SunstrikeSilver")) {ChangeSkin(265);}
    if (ImGui::CustomButton("SunstrikeGold")) {ChangeSkin(266);}
    if (ImGui::CustomButton("SunstrikeEliteGold")) {ChangeSkin(267);}
    if (ImGui::CustomButton("SunstrikePlatinum")) {ChangeSkin(268);}
    if (ImGui::CustomButton("SunstrikeDiamond")) {ChangeSkin(269);}
    if (ImGui::CustomButton("SubjectXBronze")) {ChangeSkin(270);}
    if (ImGui::CustomButton("SubjectXSilver")) {ChangeSkin(271);}
    if (ImGui::CustomButton("SubjectXGold")) {ChangeSkin(272);}
    if (ImGui::CustomButton("SubjectXEliteGold")) {ChangeSkin(273);}
    if (ImGui::CustomButton("SubjectXPlatinum")) {ChangeSkin(274);}
    if (ImGui::CustomButton("SubjectXDiamond")) {ChangeSkin(275);}
    if (ImGui::CustomButton("Veteran2024Bronze")) {ChangeSkin(276);}
    if (ImGui::CustomButton("Veteran2024Silver")) {ChangeSkin(277);}
    if (ImGui::CustomButton("Veteran2024Gold")) {ChangeSkin(278);}
    if (ImGui::CustomButton("Veteran2024EliteGold")) {ChangeSkin(279);}
    if (ImGui::CustomButton("Veteran2024Platinum")) {ChangeSkin(280);}
    if (ImGui::CustomButton("Veteran2024Diamond")) {ChangeSkin(281);}
    if (ImGui::CustomButton("FrostyChaosBronze")) {ChangeSkin(282);}
    if (ImGui::CustomButton("FrostyChaosSilver")) {ChangeSkin(283);}
    if (ImGui::CustomButton("FrostyChaosGold")) {ChangeSkin(284);}
    if (ImGui::CustomButton("FrostyChaosEliteGold")) {ChangeSkin(285);}
    if (ImGui::CustomButton("FrostyChaosPlatinum")) {ChangeSkin(286);}
    if (ImGui::CustomButton("FrostyChaosDiamond")) {ChangeSkin(287);}
    if (ImGui::CustomButton("OutcastBronze")) {ChangeSkin(288);}
    if (ImGui::CustomButton("OutcastSilver")) {ChangeSkin(289);}
    if (ImGui::CustomButton("OutcastGold")) {ChangeSkin(290);}
    if (ImGui::CustomButton("OutcastEliteGold")) {ChangeSkin(291);}
    if (ImGui::CustomButton("OutcastPlatinum")) {ChangeSkin(292);}
    if (ImGui::CustomButton("OutcastDiamond")) {ChangeSkin(293);}
    if (ImGui::CustomButton("FunAndSunBronze")) {ChangeSkin(294);}
    if (ImGui::CustomButton("FunAndSunSilver")) {ChangeSkin(295);}
    if (ImGui::CustomButton("FunAndSunGold")) {ChangeSkin(296);}
    if (ImGui::CustomButton("FunAndSunEliteGold")) {ChangeSkin(297);}
    if (ImGui::CustomButton("FunAndSunPlatinum")) {ChangeSkin(298);}
    if (ImGui::CustomButton("FunAndSunDiamond")) {ChangeSkin(299);}
    if (ImGui::CustomButton("ReforgedBronze")) {ChangeSkin(300);}
    if (ImGui::CustomButton("ReforgedSilver")) {ChangeSkin(313);}
    if (ImGui::CustomButton("ReforgedGold")) {ChangeSkin(314);}
    if (ImGui::CustomButton("ReforgedEliteGold")) {ChangeSkin(315);}
    if (ImGui::CustomButton("ReforgedPlatinum")) {ChangeSkin(316);}
    if (ImGui::CustomButton("ReforgedDiamond")) {ChangeSkin(317);}
    if (ImGui::CustomButton("NightmareBronze")) {ChangeSkin(322);}
    if (ImGui::CustomButton("NightmareSilver")) {ChangeSkin(323);}
    if (ImGui::CustomButton("NightmareGold")) {ChangeSkin(324);}
    if (ImGui::CustomButton("NightmareEliteGold")) {ChangeSkin(325);}
    if (ImGui::CustomButton("NightmarePlatinum")) {ChangeSkin(326);}
    if (ImGui::CustomButton("NightmareDiamond")) {ChangeSkin(327);}
    if (ImGui::CustomButton("Veteran2025Bronze")) {ChangeSkin(334);}
    if (ImGui::CustomButton("Veteran2025Silver")) {ChangeSkin(335);}
    if (ImGui::CustomButton("Veteran2025Gold")) {ChangeSkin(336);}
    if (ImGui::CustomButton("Veteran2025EliteGold")) {ChangeSkin(337);}
    if (ImGui::CustomButton("Veteran2025Platinum")) {ChangeSkin(338);}
    if (ImGui::CustomButton("Veteran2025Diamond")) {ChangeSkin(339);}
    if (ImGui::CustomButton("SnowJourney")) {ChangeSkin(340);}
    if (ImGui::CustomButton("KitsuneDreamsBronze")) {ChangeSkin(341);}
    if (ImGui::CustomButton("KitsuneDreamsSilver")) {ChangeSkin(342);}
    if (ImGui::CustomButton("KitsuneDreamsGold")) {ChangeSkin(343);}
    if (ImGui::CustomButton("KitsuneDreamsEliteGold")) {ChangeSkin(344);}
    if (ImGui::CustomButton("KitsuneDreamsPlatinum")) {ChangeSkin(345);}
    if (ImGui::CustomButton("KitsuneDreamsDiamond")) {ChangeSkin(346);}
    if (ImGui::CustomButton("PreyBronze")) {ChangeSkin(352);}
    if (ImGui::CustomButton("PreySilver")) {ChangeSkin(353);}
    if (ImGui::CustomButton("PreyGold")) {ChangeSkin(354);}
    if (ImGui::CustomButton("PreyEliteGold")) {ChangeSkin(355);}
    if (ImGui::CustomButton("PreyPlatinum")) {ChangeSkin(356);}
    if (ImGui::CustomButton("PreyDiamond")) {ChangeSkin(357);}
    if (ImGui::CustomButton("GambitBronze")) {ChangeSkin(366);}
    if (ImGui::CustomButton("GambitSilver")) {ChangeSkin(367);}
    if (ImGui::CustomButton("GambitGold")) {ChangeSkin(368);}
    if (ImGui::CustomButton("GambitEliteGold")) {ChangeSkin(369);}
    if (ImGui::CustomButton("GambitPlatinum")) {ChangeSkin(370);}
    if (ImGui::CustomButton("GambitDiamond")) {ChangeSkin(371);}
    if (ImGui::CustomButton("SyndicateBronze")) {ChangeSkin(372);}
    if (ImGui::CustomButton("SyndicateSilver")) {ChangeSkin(373);}
    if (ImGui::CustomButton("SyndicateGold")) {ChangeSkin(374);}
    if (ImGui::CustomButton("SyndicateEliteGold")) {ChangeSkin(375);}
    if (ImGui::CustomButton("SyndicatePlatinum")) {ChangeSkin(376);}
    if (ImGui::CustomButton("SyndicateDiamond")) {ChangeSkin(377);}
    if (ImGui::CustomButton("DiaDeMuertosBronze")) {ChangeSkin(8000);}
    if (ImGui::CustomButton("DiaDeMuertosSilver")) {ChangeSkin(8001);}
    if (ImGui::CustomButton("DiaDeMuertosGold")) {ChangeSkin(8002);}
    if (ImGui::CustomButton("DiaDeMuertosEliteGold")) {ChangeSkin(8003);}
    if (ImGui::CustomButton("DiaDeMuertosPlatinum")) {ChangeSkin(8004);}
    if (ImGui::CustomButton("DiaDeMuertosDiamond")) {ChangeSkin(8005);}
    if (ImGui::CustomButton("Veteran 2026 Bronze")) { ChangeSkin(8012); }
    if (ImGui::CustomButton("Veteran 2026 Silver")) { ChangeSkin(8013); }
    if (ImGui::CustomButton("Veteran 2026 Gold")) { ChangeSkin(8014); }
    if (ImGui::CustomButton("Veteran 2026 Elite Gold")) { ChangeSkin(8015); }
    if (ImGui::CustomButton("Veteran 2026 Platinum")) { ChangeSkin(8016); }
    if (ImGui::CustomButton("Veteran 2026 Diamond")) { ChangeSkin(8017); }
    if (ImGui::CustomButton("Winter Tale Bronze")) { ChangeSkin(8006); }
    if (ImGui::CustomButton("Winter Tale Silver")) { ChangeSkin(8007); }
    if (ImGui::CustomButton("Winter Tale Gold")) { ChangeSkin(8008); }
    if (ImGui::CustomButton("Winter Tale Elite Gold")) { ChangeSkin(8009); }
    if (ImGui::CustomButton("Winter Tale Platinum")) { ChangeSkin(8010); }
    if (ImGui::CustomButton("Winter Tale Diamond")) { ChangeSkin(8011); }
    if (ImGui::CustomButton("YearTheHorse Quartz")) { ChangeSkin(8018); }
    if (ImGui::CustomButton("YearTheHorse Topaz")) { ChangeSkin(8019); }
    if (ImGui::CustomButton("YearTheHorse Ruby")) { ChangeSkin(8020); }
    if (ImGui::CustomButton("Breakout 1")) { ChangeSkin(8021); }
    if (ImGui::CustomButton("Breakout 2")) { ChangeSkin(8022); }
    if (ImGui::CustomButton("Breakout 3")) { ChangeSkin(8023); }
    if (ImGui::CustomButton("Breakout 4")) { ChangeSkin(8024); }
    if (ImGui::CustomButton("Breakout 5")) { ChangeSkin(8025); }
    if (ImGui::CustomButton("Breakout 6")) { ChangeSkin(8026); }
    }       
    if (avatarset){
    MenuText("Avatar Frames");
    if (ImGui::CustomButton("Tier1Bronze")) {ChangeSkin(7001);}
    if (ImGui::CustomButton("Tier2Bronze")) {ChangeSkin(7002);}
    if (ImGui::CustomButton("Tier3Bronze")) {ChangeSkin(7003);}
    if (ImGui::CustomButton("Tier4Bronze")) {ChangeSkin(7004);}
    if (ImGui::CustomButton("Tier5Bronze")) {ChangeSkin(7005);}
    if (ImGui::CustomButton("Tier1Silver")) {ChangeSkin(7006);}
    if (ImGui::CustomButton("Tier2Silver")) {ChangeSkin(7007);}
    if (ImGui::CustomButton("Tier3Silver")) {ChangeSkin(7008);}
    if (ImGui::CustomButton("Tier4Silver")) {ChangeSkin(7009);}
    if (ImGui::CustomButton("Tier5Silver")) {ChangeSkin(7010);}
    if (ImGui::CustomButton("Tier1Gold")) {ChangeSkin(7011);}
    if (ImGui::CustomButton("Tier2Gold")) {ChangeSkin(7012);}
    if (ImGui::CustomButton("Tier3Gold")) {ChangeSkin(7013);}
    if (ImGui::CustomButton("Tier4Gold")) {ChangeSkin(7014);}
    if (ImGui::CustomButton("Tier5Gold")) {ChangeSkin(7015);}
    if (ImGui::CustomButton("Tier1GoldElite")) {ChangeSkin(7016);}
    if (ImGui::CustomButton("Tier2GoldElite")) {ChangeSkin(7017);}
    if (ImGui::CustomButton("Tier3GoldElite")) {ChangeSkin(7018);}
    if (ImGui::CustomButton("Tier4GoldElite")) {ChangeSkin(7019);}
    if (ImGui::CustomButton("Tier5GoldElite")) {ChangeSkin(7020);}
    if (ImGui::CustomButton("Tier1Platinum")) {ChangeSkin(7021);}
    if (ImGui::CustomButton("Tier2Platinum")) {ChangeSkin(7022);}
    if (ImGui::CustomButton("Tier3Platinum")) {ChangeSkin(7023);}
    if (ImGui::CustomButton("Tier4Platinum")) {ChangeSkin(7024);}
    if (ImGui::CustomButton("Tier5Platinum")) {ChangeSkin(7025);}
    if (ImGui::CustomButton("Tier1Diamond")) {ChangeSkin(7026);}
    if (ImGui::CustomButton("Tier2Diamond")) {ChangeSkin(7027);}
    if (ImGui::CustomButton("Tier3Diamond")) {ChangeSkin(7028);}
    if (ImGui::CustomButton("Tier4Diamond")) {ChangeSkin(7029);}
    if (ImGui::CustomButton("Tier5Diamond")) {ChangeSkin(7030);}
    if (ImGui::CustomButton("Portal")) {ChangeSkin(7031);}
    if (ImGui::CustomButton("CommanderTier1")) {ChangeSkin(7032);}
    if (ImGui::CustomButton("CommanderTier2")) {ChangeSkin(7033);}
    if (ImGui::CustomButton("CommanderTier3")) {ChangeSkin(7034);}
    if (ImGui::CustomButton("CommanderTier3CT")) {ChangeSkin(7035);}
    if (ImGui::CustomButton("KitsuneDreamsRare")) {ChangeSkin(7036);}
    if (ImGui::CustomButton("KitsuneDreamsEpic")) {ChangeSkin(7037);}
    if (ImGui::CustomButton("KitsuneDreamsLegendary")) {ChangeSkin(7038);}
    if (ImGui::CustomButton("KitsuneDreamsArcane")) {ChangeSkin(7039);}
    if (ImGui::CustomButton("DiaDeMuertos")) {ChangeSkin(7040);}
    }
    }
        EndChild();
        }
        else
        if (textPageId == 12) {
    const float childW = (menuConfig::background::size.x - 390.0f) / 1.64f * 1.1f;
    const float childH = menuConfig::background::size.y + 69.0f;
    BeginChild("##MARKUPS_CILD", ImVec2(childW, childH), true);
    {
    SectionHeader("Markups");
    CustomSwitch("Set Statrack", &set_statrack);
    CustomSwitch("Set Pattern", &set_pattern);
    }
    EndChild();
        SameLine();
        ImGui::BeginChild("##MARKUPS_SETTINGS", ImVec2(childW, childH), true);
    {
    SectionHeader("Settings");
    if (set_statrack) {
    ImGui::CustomSliderInt("Statrack value", &srarrackval, 0, 999999);
    }
        if (set_pattern) {
    ImGui::CustomSliderInt("Pattern value", &patternval, 1, 999);
    }
        }
        EndChild();
        }
        else
        if (textPageId == 13) {
    const float childW = (menuConfig::background::size.x - 390.0f) / 1.64f * 1.1f;
    const float childH = menuConfig::background::size.y + 69.0f;
    ImGui::BeginChild("##CONFIGURATION", ImVec2(childW, childH), true);
    {
    SectionHeader("Configurations");
    ScanConfigs();
        const char* createItems[] = { "Legit", "Rage", "Custom" };
        CustomCombo("Create Type", &createType, createItems, 3);
        if (!ConfigList.empty())
        {
        for (int i = 0; i < (int)ConfigList.size(); i++)
    cfgItems[i] = ConfigList[i].c_str();
    CustomCombo("Configs", &selectedConfig, cfgItems, ConfigList.size());
    }
    } 
    EndChild();
    SameLine();
    ImGui::BeginChild("##CFGFUNCTION", ImVec2(childW, childH), true);
    {
    SectionHeader("Functions");
    if (CustomButton("Create configuration", ImVec2((GetContentRegionMax().x - style->WindowPadding.x) - 0, 60 * 1.1f))) {
    CreateConfig();
    }
    if (CustomButton("Save configuration", ImVec2((GetContentRegionMax().x - style->WindowPadding.x) - 0, 60 * 1.1f))) {
    SaveConfigSelected();
    }
    if (CustomButton("Load configuration", ImVec2((GetContentRegionMax().x - style->WindowPadding.x) - 0, 60 * 1.1f))) {
    LoadConfigSelected();
    }
    if (CustomButton("Delete configuration", ImVec2((GetContentRegionMax().x - style->WindowPadding.x) - 0, 60 * 1.1f))) {
    DeleteConfigSelected();
    }
    }
    EndChild();
    }
    else
    if (textPageId == 14) {
    const float childW = (menuConfig::background::size.x - 390.0f) / 1.64f * 1.1f;
    const float childH = menuConfig::background::size.y + 69.0f;
    BeginChild("##MENUS_CILD", ImVec2(childW, childH), true);
    {
    SectionHeader("Menu");
    CustomSwitch("Background", &background);
    CustomSwitch("Snowflakes", &snowflake);
    }
    EndChild();
        SameLine();
        ImGui::BeginChild("##MENUS_SETTINGS", ImVec2(childW, childH), true);
    {
    SectionHeader("Settings");
    CustomColorEdit3("Theme color", accentColor);
    if (background) {
        CustomSliderFloat("Opacity", &backgroundOpacity, 25.0f, 75.0f);
        }
        }
        EndChild();
        }
        else
        if (textPageId == 15) {
    const float childW = (menuConfig::background::size.x - 390.0f) / 1.64f * 1.1f;
    const float childH = menuConfig::background::size.y + 69.0f;
    BeginChild("##RECORDER_CILD", ImVec2(childW, childH), true);
    {
    SectionHeader("Recorder");
    CustomSwitch("Recorder", &record_enabled);
    if (record_enabled) {
    if (ImGui::CustomButton("Start")) {
    StartRecording();
    }
    if (ImGui::CustomButton("Stop")) {
    StopRecording();
    }
    if (is_recording) {
    ImGui::TextColored(ImVec4(0, 1, 0, 1), "Recording");
    } else {
    ImGui::TextColored(ImVec4(1, 1, 1, 0.5), "Stopped");
    }
    }
    }
    EndChild();
        SameLine();
        ImGui::BeginChild("##RECORDER_SETTINGS", ImVec2(childW, childH), true);
    {
    SectionHeader("Settings");
    if (record_enabled) {
    const char* qitem[] = { "Low", "Medium", "High" };
    CustomCombo("Quality", &record_quality, qitem, 3);
    CustomSliderInt("Time", &record_time, 30, 1800, "%d");
    }
        }
        EndChild();
        }
    PopStyleVar();
    End();
}
