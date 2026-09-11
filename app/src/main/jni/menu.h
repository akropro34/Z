#pragma once
#include "include/includes.h"
#include "game.h"
#include "game/Ruleset.h"
#include "game/inc/NumberUtils.h"
#include "game/inc/AutoAim.h"
#include "game/inc/AutoQueue.h"
#include "imgui/inc/8bp.h"
#include "include/random_defs.h"
#include "mod/keylogin.h"
#include "oxorany/oxorany.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <sys/system_properties.h>
#include <ctime>
#include <cstring>
#include <Vector/Vectors.h>
#include <imgui/imgui.h>
#include "icons/icons.h"


using namespace ImGui;
using namespace std;

struct MenuState {
    bool isOpen = false;
    int currentTab = 0;
    float sidebarWidth = 750.0f;
    float animProgress = 0.0f;
    float menuAlpha = 0.0f;
    float menuScale = 0.9f;
    ImVec4 accentColor = ImVec4(0.35f, 0.65f, 0.95f, 1.0f);
};
static MenuState g_menu;

static const int64_t EXPIRY_TS = O(1799999200LL);

static bool DEBUG_BYPASS_LOGIN = false;

// ── TIGER: loader toggle — read from .menu_enabled file ─────────────────────
static std::atomic<bool> g_menu_enabled{true};
static std::atomic<bool> g_tiger_login_done{false};

static void ReadMenuToggle() {
    static bool read_once = false;
    if (read_once) return;
    read_once = true;
    std::ifstream ifs("/data/data/com.miniclip.eightballpool/files/.menu_enabled");
    if (!ifs.is_open())
        ifs.open("/data/data/com.fs4ip.delta/files/.menu_enabled");
    if (ifs.is_open()) {
        char c = '1';
        ifs.get(c);
        g_menu_enabled.store(c != '0');
    }
}

static void TigerAutoLogin() {
    if (g_tiger_login_done.exchange(true)) return;
    std::thread([]() {
        const std::string paths[] = {
            std::string("/data/data/com.miniclip.eightballpool/files/.tiger_k"),
            std::string("/data/data/com.fs4ip.delta/files/.tiger_k")
        };
        std::string key;
        for (auto& p : paths) {
            std::ifstream ifs(p);
            if (!ifs.is_open()) continue;
            std::getline(ifs, key);
            auto s = key.find_first_not_of(" \t\r\n");
            auto e = key.find_last_not_of(" \t\r\n");
            key = (s == std::string::npos) ? "" : key.substr(s, e - s + 1);
            if (!key.empty()) break;
        }
        if (key.empty()) { LOGI("tiger_k not found"); return; }
        JNIEnv* env = GetJNIEnv();
        std::string aid = env ? getAndroidID(env) : "";
        Login(aid, key);
        LOGI("TigerAutoLogin: logged_in=%d", (int)logged_in);
    }).detach();
}

static float EaseOutBack(float x) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return 1.0f + c3 * powf(x - 1.0f, 3.0f) + c1 * powf(x - 1.0f, 2.0f);
}

static float EaseOutQuart(float x) {
    return 1.0f - powf(1.0f - x, 4.0f);
}

static void DrawGradientRect(ImDrawList* dl, ImVec2 p1, ImVec2 p2, ImU32 col1, ImU32 col2, bool horizontal = true) {
    if (horizontal) {
        dl->AddRectFilledMultiColor(p1, p2, col1, col2, col2, col1);
    } else {
        dl->AddRectFilledMultiColor(p1, p2, col1, col1, col2, col2);
    }
}

// ── Theme: Deep Crimson Red (🐯 TIGER) ─────────────────────────────────────
namespace Theme {
    static const ImU32 BgDeep       = IM_COL32(18, 3, 6, 255);
    static const ImU32 BgPanel      = IM_COL32(46, 6, 12, 235);
    static const ImU32 Crimson      = IM_COL32(120, 8, 24, 255);
    static const ImU32 CrimsonDark  = IM_COL32(70, 4, 14, 255);
    static const ImU32 CrimsonGlow  = IM_COL32(220, 20, 45, 90);
    static const ImU32 Accent       = IM_COL32(200, 18, 40, 255);
    static const ImU32 AccentBright = IM_COL32(255, 45, 70, 255);
    static const ImU32 TextPrimary  = IM_COL32(255, 235, 236, 255);
    static const ImU32 TextMuted    = IM_COL32(210, 160, 165, 210);
}

// Liquid-glass fill/border used for the selected-tab capsule and panel chrome,
// ported from the reference draw.h look but recolored to dark crimson.
static void DrawLiquidGlass(ImDrawList* dl, ImVec2 p1, ImVec2 p2, float rounding, float intensity = 1.0f, bool selected = false) {
    int aFill = (int)(10 * intensity);
    int aBorder = (int)(32 * intensity);
    int aTop = (int)(50 * intensity);
    float t = ImGui::GetTime();
    float pulse = 0.55f + 0.45f * sinf(t * 2.4f);

    if (selected) {
        dl->AddRectFilled(p1, p2, IM_COL32(120, 8, 24, (int)(85 * intensity)), rounding);
        dl->AddRectFilled(p1, p2, IM_COL32(255, 30, 55, (int)(28 * intensity)), rounding);
        dl->AddRect(ImVec2(p1.x - 2, p1.y - 2), ImVec2(p2.x + 2, p2.y + 2),
                    IM_COL32(255, 40, 65, (int)(35 + 25 * pulse)), rounding + 2.0f, 0, 2.0f);
        dl->AddRect(ImVec2(p1.x - 5, p1.y - 5), ImVec2(p2.x + 5, p2.y + 5),
                    IM_COL32(180, 10, 30, (int)(18 + 12 * pulse)), rounding + 4.0f, 0, 3.5f);
    } else {
        dl->AddRectFilled(p1, p2, IM_COL32(255, 190, 195, aFill), rounding);
    }

    float inset = 1.0f;
    dl->AddRectFilledMultiColor(
        ImVec2(p1.x + inset, p1.y + inset),
        ImVec2(p2.x - inset, p1.y + (p2.y - p1.y) * 0.42f),
        IM_COL32(255, 210, 215, aTop),
        IM_COL32(255, 150, 160, aTop),
        IM_COL32(255, 255, 255, 0),
        IM_COL32(255, 255, 255, 0)
    );

    if (selected) {
        dl->AddRect(p1, p2, IM_COL32(255, 60, 85, (int)(200 * intensity)), rounding, 0, 1.8f);
        dl->AddRect(ImVec2(p1.x + 1.2f, p1.y + 1.2f), ImVec2(p2.x - 1.2f, p2.y - 1.2f),
                    IM_COL32(150, 10, 30, (int)(140 * intensity)), rounding - 1.0f, 0, 1.2f);
    } else {
        dl->AddRect(p1, p2, IM_COL32(255, 170, 175, aBorder), rounding, 0, 1.1f);
    }
}

// Pulsing crimson rim + specular hotspots drawn around the whole menu panel.
static void DrawGlassFrameBorder(ImDrawList* dl, ImVec2 p1, ImVec2 p2, float rounding, float alpha) {
    float t = ImGui::GetTime();
    float pulse = 0.6f + 0.4f * sinf(t * 1.8f);

    dl->AddRect(ImVec2(p1.x - 3, p1.y - 3), ImVec2(p2.x + 3, p2.y + 3),
                IM_COL32(220, 15, 40, (int)(20 * alpha * pulse)), rounding + 3.0f, 0, 4.0f);
    dl->AddRect(p1, p2, IM_COL32(255, 90, 100, (int)(95 * alpha)), rounding, 0, 1.35f);
    dl->AddRectFilledMultiColor(
        ImVec2(p1.x + 8, p1.y + 1), ImVec2(p1.x + 90, p1.y + 3),
        IM_COL32(255, 200, 205, (int)(140 * alpha)), IM_COL32(200, 10, 30, 0),
        IM_COL32(200, 10, 30, 0), IM_COL32(255, 200, 205, (int)(140 * alpha))
    );
    dl->AddRectFilledMultiColor(
        ImVec2(p2.x - 100, p2.y - 3), ImVec2(p2.x - 10, p2.y - 1),
        IM_COL32(200, 10, 30, 0), IM_COL32(255, 220, 220, (int)(100 * alpha)),
        IM_COL32(255, 220, 220, (int)(100 * alpha)), IM_COL32(200, 10, 30, 0)
    );
}

static bool SidebarButton(const char* label, GLuint iconTex, bool selected, float width) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    float iconSize   = 60.0f;
    float vPad       = 10.0f;
    float btnH       = vPad + iconSize + 4.0f + g.FontSize + vPad;

    ImVec2 pos  = window->DC.CursorPos;
    ImVec2 size = ImVec2(width, btnH);

    const ImRect bb(pos, pos + size);
    ItemSize(size, style.FramePadding.y);
    if (!ItemAdd(bb, id)) return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);

    ImDrawList* dl = window->DrawList;

    // Icon center position
    float iconBgPad  = 6.0f;
    float iconBgSize = iconSize + iconBgPad * 2.0f;
    ImVec2 iconCenter = ImVec2(
        bb.Min.x + width * 0.5f,
        bb.Min.y + vPad + iconSize * 0.5f
    );

    // Selected: liquid-glass crimson capsule behind icon, with a pulsing rim
    if (selected) {
        DrawLiquidGlass(
            dl,
            ImVec2(iconCenter.x - iconBgSize * 0.5f, iconCenter.y - iconBgSize * 0.5f),
            ImVec2(iconCenter.x + iconBgSize * 0.5f, iconCenter.y + iconBgSize * 0.5f),
            12.0f, 1.0f, true
        );
    }

    // Draw icon texture centered, with color filter when not selected
    if (iconTex) {
        ImVec2 iconMin = ImVec2(iconCenter.x - iconSize * 0.5f, iconCenter.y - iconSize * 0.5f);
        ImVec2 iconMax = ImVec2(iconCenter.x + iconSize * 0.5f, iconCenter.y + iconSize * 0.5f);
        ImU32 tint = selected ? IM_COL32(255, 255, 255, 255) : IM_COL32(255, 255, 255, 255);
        dl->AddImage((void*)(intptr_t)iconTex, iconMin, iconMax, ImVec2(0,0), ImVec2(1,1));
    }

    // Draw label centered below icon
    ImVec2 labelSize = CalcTextSize(label);
    ImVec2 textPos   = ImVec2(
        bb.Min.x + (width - labelSize.x) * 0.5f,
        bb.Min.y + vPad + iconSize + 4.0f
    );
    ImU32 textCol = selected ? IM_COL32(255, 255, 255, 255) : IM_COL32(140, 140, 150, 255);
    dl->AddText(textPos, textCol, label);

    return pressed;
}

static bool ToggleSwitch(const char* label, bool* v) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    float scale = 1.5f; // 1.5f is with 50% bigger than writen values
    float height = 32.0f * scale;
    float width = 56.0f * scale;
    float radius = height * 0.5f;

    ImVec2 textSize = CalcTextSize(label);
    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = ImVec2(GetContentRegionAvail().x, ImMax(height, textSize.y) + style.FramePadding.y * 2 + 10.0f);

    const ImRect bb(pos, pos + size);
    ItemSize(size, style.FramePadding.y);
    if (!ItemAdd(bb, id)) return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);
    if (pressed) *v = !*v;

    static std::map<ImGuiID, float> switchAnim;
    float& animT = switchAnim[id];
    float targetT = *v ? 1.0f : 0.0f;
    animT += (targetT - animT) * g.IO.DeltaTime * 14.0f;

    ImDrawList* dl = window->DrawList;
    
    if (hovered) {
        dl->AddRectFilled(bb.Min, bb.Max, IM_COL32(45, 45, 55, 100), 10.0f);
    }
    
    ImVec2 togglePos = ImVec2(bb.Max.x - width - 15.0f, bb.Min.y + (size.y - height) * 0.5f);
    ImVec2 toggleEnd = ImVec2(togglePos.x + width, togglePos.y + height);
    
    ImVec4 offColor = ImVec4(0.27f, 0.27f, 0.31f, 1.0f);
    ImVec4 onColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    ImVec4 bgColorV = ImLerp(offColor, onColor, animT);
    dl->AddRectFilled(togglePos, toggleEnd, ImColor(bgColorV), 0.0f);
    
    float knobX = togglePos.x + radius + (width - height) * animT;
    float knobY = togglePos.y + radius;
    float knobR = radius - 4.0f;
    
    dl->AddCircleFilled(ImVec2(knobX, knobY), knobR + 2.0f, IM_COL32(0, 0, 0, 40));
    dl->AddCircleFilled(ImVec2(knobX, knobY), knobR, IM_COL32(255, 255, 255, 255));

    dl->AddText(ImVec2(bb.Min.x + 15.0f, bb.Min.y + (size.y - textSize.y) * 0.5f), IM_COL32(230, 230, 240, 255), label);

    return pressed;
}

// File-scope so DrawToggleButton cancel can also reset countdown
static bool g_aqCounting = false;
static std::chrono::steady_clock::time_point g_aqLastCall;
static std::chrono::steady_clock::time_point g_aqCountdownStart;


static bool IsExpired() {
    return GetServerNow() >= EXPIRY_TS;
}

INLINE void DrawExpired(ImGuiIO& io) {
    float winW = g_menu.sidebarWidth;

    SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    SetNextWindowSize(ImVec2(winW, 0), ImGuiCond_Always);
    PushStyleColor(ImGuiCol_WindowBg, IM_COL32(180, 0, 0, 255));
    PushStyleVar(ImGuiStyleVar_WindowRounding, 20.0f);
    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(30.0f, 30.0f));
    PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    if (Begin(O("##ExpiredWin"), nullptr,
              ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
              ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
              ImGuiWindowFlags_AlwaysAutoResize)) {

        SetWindowFontScale(1.6f);
        ImVec2 titleSz = CalcTextSize(O("MOD EXPIRED"));
        SetCursorPosX((winW - 60.0f - titleSz.x) * 0.5f);
        TextColored(ImVec4(1.0f, 0.1f, 0.1f, 1.0f), "%s", O("MOD EXPIRED"));
        SetWindowFontScale(1.0f);

        Dummy(ImVec2(0, 16));

        PushTextWrapPos(GetCursorPosX() + winW - 60.0f);
        TextColored(ImVec4(0.85f, 0.85f, 0.90f, 1.0f), "%s",
            O("Beta Version Expired. Update on our Telegram @AKOJO"));
        PopTextWrapPos();

        Dummy(ImVec2(0, 10));
    }
    End();
    PopStyleVar(3);
    PopStyleColor();
}

INLINE void DrawAutoQueue() {
    if ((g_ExpiryTime.load() > 0) || DEBUG_BYPASS_LOGIN) {
        auto now = std::chrono::steady_clock::now();

        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - g_aqLastCall).count() > 500)
            g_aqCounting = false;
        g_aqLastCall = now;

        if (!g_aqCounting) {
            g_aqCounting = true;
            g_aqCountdownStart = now;
        }

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_aqCountdownStart).count();
        int remaining_ms = 8000 - (int)elapsed;

        if (remaining_ms <= 0) {
            if (sharedMenuManager.getMenuStateId() == 13) PopMenuState(13);
            StartLastMatch();
            g_aqCounting = false;
            return;
        }

        std::string count_str = std::to_string((remaining_ms / 1000) + 1);

        // Minimal auto-sized window, transparent bg — we draw our own rounded rect
        SetNextWindowPos(ImVec2(Width * 0.5f, Height * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 1.f));
        PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(32.0f, 20.0f));
        PushStyleVar(ImGuiStyleVar_WindowRounding, 24.0f);

        if (Begin(O("##AutoQueueCD"), nullptr,
                  ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                  ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
                  ImGuiWindowFlags_AlwaysAutoResize)) {
            ImDrawList* dl  = GetWindowDrawList();
            ImVec2      wp  = GetWindowPos();
            ImVec2      ws  = GetWindowSize();
            dl->AddRectFilled(wp, ImVec2(wp.x + ws.x, wp.y + ws.y), IM_COL32(20, 20, 28, 0), 24.0f);

            SetWindowFontScale(3.5f);
            TextColored(ImVec4(1.f, 0.f, 0.f, 1.0f), "%s", count_str.c_str());
            SetWindowFontScale(1.0f);
        }
        End();
        PopStyleVar(2);
        PopStyleColor();
    }
}

#include "mod/ButtonClicker.h"

static void DrawToggleButton(bool cancelMode); // forward declaration — defined after DrawFloatingButton
static void DrawShotPreview(ImDrawList* draw);  // forward declaration — defined further below
static void DrawToasts(ImDrawList* draw);       // forward declaration — defined further below

// Bilingual UI text helper: L(english, arabic) returns one of the two based
// on the "bArabicUI" setting (English by default, matching the rest of the
// menu's text). Was called throughout the file but never defined anywhere.
static const char* L(const char* en, const char* ar) {
    return persistent_bool[O("bArabicUI")] ? ar : en;
}

INLINE void DrawESP(ImDrawList* draw) {
    if ((g_ExpiryTime.load() > 0) || DEBUG_BYPASS_LOGIN) {
        if (!sharedGameManager) return;

        UpdateScreenTable();

        sharedDirector = F(ptr, libmain + O(0x4f06288));
        if (!sharedDirector) return;

        sharedUserInfo = F(ptr, libmain + O(0x4e9feb8));
        if (!sharedUserInfo) return;

        F(bool, sharedUserInfo + 0x340) = true;

        sharedMainManager = F(ptr, libmain + O(0x4dde3e0));
        if (!sharedMainManager) return;

        sharedMenuManager = F(ptr, libmain + O(0x4dfe838));
        if (!sharedMenuManager) return;

        MainStateManager mainStateManager = sharedMainManager.mStateManager;
        if (!mainStateManager) return;
        if (!mainStateManager.isInGame()) {
        if (persistent_bool[O("AutoQueue")]) {
            if (!sharedMenuManager.isInQueue()) DrawAutoQueue();
            DrawToggleButton(true);  // acts as cancel button for autoqueue
        } return;
        }

        auto visualCue = sharedGameManager.mVisualCue();

        Ball::Classification myclass = sharedGameManager.getPlayerClassification();

        Table table = sharedGameManager.mTable;
        if (!table) return;

        auto tableProperties = table.mTableProperties();
        if (!tableProperties) return;

        auto& pockets = tableProperties.mPockets();

        GameStateManager gameStateManager = sharedGameManager.mStateManager;
        if (!gameStateManager) return;

        // Drive AutoPlay's internal mode from the two menu toggles. currentMode
        // was previously never assigned anywhere (stuck at MODE_OFF forever),
        // which silently killed the whole STYLE_HUMAN scanning branch and made
        // the built-in MODE_AUTO_AIM aim-only path unreachable.
        if (persistent_bool[O("AutoPlay")]) {
            // Full AutoPlay: scans, aims, and fires on its own using whichever
            // Play Style the user picked in the menu.
            DrawToggleButton(false);
            AutoPlay::currentMode = AutoPlay::MODE_AUTO_PLAY;
            AutoPlay::playStyle   = (AutoPlay::PlayStyle)persistent_int[O("iPlayStyle")];
        } else if (persistent_bool[O("bAutoAim")]) {
            // Auto Aim only: snaps the cue onto the best shot and holds it there
            // (MODE_AUTO_AIM + STYLE_WILD stops right after aiming, see Shoot()).
            // No shot is fired here - the player still pulls and releases the
            // cue manually.
            AutoPlay::currentMode  = AutoPlay::MODE_AUTO_AIM;
            AutoPlay::playStyle    = AutoPlay::STYLE_WILD;
            AutoPlay::bAutoPlaying = true;
        } else {
            AutoPlay::currentMode  = AutoPlay::MODE_OFF;
            AutoPlay::bAutoPlaying = false;
        }
        // Always run Update() - its own abort handler is what safely releases
        // touches/resets state the frame after bAutoPlaying drops to false.
        AutoPlay::Update();

        auto stateId = gameStateManager.getCurrentStateId();
        if (stateId == 4) gPrediction->determineShotResult(false);
        if (stateId == 6 || stateId == 7 || stateId == 8) return;

        if (persistent_bool[O("bESP_DrawPocketsShotState")]) {
            for (int i = 0; i < 6; i++) {
                if (Prediction::pocketStatus[i]) {
                    auto screenPos = WorldToScreen(pockets[i]);
                    draw->AddCircle(ImVec2(screenPos.x, screenPos.y), 30, GREEN, 0, 5.f);
                }
            }
        }

        if (persistent_bool[O("bESP_DrawPredictionLine")]) {
            for (int i = 0; i < gPrediction->guiData.ballsCount; i++) {
                auto& ball = gPrediction->guiData.balls[i];

                if (ball.initialPosition != ball.predictedPosition) {
                    ImVec2 lastPos{};
                    float lineThick = (float)persistent_int[O("iLineThickness")];
                    if (lineThick < 1.f) lineThick = 1.f;
                    for (int j = 1; j < ball.positions.size(); j++) {
                        auto point = WorldToScreen(ball.positions[j]);
                        if (lastPos.x || lastPos.y) draw->AddLine(lastPos, point, colors[i], lineThick);
                        lastPos = point;
                    }
                }
            }
        }

        if (persistent_bool[O("bESP_DrawPredictionLine")]) {
            for (int i = 0; i < gPrediction->guiData.ballsCount; i++) {
                auto& ball = gPrediction->guiData.balls[i];

                if (ball.initialPosition != ball.predictedPosition) {
                    float circleR = (float)persistent_int[O("iLineThickness")] + 1.f;
                    if (circleR < 2.f) circleR = 2.f;
                    draw->AddCircleFilled(WorldToScreen(ball.initialPosition), circleR, colors[i]);
                    draw->AddCircleFilled(WorldToScreen(ball.predictedPosition), 16, colors[i]);
                }
            }
        }

        DrawShotPreview(draw);
        DrawToasts(draw);
    }
}

// ── Vertical icon rail (menu3 shape), using our own textures/tabs/theme ──
static const float RAIL_W = 96.0f;

static bool RailButton(GLuint iconTex, bool selected, ImVec2 pos, float size) {
    ImGuiWindow* window = GetCurrentWindow();
    ImGuiContext& g = *GImGui;
    char idbuf[16]; snprintf(idbuf, sizeof(idbuf), "##rail%p", (void*)(intptr_t)iconTex);
    const ImGuiID id = window->GetID(idbuf);

    ImRect bb(pos, pos + ImVec2(size, size));
    ItemSize(ImVec2(size, size), g.Style.FramePadding.y);
    if (!ItemAdd(bb, id)) return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);
    ImDrawList* dl = window->DrawList;
    ImVec2 c = ImVec2(bb.Min.x + size * 0.5f, bb.Min.y + size * 0.5f);

    if (selected) {
        DrawLiquidGlass(dl, ImVec2(c.x - size * 0.5f, c.y - size * 0.5f),
                         ImVec2(c.x + size * 0.5f, c.y + size * 0.5f), size * 0.5f, 1.0f, true);
    } else if (hovered) {
        dl->AddCircleFilled(c, size * 0.5f, IM_COL32(255, 255, 255, 14), 48);
    }

    if (iconTex) {
        float iconSize = size * 0.55f;
        ImVec2 iconMin = ImVec2(c.x - iconSize * 0.5f, c.y - iconSize * 0.5f);
        ImVec2 iconMax = ImVec2(c.x + iconSize * 0.5f, c.y + iconSize * 0.5f);
        dl->AddImage((void*)(intptr_t)iconTex, iconMin, iconMax, ImVec2(0,0), ImVec2(1,1));
    }
    return pressed;
}

static void DrawSidebar(float winW, float winH) {
    static GLuint draw_icon_tex = LoadTextureFromMemory(draw_icon_png, draw_icon_png_len);
    static GLuint play_icon_tex = LoadTextureFromMemory(play_icon_png, play_icon_png_len);
    static GLuint q_icon_tex    = LoadTextureFromMemory(q_icon_png,    q_icon_png_len);
    static GLuint user_icon_tex = LoadTextureFromMemory(user_icon_png, user_icon_png_len);

    ImDrawList* dl = GetWindowDrawList();
    ImVec2      wp = GetWindowPos();

    float railX = winW - RAIL_W;
    ImVec2 rp   = ImVec2(wp.x + railX, wp.y);

    // Rail background (rounded dark crimson column)
    dl->AddRectFilled(rp, ImVec2(rp.x + RAIL_W, wp.y + winH), Theme::BgDeep, 22.0f);

    GLuint icons[4] = { draw_icon_tex, play_icon_tex, q_icon_tex, user_icon_tex };
    float btnSize = 62.0f;
    float gap     = 20.0f;
    float startY  = 30.0f;

    for (int i = 0; i < 4; ++i) {
        ImVec2 pos = ImVec2(railX + (RAIL_W - btnSize) * 0.5f, startY + i * (btnSize + gap));
        SetCursorPos(pos);
        if (RailButton(icons[i], g_menu.currentTab == i, ImVec2(wp.x + pos.x, wp.y + pos.y), btnSize))
            g_menu.currentTab = i;
    }

    // Close (X) button pinned to the bottom of the rail
    float closeSize = 46.0f;
    ImVec2 closePos = ImVec2(railX + (RAIL_W - closeSize) * 0.5f, winH - closeSize - 24.0f);
    SetCursorPos(closePos);
    {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* win = GetCurrentWindow();
        ImGuiID closeId  = win->GetID(O("##CloseMenu"));
        ImVec2 cp        = win->DC.CursorPos;
        ImRect bb(cp, cp + ImVec2(closeSize, closeSize));
        ItemSize(ImVec2(closeSize, closeSize), g.Style.FramePadding.y);
        if (ItemAdd(bb, closeId)) {
            bool hovered, held;
            if (ButtonBehavior(bb, closeId, &hovered, &held)) g_menu.isOpen = false;
            ImVec2 c = ImVec2(bb.Min.x + closeSize * 0.5f, bb.Min.y + closeSize * 0.5f);
            if (hovered) dl->AddCircleFilled(c, closeSize * 0.5f, IM_COL32(255, 255, 255, 16), 48);
            float xH = closeSize * 0.22f;
            ImU32 xc = hovered ? IM_COL32(255, 255, 255, 255) : IM_COL32(255, 235, 236, 200);
            dl->AddLine(ImVec2(c.x - xH, c.y - xH), ImVec2(c.x + xH, c.y + xH), xc, 3.0f);
            dl->AddLine(ImVec2(c.x + xH, c.y - xH), ImVec2(c.x - xH, c.y + xH), xc, 3.0f);
        }
    }

    SetCursorPos(ImVec2(0.0f, 0.0f));
}

// Reads an IL2CPP/Unity NSString (UTF-16 internal buffer at offset 0x14, length at 0x10)
static std::string ReadNSString(ptr str) {
    if (!str) return "null";
    int32_t len = F(int32_t, str + 0x10);
    if (len <= 0 || len > 512) return "?";
    std::string result;
    result.reserve(len);
    for (int32_t i = 0; i < len; i++) {
        uint16_t ch = F(uint16_t, str + 0x14 + i * 2);
        result += (ch > 0 && ch < 128) ? (char)ch : '?';
    }
    return result;
}

// Shared vertical position for DrawToggleButton and DrawFloatingButton (they move together)
static float g_sideBtnsY      = 0.0f;
// Kept for linker compatibility — no longer used for animation
static float g_toggleRotAngle = 0.0f;
// Set true by AutoPlay when in SLOW scan state — shows CALCULATING overlay
static bool  g_autoPlayCalculating = false;

// ── svConfig ──────────────────────────────────────────────────────────────────
static void svConfig_Save() {
    std::string path = O("/data/user/0/") + PACKAGE_NAME + O("/files/svConfig.txt");
    FILE* f = fopen(path.c_str(), O("w"));
    if (!f) return;
    fprintf(f, O("iLineThickness=%d\n"),  persistent_int[O("iLineThickness")]);
    fprintf(f, O("iMenuSizeOffset=%d\n"), persistent_int[O("iMenuSizeOffset")]);
    fclose(f);
}
static void svConfig_Load() {
    std::string path = O("/data/user/0/") + PACKAGE_NAME + O("/files/svConfig.txt");
    FILE* f = fopen(path.c_str(), O("r"));
    if (!f) return;
    char line[64];
    while (fgets(line, sizeof(line), f)) {
        int v = 0;
        if (sscanf(line, O("iLineThickness=%d"),  &v) == 1) { persistent_int[O("iLineThickness")]  = v; continue; }
        if (sscanf(line, O("iMenuSizeOffset=%d"), &v) == 1) { persistent_int[O("iMenuSizeOffset")] = v; }
    }
    fclose(f);
}

// ── CALCULATING overlay (shown during AutoPlay SLOW scan) ─────────────────────
static void DrawCalculating(ImGuiIO& io) {
    // Setăm poziția pe centrul ecranului (Width*0.5, Height*0.5)
    // Pivotul (0.5f, 0.5f) înseamnă că mijlocul ferestrei va fi fix pe coordonatele date
    SetNextWindowPos(ImVec2(Width * 0.5f, Height * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    
    // Auto-resize face ca fereastra să aibă dimensiunea textului automat
    PushStyleColor(ImGuiCol_WindowBg, IM_COL32(180, 0, 0, 255));
    PushStyleColor(ImGuiCol_Border, IM_COL32(220, 30, 30, 255));
    PushStyleVar(ImGuiStyleVar_WindowRounding, 18.0f);
    PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);

    if (Begin(O("##CalcOverlay"), nullptr,
              ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
              ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | 
              ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoInputs)) {
        
        SetWindowFontScale(1.4f);
        TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), O("CALCULATING..."));
        SetWindowFontScale(1.0f);
    }
    End();
    PopStyleVar(2);
    PopStyleColor(2);
}


// ============================================================
// New from menu3 (features you didn't have): shot preview overlay
// + toast notifications / cue-ball scratch-danger alert.
// Gated behind their own toggles below.
// ============================================================
static bool g_scratchDetected = false;
static double g_scratchDetectTime = 0.0;
static int g_scratchAttemptCount = 0;
static void DrawShotPreview(ImDrawList* draw) {
    if (!persistent_bool[O("bShotPreview")]) return;
    if (AutoPlay::g_CurrentCandidate.idx == -1) return;
    if (AutoPlay::state == AutoPlay::IDLE) return;
    
    int targetIdx = AutoPlay::g_CurrentCandidate.idx;
    int pocketIdx = AutoPlay::g_CurrentCandidate.pocketIndex;
    
    if (targetIdx < 0 || targetIdx >= gPrediction->guiData.ballsCount) return;
    if (pocketIdx < 0 || pocketIdx >= 6) return;
    
    auto& targetBall = gPrediction->guiData.balls[targetIdx];
    auto& cueBall = gPrediction->guiData.balls[0];
    
    // 1. موقع الكرة المستهدفة على الشاشة
    ImVec2 targetPos = WorldToScreen(targetBall.initialPosition);
    ImVec2 pocketPos = GetPocketScreenPos(pocketIdx);
    ImVec2 cuePos = WorldToScreen(cueBall.initialPosition);
    
    // 2. رسم دائرة حول الكرة المستهدفة
    float pulse = sinf(ImGui::GetTime() * 4.0f) * 0.5f + 0.5f;
    float ballSize = persistent_float[O("fESP_BallSize")] * 1.5f;
    
    draw->AddCircle(targetPos, ballSize + pulse * 8.0f, 
                    IM_COL32(0, 255, 200, (int)(150 + 100 * pulse)), 0, 3.0f);
    draw->AddCircle(targetPos, ballSize + pulse * 4.0f, 
                    IM_COL32(0, 255, 200, 255), 0, 2.0f);
    
    // 3. رسم سهم إلى الجيب
    ImVec2 dir = ImVec2(pocketPos.x - targetPos.x, pocketPos.y - targetPos.y);
    float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
    if (len > 1.0f) {
        dir.x /= len;
        dir.y /= len;
        
        // خط متقطع
        float dashLen = 20.0f;
        float gapLen = 15.0f;
        float totalLen = len;
        float phase = fmodf(ImGui::GetTime() * 100.0f, dashLen + gapLen);
        for (float t = phase; t < totalLen; t += dashLen + gapLen) {
            float start = t;
            float end = t + dashLen;
            if (start > totalLen) break;
            if (end > totalLen) end = totalLen;
            ImVec2 p1 = ImVec2(targetPos.x + dir.x * start, targetPos.y + dir.y * start);
            ImVec2 p2 = ImVec2(targetPos.x + dir.x * end, targetPos.y + dir.y * end);
            draw->AddLine(p1, p2, IM_COL32(0, 255, 200, 200), 2.0f);
        }
        
        // رأس السهم
        float arrowSize = 15.0f;
        ImVec2 arrowTip = pocketPos;
        ImVec2 arrowBase = ImVec2(pocketPos.x - dir.x * arrowSize, pocketPos.y - dir.y * arrowSize);
        ImVec2 perp = ImVec2(-dir.y, dir.x);
        draw->AddTriangleFilled(arrowTip, 
                                ImVec2(arrowBase.x + perp.x * arrowSize * 0.4f, 
                                       arrowBase.y + perp.y * arrowSize * 0.4f),
                                ImVec2(arrowBase.x - perp.x * arrowSize * 0.4f, 
                                       arrowBase.y - perp.y * arrowSize * 0.4f),
                                IM_COL32(0, 255, 200, 255));
    }
    
    // 4. حساب نسبة نجاح الضربة
    float quality = 0.0f;
    Vec2d diff = targetBall.predictedPosition - targetBall.initialPosition;
    float dist = sqrtf(diff.x * diff.x + diff.y * diff.y);
    
    if (dist < 0.5f) quality += 40.0f;
    else if (dist < 1.0f) quality += 30.0f;
    else if (dist < 2.0f) quality += 20.0f;
    else quality += 10.0f;
    
    float power = sharedGameManager.mVisualCue().getShotPower();
    if (power > 30.0f && power < 70.0f) quality += 30.0f;
    else if (power < 80.0f) quality += 20.0f;
    else quality += 10.0f;
    
    // زاوية الدخول للجيب
    quality += 20.0f;
    
    quality = ImClamp(quality, 0.0f, 100.0f);
    
    // 5. عرض المعلومات على الشاشة
    ImVec2 infoPos = ImVec2(20.0f, 20.0f);
    ImU32 qualityColor = quality > 70.0f ? IM_COL32(0, 255, 100, 255) :
                         quality > 40.0f ? IM_COL32(255, 200, 0, 255) :
                                           IM_COL32(255, 50, 50, 255);
    
    char buf[128];
    snprintf(buf, sizeof(buf), "🎯 Target: Ball #%d", targetIdx);
    draw->AddText(infoPos, IM_COL32(255, 255, 255, 255), buf);
    infoPos.y += 25.0f;
    
    snprintf(buf, sizeof(buf), "📊 Quality: %.0f%%", quality);
    draw->AddText(infoPos, qualityColor, buf);
    infoPos.y += 25.0f;
    
    snprintf(buf, sizeof(buf), "📍 Pocket: %d", pocketIdx + 1);
    draw->AddText(infoPos, IM_COL32(200, 200, 200, 255), buf);
}

static void ApplyShotPreview() {
    if (!persistent_bool[O("bShotPreview")]) return;
    // يمكن إضافة تعديلات بسيطة هنا إذا لزم الأمر
}

struct ToastMsg {
    std::string text;
    float alpha;
    float targetY;
    float currentY;
    double spawnTime;
};
static std::vector<ToastMsg> g_Toasts;
static double g_lastToastTime = 0.0;
static int g_toastState = 0; // 0: idle, 1: scanning, 2: found

static void PushToast(const std::string& text) {
    for (auto& t : g_Toasts) {
        t.targetY -= 85.0f; // Shift older toasts up higher to avoid overlap with bigger boxes
    }
    ToastMsg t;
    t.text = text;
    t.alpha = 0.0f;
    t.targetY = ImGui::GetIO().DisplaySize.y - 140.0f; // Lifted much higher to avoid bottom screen/nav bar clipping
    t.currentY = t.targetY + 40.0f; // Start further down for a smoother slide-up spawn
    t.spawnTime = ImGui::GetTime();
    g_Toasts.push_back(t);
    g_lastToastTime = ImGui::GetTime();
}

// ============================================================
// SCRATCH DETECTION - FUNCTIONS
// ============================================================
static void ResetScratchAlert() {
    g_scratchDetected = false;
    g_scratchAttemptCount = 0;
}

static bool IsCueBallInDanger() {
    if (!sharedGameManager) return false;
    
    const int MAX_BALLS = 16;
    auto& balls = gPrediction->guiData.balls;
    
    auto& cueBall = balls[0];
    if (cueBall.initialPosition == cueBall.predictedPosition) return false;
    
    Table table = sharedGameManager.mTable;
    if (!table) return false;
    
    auto tableProperties = table.mTableProperties();
    if (!tableProperties) return false;
    
    auto& pockets = tableProperties.mPockets();
    
    for (int i = 0; i < 6; i++) {
        if (Prediction::pocketStatus[i]) {
            Vec2d diff = cueBall.predictedPosition - pockets[i];
            float dist = sqrtf(diff.x * diff.x + diff.y * diff.y);
            if (dist < 0.3f) {
                return true;
            }
        }
    }
    
    return false;
}

static bool FindAlternativeBall() {
    if (!sharedGameManager) return false;
    
    const int MAX_BALLS = 16;
    auto& balls = gPrediction->guiData.balls;
    
    int currentTarget = AutoPlay::g_CurrentCandidate.idx;
    
    Table table = sharedGameManager.mTable;
    if (!table) return false;
    
    auto tableProperties = table.mTableProperties();
    if (!tableProperties) return false;
    
    auto& pockets = tableProperties.mPockets();
    
    for (int i = 1; i < MAX_BALLS; i++) {
        if (i == currentTarget) continue;
        if (balls[i].predictedPosition == balls[i].initialPosition) continue;
        
        bool isSafe = true;
        for (int p = 0; p < 6; p++) {
            if (Prediction::pocketStatus[p]) {
                Vec2d diff = balls[i].predictedPosition - pockets[p];
                float dist = sqrtf(diff.x * diff.x + diff.y * diff.y);
                if (dist < 0.5f) {
                    isSafe = false;
                    break;
                }
            }
        }
        
        if (isSafe) {
            AutoPlay::g_CurrentCandidate.idx = i;
            AutoPlay::g_CurrentCandidate.pocketIndex = -1;
            return true;
        }
    }
    
    return false;
}
// ============================================================
// دالة DrawToasts المعدلة بالكامل
// ============================================================
static void DrawToasts(ImDrawList* draw) {
    if (!persistent_bool[O("bDisableFlicker")]) {
        if (!g_Toasts.empty()) { g_Toasts.clear(); g_toastState = 0; }
        ResetScratchAlert();
        return;
    }

    double now = ImGui::GetTime();
    bool isScanning = (AutoPlay::currentMode != AutoPlay::MODE_OFF && AutoPlay::state == AutoPlay::SCANNING);
    bool isFound = (AutoPlay::g_CurrentCandidate.idx != -1);

    // ============================================================
    // SCRATCH DETECTION - كشف خطر الكرة البيضاء (معدل بالكامل)
    // ============================================================
    static double g_lastScratchAttempt = 0.0;
    static const double SCRATCH_COOLDOWN = 0.5;

    if (AutoPlay::bAutoPlaying && isFound && !isScanning) {
        if (IsCueBallInDanger()) {
            double nowTime = ImGui::GetTime();
            if (nowTime - g_lastScratchAttempt < SCRATCH_COOLDOWN) {
                // فترة تهدئة - لا نفعل شيئاً
            } else if (!g_scratchDetected) {
                g_lastScratchAttempt = nowTime;
                g_scratchDetected = true;
                g_scratchDetectTime = now;
                g_scratchAttemptCount++;
                
                if (FindAlternativeBall()) {
                    // bAutoPlaySwitch / bAutoAimSwitch are the same dead flags
                    // documented in DrawToggleButton() above - declared in
                    // AutoPlay.h but never assigned true anywhere, so this
                    // reassignment was always a no-op. Use the real menu
                    // toggles instead, matching DrawESP.
                    if (persistent_bool[O("AutoPlay")]) {
                        AutoPlay::currentMode = AutoPlay::MODE_AUTO_PLAY;
                    } else if (persistent_bool[O("bAutoAim")]) {
                        AutoPlay::currentMode = AutoPlay::MODE_AUTO_AIM;
                    }
                    AutoPlay::ClearState();
                    
                    char alertMsg[128];
                    snprintf(alertMsg, sizeof(alertMsg), 
                             L("⚡ Switched to ball #%d", 
                               "⚡ تم التبديل للكرة رقم %d"), 
                             AutoPlay::g_CurrentCandidate.idx);
                    PushToast(alertMsg);
                } else {
                    PushToast(L("🎯 No safe ball! Aim carefully.", 
                               "🎯 لا توجد كرة آمنة! صوب بحذر."));
                }
                
                g_toastState = 3;
            }
        } else {
            if (g_scratchDetected) {
                ResetScratchAlert();
            }
        }
    }

    // ============================================================
    // 🚨 SCRATCH ALERT UI - تحذير الخطر (نسخة VIP)
    // ============================================================
    if (g_scratchDetected) {
        float scale = ImClamp(ImGui::GetIO().DisplaySize.y / 1080.0f, 0.70f, 1.1f);
        float size = 200.0f * scale;
        
        // 📍 أعلى الوسط
        float x = (ImGui::GetIO().DisplaySize.x - size) * 0.5f;
        float y = 35.0f * scale;
        
        ImVec2 center = ImVec2(x + size * 0.5f, y + size * 0.5f);
        float radius = size * 0.42f;
        float flash = sinf(now * 8.0f) * 0.5f + 0.5f;
        
        // ===== خلفية متوهجة =====
        for (int i = 5; i > 0; i--) {
            float r = radius * 1.4f + i * 8.0f * scale;
            int alpha = (int)(15 * (1.0f - i * 0.15f) * (0.5f + 0.5f * flash));
            draw->AddCircleFilled(center, r, IM_COL32(255, 50, 50, alpha), 48);
        }
        
        draw->AddCircleFilled(center, radius * 1.2f, IM_COL32(20, 8, 8, 235));
        draw->AddCircleFilled(center, radius * 1.1f, IM_COL32(40, 10, 10, 220));
        
        // ===== حلقات نابضة حمراء =====
        for (int i = 0; i < 4; i++) {
            float pulse = fmod(now * 3.0f + i * 0.4f, 1.0f);
            float ringR = radius * 0.3f + pulse * radius * 0.7f;
            int alpha = (int)((1.0f - pulse) * 150);
            float thickness = 2.0f + (1.0f - pulse) * 4.0f;
            draw->AddCircle(center, ringR, IM_COL32(255, 50, 50, alpha), 0, thickness * scale);
        }
        
        // ===== ⚠️ أيقونة التحذير =====
        float iconSize = radius * 0.55f;
        
        // مثلث التحذير
        ImVec2 p1 = ImVec2(center.x, center.y - iconSize);
        ImVec2 p2 = ImVec2(center.x - iconSize * 0.85f, center.y + iconSize * 0.5f);
        ImVec2 p3 = ImVec2(center.x + iconSize * 0.85f, center.y + iconSize * 0.5f);
        
        draw->AddTriangleFilled(p1, p2, p3, IM_COL32(255, 200, 0, 230));
        draw->AddTriangle(p1, p2, p3, IM_COL32(200, 150, 0, 255), 2.5f * scale);
        
        // علامة التعجب
        float exW = 5.0f * scale;
        float exH = iconSize * 0.45f;
        float exX = center.x - exW * 0.5f;
        float exY = center.y - iconSize * 0.28f;
        draw->AddRectFilled(ImVec2(exX, exY), ImVec2(exX + exW, exY + exH), 
                            IM_COL32(0, 0, 0, 255), 2.0f * scale);
        
        float dotY = center.y + iconSize * 0.22f;
        draw->AddCircleFilled(ImVec2(center.x, dotY), exW * 0.8f, IM_COL32(0, 0, 0, 255));
        
        // ===== النص =====
        const char* warnText = L("⚠️ CUE BALL DANGER!", "⚠️ خطر الكرة البيضاء!");
        ImVec2 textSize = ImGui::CalcTextSize(warnText);
        float textX = center.x - textSize.x * 0.5f;
        float textY = y + size * 0.78f;
        
        // توهج النص
        for (int i = 0; i < 3; i++) {
            float offset = 2.0f * (1.0f - i * 0.3f);
            draw->AddText(ImVec2(textX + offset, textY + offset), 
                         IM_COL32(255, 50, 50, (int)(100 - i * 30)), warnText);
        }
        draw->AddText(ImVec2(textX, textY), IM_COL32(255, 100, 100, 255), warnText);
        
        // النص الثانوي
        const char* subText = g_scratchAttemptCount > 1 ? 
            L("🔄 Multiple attempts made!", "🔄 تمت محاولات متعددة!") :
            L("🎯 Switching to safe ball...", "🎯 جاري التبديل لكرة آمنة...");
        
        ImVec2 subSize = ImGui::CalcTextSize(subText);
        float subX = center.x - subSize.x * 0.5f;
        float subY = textY + 30.0f * scale;
        draw->AddText(ImVec2(subX, subY), IM_COL32(255, 200, 200, 200), subText);
        
        // ===== بريق متطاير =====
        for (int i = 0; i < 6; i++) {
            float angle = now * 2.0f + i * 1.047f;
            float dist = radius * (0.5f + 0.5f * (0.5f + 0.5f * sinf(now * 3.0f + i)));
            int alpha = (int)(100 + 155 * (0.5f + 0.5f * sinf(now * 5.0f + i * 2.3f)));
            float sparkleSize = 2.0f * scale + 4.0f * scale * (0.5f + 0.5f * sinf(now * 4.0f + i));
            
            ImVec2 sparklePos = ImVec2(center.x + cosf(angle) * dist, 
                                       center.y + sinf(angle) * dist);
            draw->AddCircleFilled(sparklePos, sparkleSize, 
                                 IM_COL32(255, 200, 100, alpha));
        }
        
        g_Toasts.clear();
        return;
    }

    // ============================================================
    // 🚀 SCANNING INDICATOR - RADAR (نسخة محسنة ومبسطة)
    // ============================================================
    if (isScanning) {
        float scale = ImClamp(ImGui::GetIO().DisplaySize.y / 1080.0f, 0.70f, 1.1f);
        float size = 160.0f * scale;
        
        // 📍 أعلى الوسط - فوق الطاولة
        float x = (ImGui::GetIO().DisplaySize.x - size) * 0.5f;
        float y = 50.0f * scale;
        
        ImVec2 center = ImVec2(x + size * 0.5f, y + size * 0.5f);
        float radius = size * 0.42f;
        float pulse = sinf(now * 2.5f) * 0.5f + 0.5f;
        
        // ===== 1. GLOWING BACKGROUND =====
        for (int i = 4; i > 0; i--) {
            float r = radius * 1.3f + i * 5.0f * scale;
            int alpha = (int)(12 * (1.0f - i * 0.2f) * (0.5f + 0.5f * pulse));
            draw->AddCircleFilled(center, r, IM_COL32(255, 45, 70, alpha), 48);
        }
        
        draw->AddCircleFilled(center, radius * 1.15f, IM_COL32(10, 10, 15, 220));
        draw->AddCircleFilled(center, radius * 1.05f, IM_COL32(18, 18, 25, 200));
        
        // ===== 2. OUTER GLOW RING =====
        for (int i = 0; i < 2; i++) {
            float ringR = radius * 1.1f + i * 4.0f * scale;
            int alpha = (int)(30 + 50 * pulse * (1.0f - i * 0.3f));
            draw->AddCircle(center, ringR, IM_COL32(255, 45, 70, alpha), 0, 2.0f - i * 0.5f);
        }
        
        // ===== 3. RADAR SWEEP (الماسح الدوار) =====
        float sweepAngle = fmod(now * 1.5f, 2.0f * IM_PI);
        float sweepWidth = IM_PI * 0.35f;
        
        for (int i = 8; i > 0; i--) {
            float a = sweepAngle - sweepWidth * (i / 10.0f);
            float alpha = (30.0f / (i + 2)) * (0.5f + 0.5f * pulse);
            float thickness = 2.5f - i * 0.15f;
            if (thickness < 0.5f) thickness = 0.5f;
            draw->PathArcTo(center, radius + i * 1.5f * scale, a - sweepWidth * 0.3f, a, 24);
            draw->PathStroke(IM_COL32(255, 45, 70, (int)alpha), false, thickness);
        }
        
        draw->PathArcTo(center, radius, sweepAngle - sweepWidth * 0.3f, sweepAngle, 24);
        draw->PathStroke(IM_COL32(255, 45, 70, 200), false, 3.5f * scale);
        
        float sweepLen = radius * 1.05f;
        draw->AddLine(center, 
                      ImVec2(center.x + cosf(sweepAngle) * sweepLen, 
                             center.y + sinf(sweepAngle) * sweepLen),
                      IM_COL32(255, 45, 70, (int)(120 + 80 * pulse)), 2.0f * scale);
        
        // ===== 4. CONCENTRIC RINGS (حلقات متحدة المركز) =====
        for (int i = 0; i < 2; i++) {
            float ringRadius = radius * (0.33f * (i + 1));
            float ringAlpha = 30 + 50 * (1.0f - i * 0.3f) * (0.5f + 0.5f * sinf(now * 2.0f + i * 2.0f));
            draw->AddCircle(center, ringRadius, IM_COL32(255, 45, 70, (int)ringAlpha), 0, 1.5f * scale);
            
            for (int j = 0; j < 6; j++) {
                float angle = now * 1.2f + i * 2.094f + j * 0.785f;
                ImVec2 dotPos = ImVec2(center.x + cosf(angle) * ringRadius, 
                                       center.y + sinf(angle) * ringRadius);
                int dotAlpha = (int)(100 + 120 * (0.5f + 0.5f * sinf(now * 3.0f + i * 1.5f + j)));
                draw->AddCircleFilled(dotPos, 3.0f * scale, IM_COL32(255, 45, 70, dotAlpha));
                draw->AddCircleFilled(dotPos, 6.0f * scale, IM_COL32(255, 45, 70, (int)(dotAlpha * 0.3f)));
            }
        }
        
        // ===== 5. CROSSHAIR (نقطة التقاطع) =====
        float crossLen = radius * 0.2f;
        ImU32 crossCol = IM_COL32(255, 45, 70, 180);
        draw->AddLine(ImVec2(center.x - crossLen, center.y), 
                      ImVec2(center.x + crossLen, center.y), crossCol, 1.2f * scale);
        draw->AddLine(ImVec2(center.x, center.y - crossLen), 
                      ImVec2(center.x, center.y + crossLen), crossCol, 1.2f * scale);
        
        // ===== 6. CENTER GLOW (القلب المتوهج) =====
        draw->AddCircleFilled(center, 3.0f * scale, IM_COL32(255, 255, 255, 255));
        draw->AddCircleFilled(center, 6.0f * scale, IM_COL32(255, 45, 70, 200));
        draw->AddCircleFilled(center, 12.0f * scale, IM_COL32(255, 45, 70, 100));
        draw->AddCircleFilled(center, 20.0f * scale, IM_COL32(255, 45, 70, 50));
        draw->AddCircleFilled(center, 30.0f * scale, IM_COL32(255, 45, 70, 20));
        
        // ===== 7. SCANNING DOTS (جسيمات دوارة حول الحافة) =====
        for (int i = 0; i < 4; i++) {
            float angle = now * 1.5f + i * 1.047f;
            float dist = radius * (0.85f + 0.15f * sinf(now * 2.0f + i * 1.2f));
            ImVec2 dotPos = ImVec2(center.x + cosf(angle) * dist, 
                                   center.y + sinf(angle) * dist);
            int alpha = (int)(120 + 100 * (0.5f + 0.5f * sinf(now * 4.0f + i)));
            draw->AddCircleFilled(dotPos, 4.0f * scale, IM_COL32(255, 45, 70, alpha));
            draw->AddCircleFilled(dotPos, 8.0f * scale, IM_COL32(255, 45, 70, (int)(alpha * 0.25f)));
        }
        
        // ===== 8. TEXT - "SCANNING" =====
        std::string scanText = L("⚡ SCANNING", "⚡ جاري المسح");
        ImVec2 textSize = ImGui::CalcTextSize(scanText.c_str());
        
        float textX = center.x - textSize.x * 0.5f;
        float textY = y + size * 0.80f;
        
        for (int i = 0; i < 3; i++) {
            float offset = 1.5f * (1.0f - i * 0.3f);
            draw->AddText(ImVec2(textX + offset, textY + offset), 
                         IM_COL32(0, 0, 0, (int)(120 - i * 30)), scanText.c_str());
        }
        
        draw->AddText(ImVec2(textX, textY), IM_COL32(255, 45, 70, 255), scanText.c_str());
        draw->AddText(ImVec2(textX, textY), IM_COL32(255, 255, 255, (int)(80 + 80 * pulse)), scanText.c_str());
        
        // ===== 9. OUTER BORDER GLOW =====
        for (int i = 0; i < 3; i++) {
            float r = radius * 1.18f + i * 2.5f * scale;
            int alpha = (int)(15 + 30 * pulse * (1.0f - i * 0.3f));
            draw->AddCircle(center, r, IM_COL32(255, 45, 70, alpha), 0, 1.2f - i * 0.3f);
        }
        
        // ===== 10. SPARKLE EFFECT (تأثير بريق خفيف) =====
        for (int i = 0; i < 4; i++) {
            float angle = now * 0.7f + i * 0.785f;
            float dist = radius * (0.3f + 0.7f * (0.5f + 0.5f * sinf(now * 2.0f + i * 1.3f)));
            float sparkleSize = 2.0f * scale + 2.0f * scale * (0.5f + 0.5f * sinf(now * 5.0f + i * 2.1f));
            int alpha = (int)(60 + 140 * (0.5f + 0.5f * sinf(now * 3.0f + i * 1.7f)));
            
            ImVec2 sparklePos = ImVec2(center.x + cosf(angle) * dist, 
                                       center.y + sinf(angle) * dist);
            draw->AddCircleFilled(sparklePos, sparkleSize, IM_COL32(255, 255, 255, alpha));
        }
        
        g_Toasts.clear();
        g_toastState = 1;
        return;
    }

    // ============================================================
    // SHOT FOUND! TOAST
    // ============================================================
    if (isFound) {
        if (g_toastState != 2) {
            g_Toasts.clear();
            PushToast(L("⚡ SHOT FOUND!", "⚡ تم العثور على ضربة!"));
            g_toastState = 2;
            g_lastToastTime = now;
        }
        
        if (now - g_lastToastTime > 2.0) {
            g_Toasts.clear();
            g_toastState = 0;
        }
    }

    // ============================================================
    // DRAW TOASTS (with premium styling)
    // ============================================================
    float dt = ImGui::GetIO().DeltaTime;
    
    for (int i = 0; i < (int)g_Toasts.size(); i++) {
        auto& t = g_Toasts[i];
        
        bool isLatest = (i == (int)g_Toasts.size() - 1);
        
        if (!isLatest || (now - t.spawnTime > 1.8 && g_toastState != 2)) {
            t.alpha -= dt * 2.8f;
        } else {
            t.alpha += dt * 5.0f;
        }
        t.alpha = ImClamp(t.alpha, 0.0f, 1.0f);
        
        t.currentY += (t.targetY - t.currentY) * 8.0f * dt;
        
        if (t.alpha <= 0.01f) {
            g_Toasts.erase(g_Toasts.begin() + i);
            i--;
            continue;
        }
        
        ImFont* font = ImGui::GetFont();
        float fontSize = font->FontSize * 1.3f;
        ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, t.text.c_str());
        
        float paddingX = 32.0f;
        float paddingY = 20.0f;
        float boxW = textSize.x + paddingX * 2.0f;
        float boxH = textSize.y + paddingY * 2.0f;
        
        float x = ImGui::GetIO().DisplaySize.x - boxW - 25.0f;
        float y = t.currentY;
        
        // ===== PREMIUM TOAST STYLING =====
        
        // 1. Glass-morphism background
        ImU32 bgCol = IM_COL32(15, 15, 20, (int)(t.alpha * 235));
        draw->AddRectFilled(ImVec2(x, y), ImVec2(x + boxW, y + boxH), bgCol, 10.0f);
        
        // 2. Gradient border - top shine
        draw->AddRectFilledMultiColor(
            ImVec2(x + 4, y + 2), ImVec2(x + boxW - 4, y + 6),
            IM_COL32(255, 255, 255, (int)(20 * t.alpha)),
            IM_COL32(255, 255, 255, (int)(5 * t.alpha)),
            IM_COL32(255, 255, 255, (int)(5 * t.alpha)),
            IM_COL32(255, 255, 255, (int)(20 * t.alpha))
        );
        
        // 3. Main border with animated glow
        float t_anim = (float)now * 2.5f;
        ImVec2 p_min = ImVec2(x, y);
        ImVec2 p_max = ImVec2(x + boxW, y + boxH);
        float perimeter = (boxW * 2.0f) + (boxH * 2.0f);
        float runLen = 70.0f;
        
        draw->AddRect(p_min, p_max, IM_COL32(255, 45, 70, (int)(80 * t.alpha)), 10.0f, 0, 1.5f);
        
        float offset = fmodf(t_anim * 160.0f, perimeter);
        
        ImVec2 glowMin = p_min;
        ImVec2 glowMax = p_max;
        
        if (offset < boxW) {
            glowMin.x = p_min.x + offset - runLen;
            glowMax.x = p_min.x + offset;
            glowMin.y = p_min.y - 10.0f;
            glowMax.y = p_min.y + 10.0f;
        } else if (offset < boxW + boxH) {
            glowMin.x = p_max.x - 10.0f;
            glowMax.x = p_max.x + 10.0f;
            glowMin.y = p_min.y + (offset - boxW) - runLen;
            glowMax.y = p_min.y + (offset - boxW);
        } else if (offset < boxW * 2.0f + boxH) {
            glowMin.x = p_max.x - (offset - boxW - boxH);
            glowMax.x = p_max.x - (offset - boxW - boxH) + runLen;
            glowMin.y = p_max.y - 10.0f;
            glowMax.y = p_max.y + 10.0f;
        } else {
            glowMin.x = p_min.x - 10.0f;
            glowMax.x = p_min.x + 10.0f;
            glowMin.y = p_max.y - (offset - boxW * 2.0f - boxH);
            glowMax.y = p_max.y - (offset - boxW * 2.0f - boxH) + runLen;
        }
        
        draw->PushClipRect(
            ImVec2(ImMax(p_min.x, glowMin.x), ImMax(p_min.y, glowMin.y)),
            ImVec2(ImMin(p_max.x, glowMax.x), ImMin(p_max.y, glowMax.y)), 
            true
        );
        draw->AddRect(p_min, p_max, IM_COL32(255, 45, 70, (int)(255 * t.alpha)), 10.0f, 0, 2.5f);
        draw->PopClipRect();

        // 4. Left accent bar
        float accentHeight = boxH * 0.4f;
        float accentY = y + (boxH - accentHeight) * 0.5f;
        draw->AddRectFilled(
            ImVec2(x + 2, accentY),
            ImVec2(x + 6, accentY + accentHeight),
            IM_COL32(255, 45, 70, (int)(220 * t.alpha)),
            3.0f
        );

        // 5. Text with glow
        ImVec2 textPos = ImVec2(x + paddingX + 4, y + paddingY);
        draw->AddText(font, fontSize, ImVec2(textPos.x + 1, textPos.y + 1), 
                     IM_COL32(0, 0, 0, (int)(180 * t.alpha)), t.text.c_str());
        draw->AddText(font, fontSize, textPos, 
                     IM_COL32(255, 255, 255, (int)(255 * t.alpha)), t.text.c_str());
        
        // 6. Sparkle effect for "Found!" toasts
        if (g_toastState == 2) {
            float sparkle = sinf(now * 8.0f + i * 2.0f) * 0.5f + 0.5f;
            ImU32 sparkleCol = IM_COL32(255, 215, 0, (int)(sparkle * 80 * t.alpha));
            
            for (int s = 0; s < 3; s++) {
                float angle = now * 2.0f + s * 2.094f + i * 0.5f;
                float dist = 12.0f + 8.0f * sparkle;
                ImVec2 sparklePos = ImVec2(
                    x + boxW * 0.5f + cosf(angle) * dist,
                    y + boxH * 0.5f + sinf(angle) * dist
                );
                draw->AddCircleFilled(sparklePos, 2.0f + sparkle * 2.0f, sparkleCol);
            }
        }
    }
}

static void DrawContentArea(float winW, float winH) {
    bool need_save = false;
    
    ImDrawList* dl  = GetWindowDrawList();
    ImVec2      wp  = GetWindowPos();

    // startY: content now starts at the top (rail sits on the right, doesn't
    // consume cursor space) instead of below a horizontal top banner.
    float startY   = GetCursorPosY();
    float contentW = winW - RAIL_W;

    // Desenăm fundalul zonei de conținut sub sidebar
    dl->AddRectFilled(
        ImVec2(wp.x, wp.y + startY),
        ImVec2(wp.x + contentW, wp.y + winH),
        Theme::Crimson, 20.0f
    );
    
    const char* tabTitles[] = { 
    O("Draw"), 
    O("Play"), 
    O("Queue"), 
    O("User") 
};

    // --- CENTRARE TITLU TAB ---
    const char* currentTitle = tabTitles[g_menu.currentTab];
    float titlePadT = 18.0f;
    float titlePadB = 12.0f;

    // 1. Setăm scara fontului înainte de calcul
    SetWindowFontScale(1.15f);
    ImVec2 ts = CalcTextSize(currentTitle);
    
    // 2. Calculăm X pentru centrare: (Lățime fereastră - Lățime text) / 2
    float centeredX = (contentW - ts.x) * 0.5f;
    SetCursorPos(ImVec2(centeredX, startY + titlePadT));
    
    // 3. Afișăm textul
    TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", currentTitle);
    SetWindowFontScale(1.0f); // Resetăm imediat

    // Linie separatoare centrată și ea (lăsăm 20px margini)
    float lineY = startY + titlePadT + ts.y + titlePadB;
    dl->AddLine(
        ImVec2(wp.x + 20.0f, wp.y + lineY),
        ImVec2(wp.x + contentW - 20.0f, wp.y + lineY),
        IM_COL32(255, 90, 100, 110), 1.0f
    );

    float headerH = (lineY - startY) + 10.0f;
    SetCursorPos(ImVec2(10.0f, startY + headerH));
    
    // Începutul zonei de child (conținutul propriu-zis)
    PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
    BeginChild(O("##ContentArea"), ImVec2(contentW - 20.0f, winH - startY - headerH - 10.0f), false);
    
    switch (g_menu.currentTab) {
        case 0: {
            Dummy(ImVec2(0, 10));
            need_save |= ToggleSwitch(O("Munu Lines"), &persistent_bool[O("bESP_DrawPredictionLine")]);
            need_save |= ToggleSwitch(O("Pockets"), &persistent_bool[O("bESP_DrawPocketsShotState")]);
            need_save |= ToggleSwitch(O("Shot Preview"), &persistent_bool[O("bShotPreview")]);
            need_save |= ToggleSwitch(O("Scratch Alerts"), &persistent_bool[O("bDisableFlicker")]);

            Dummy(ImVec2(0, 16));
            TextColored(ImVec4(0.75f, 0.75f, 0.8f, 1.0f), O("Line Thickness"));
            Dummy(ImVec2(0, 8));
            {
                if (persistent_int[O("iLineThickness")] < 1) persistent_int[O("iLineThickness")] = 2;
                PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
                PushStyleVar(ImGuiStyleVar_GrabRounding, 10.0f);
                PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.12f, 0.15f, 1.0f));
                PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.0f, 0, 0, 1.0f));
                PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(1.0f, 0, 0, 1.0f));
                SetNextItemWidth(GetContentRegionAvail().x);
                need_save |= SliderInt(O("##lineThick"), &persistent_int[O("iLineThickness")], 1, 10, "%d");
                PopStyleColor(3);
                PopStyleVar(2);
            }

            Dummy(ImVec2(0, 16));
            TextColored(ImVec4(0.75f, 0.75f, 0.8f, 1.0f), O("Fix Menu Size"));
            Dummy(ImVec2(0, 8));
            {
                PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
                PushStyleVar(ImGuiStyleVar_GrabRounding, 10.0f);
                PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.12f, 0.15f, 1.0f));
                PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.0f, 0, 0, 1.0f));
                PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(1.0f, 0, 0, 1.0f));
                SetNextItemWidth(GetContentRegionAvail().x);
                int& menuSz = persistent_int[O("iMenuSizeOffset")];
                bool changed = SliderInt(O("##menuSize"), &menuSz, -10, 10,
                    menuSz == 0 ? O("Normal") : "%d");
                need_save |= changed;
                PopStyleColor(3);
                PopStyleVar(2);
            }

            Dummy(ImVec2(0, 20));
            {
                PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
                PushStyleColor(ImGuiCol_Button,        ImVec4(0.12f, 0.55f, 0.20f, 1.0f));
                PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.16f, 0.68f, 0.26f, 1.0f));
                PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.09f, 0.42f, 0.15f, 1.0f));
                if (Button(O("Zoom in+Zoom out"), ImVec2(GetContentRegionAvail().x, 55.0f))) {
                    svConfig_Save();
                }
                PopStyleColor(3);
                PopStyleVar();
            }
            break;
        }
        
        case 1: {
            Dummy(ImVec2(0, 10));
            need_save |= ToggleSwitch(O("AutoPlay"), &persistent_bool[O("AutoPlay")]);
            Dummy(ImVec2(0, 20));
            TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), O("Auto Play"));
            TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), O("Aim And Shoot"));

            Dummy(ImVec2(0, 20));
            TextColored(ImVec4(0.75f, 0.75f, 0.8f, 1.0f), O("Play Style"));
            Dummy(ImVec2(0, 8));
            {
                if (!persistent_int.count(O("iPlayStyle"))) persistent_int[O("iPlayStyle")] = 0;
                PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
                PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(15, 12));
                PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.30f, 0.02f, 0.06f, 1.0f));
                PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.38f, 0.04f, 0.09f, 1.0f));
                SetNextItemWidth(GetContentRegionAvail().x);
                if (Combo(O("##playstyle"), &persistent_int[O("iPlayStyle")], "Human Play\0Instant Play\0")) {
                    AutoPlay::playStyle = (AutoPlay::PlayStyle)persistent_int[O("iPlayStyle")];
                    need_save = true;
                }
                PopStyleColor(2);
                PopStyleVar(2);
            }
            AutoPlay::playStyle = (AutoPlay::PlayStyle)persistent_int[O("iPlayStyle")];

            Dummy(ImVec2(0, 20));
            need_save |= ToggleSwitch(O("Auto Aim"), &persistent_bool[O("bAutoAim")]);
            TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), O("Manual Angle Nudge"));
            break;
        }
        
        case 2: {
            Dummy(ImVec2(0, 10));
            need_save |= ToggleSwitch(O("Queue"), &persistent_bool[O("AutoQueue")]);
            Dummy(ImVec2(0, 20));
            
            TextColored(ImVec4(0.75f, 0.75f, 0.8f, 1.0f), O("Mode"));
            Dummy(ImVec2(0, 8));
            PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
            PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(15, 12));
            PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.12f, 0.15f, 1.0f));
            PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.16f, 0.16f, 0.20f, 1.0f));
            SetNextItemWidth(GetContentRegionAvail().x);
            need_save |= Combo("##mode", &persistent_int["iAutoQueue_Mode"], "Last Selected\0Smart\0Fix Table\0");
            PopStyleColor(2);
            PopStyleVar(2);
            
            if (persistent_int["iAutoQueue_Mode"] == 1) {
                Dummy(ImVec2(0, 15));
                TextColored(ImVec4(0.75f, 0.75f, 0.8f, 1.0f), O("Bet Percent"));
                Dummy(ImVec2(0, 8));
                PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
                PushStyleVar(ImGuiStyleVar_GrabRounding, 10.0f);
                PushStyleColor(ImGuiCol_FrameBg,
    ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.0f, 0, 0, 1.0f));
                PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(1.0f, 0, 0, 1.0f));
                SetNextItemWidth(GetContentRegionAvail().x);
                need_save |= SliderInt("##betpercent", &persistent_int["iAutoQueue_BetPercent"], 1, 100, "%d%%");
                PopStyleColor(3);
                PopStyleVar(2);
            }

            if (persistent_int["iAutoQueue_Mode"] == 2) {
                Dummy(ImVec2(0, 15));
                TextColored(ImVec4(0.75f, 0.75f, 0.8f, 1.0f), O("Select Table"));
                Dummy(ImVec2(0, 8));

                struct TableEntry { const char* label; ImU32 bg; ImU32 bgHov; };
                static const TableEntry tables[17] = {
                    { "100",   IM_COL32( 55,  90, 200, 255), IM_COL32( 75, 110, 220, 255) }, // M1  Blue
                    { "200",   IM_COL32( 40, 150,  65, 255), IM_COL32( 55, 170,  80, 255) }, // M2  Green
                    { "1k",    IM_COL32( 55,  90, 200, 255), IM_COL32( 75, 110, 220, 255) }, // M3  Blue
                    { "2.5k",  IM_COL32(130,  25,  25, 255), IM_COL32(155,  40,  40, 255) }, // M4  Dark Red
                    { "10k",   IM_COL32( 35,  35,  38, 255), IM_COL32( 55,  55,  60, 255) }, // M5  Black
                    { "50k",   IM_COL32(110,   0,   0, 255), IM_COL32(135,  15,  15, 255) }, // M6  Maroon
                    { "100k",  IM_COL32(140, 140, 145, 255), IM_COL32(160, 160, 165, 255) }, // M7  Light Grey
                    { "500k",  IM_COL32(185, 160,   0, 255), IM_COL32(210, 185,  10, 255) }, // M8  Yellow
                    { "1M",    IM_COL32( 20,  45, 130, 255), IM_COL32( 35,  60, 155, 255) }, // M9  Dark Blue
                    { "2M",    IM_COL32(190,  90,  15, 255), IM_COL32(215, 110,  30, 255) }, // M10 Dark Orange
                    { "5M",    IM_COL32(  0, 148, 110, 255), IM_COL32( 15, 170, 128, 255) }, // M11 Emerald
                    { "8M",    IM_COL32(165,  65,  65, 255), IM_COL32(185,  85,  85, 255) }, // M12 Light Maroon
                    { "10M",   IM_COL32( 18,  90,  35, 255), IM_COL32( 30, 112,  50, 255) }, // M13 Dark Green
                    { "20M",   IM_COL32(100, 100, 110, 255), IM_COL32(120, 120, 130, 255) }, // M14 Grey
                    { "30M",   IM_COL32(130,  15,  35, 255), IM_COL32(155,  30,  50, 255) }, // M15 Red Maroon
                    { "50M",   IM_COL32(  0, 148, 110, 255), IM_COL32( 15, 170, 128, 255) }, // M16 Emerald
                    { "200M",  IM_COL32( 20,  45, 130, 255), IM_COL32( 35,  60, 155, 255) }, // M17 Dark Blue
                };

                int& selected = persistent_int["iAutoQueue_FixTable"];
                float avail   = GetContentRegionAvail().x;
                int   cols    = 4;
                float gap     = 8.0f;
                float btnW    = (avail - gap * (cols - 1)) / cols;
                float btnH    = 42.0f;

                PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
                PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 6));

                for (int i = 0; i < 17; i++) {
                    if (i % cols != 0) SameLine(0, gap);

                    bool isSel = (selected == i);
                    ImU32 bgCol = isSel ? tables[i].bgHov : tables[i].bg;

                    PushStyleColor(ImGuiCol_Button,        (ImU32)bgCol);
                    PushStyleColor(ImGuiCol_ButtonHovered, (ImU32)tables[i].bgHov);
                    PushStyleColor(ImGuiCol_ButtonActive,  (ImU32)tables[i].bgHov);
                    PushStyleColor(ImGuiCol_Text,          isSel ? IM_COL32(255,255,255,255) : IM_COL32(220,220,220,200));

                    char btnId[32];
                    snprintf(btnId, sizeof(btnId), "%s##ft%d", tables[i].label, i);
                    if (Button(btnId, ImVec2(btnW, btnH))) {
                        selected = i;
                        need_save = true;
                    }

                    // Selected indicator: white outline
                    if (isSel) {
                        ImVec2 p = GetItemRectMin();
                        ImVec2 q = GetItemRectMax();
                        GetWindowDrawList()->AddRect(p, q, IM_COL32(255,255,255,200), 10.0f, 0, 2.0f);
                    }

                    PopStyleColor(4);
                }

                PopStyleVar(2);
            }

            if (persistent_int["iAutoQueue_Mode"] == 0) {
                Dummy(ImVec2(0, 15));
                TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), O("You Will Be Auto Queued To"));
                TextColored(ImVec4(0.5f, 0.5f, 0.55f, 1.0f), O("The Last Game Mode You Played"));
            }
            break;
        }

        case 3: {
            // ── helpers ──────────────────────────────────────────────────────
            auto DrawSectionHeader = [&](const char* title) {
                Dummy(ImVec2(0, 14));
                float avail = GetContentRegionAvail().x;
                ImVec2 p    = GetCursorScreenPos();
                float  fs   = GImGui->FontSize;
                ImVec2 ts   = CalcTextSize(title);
                float  lineY = p.y + fs * 0.5f;
                float  gap   = 8.0f;
                float  lineW = (avail - ts.x - gap * 2.0f) * 0.5f;
                ImDrawList* dl2 = GetWindowDrawList();
                dl2->AddLine(ImVec2(p.x,                      lineY), ImVec2(p.x + lineW,                      lineY), IM_COL32(60,60,75,160), 1.0f);
                dl2->AddLine(ImVec2(p.x + lineW + gap + ts.x + gap, lineY), ImVec2(p.x + avail, lineY), IM_COL32(60,60,75,160), 1.0f);
                SetCursorPosX(GetCursorPosX() + lineW + gap);
                TextColored(ImVec4(0.55f, 0.55f, 0.65f, 1.0f), "%s", title);
                Dummy(ImVec2(0, 6));
            };

            auto DrawInfoRow = [&](const char* key, const char* val) {
                TextColored(ImVec4(0.55f, 0.55f, 0.65f, 1.0f), "%s", key);
                SameLine();
                TextColored(ImVec4(0.90f, 0.90f, 0.95f, 1.0f), "%s", val);
                Dummy(ImVec2(0, 4));
            };

            // ── Device Info ───────────────────────────────────────────────────
            DrawSectionHeader(O("System Status")); {
                static char s_manufacturer[PROP_VALUE_MAX] = {};
                static char s_model[PROP_VALUE_MAX] = {};
                static char s_abi[PROP_VALUE_MAX] = {};
                static char s_android[PROP_VALUE_MAX] = {};
                static bool s_props_loaded = false;
                
                if (!s_props_loaded) {
                    __system_property_get("ro.product.manufacturer", s_manufacturer);
                    __system_property_get("ro.product.model", s_model);
                    __system_property_get("ro.product.cpu.abi", s_abi);
                    __system_property_get("ro.build.version.release", s_android);
                    s_props_loaded = true;}
                    
                int64_t now_ts = GetServerNow();
                int64_t diff = EXPIRY_TS - now_ts;
                char expireBuf[64];
                if (diff > 0) {
                    int64_t totalSecs = diff;
                    int days  = (int)(totalSecs / 86400);
                    int hours = (int)((totalSecs % 86400) / 3600);
                    int mins  = (int)((totalSecs % 3600)  / 60);
                    snprintf(expireBuf, sizeof(expireBuf), "%dd - %dh - %dm", days, hours, mins);
                } else {
                snprintf(expireBuf, sizeof(expireBuf), "%s", O("Expired"));}
                    
                DrawInfoRow(O("ManuFacturer: "), s_manufacturer);
                DrawInfoRow(O("Dragon"), s_model);
                DrawInfoRow(O("Abi:"), s_abi);
                DrawInfoRow(O("Saboort"), s_android);
                DrawInfoRow(O("KeY TIME "), persistent_string["key"].c_str());
                DrawInfoRow(O("TIME"), expireBuf);}


            break;
        }
    }
    
    if (need_save) save_persistence();
    
    EndChild();
    PopStyleColor();
}

// مربع تشخيصي بيظهر دايمًا في ركن الشاشة، بيحسب أوفست GameManager
// لايف من الفرق بين العنوان الحقيقي بتاعه والـ libmain base — مؤقت،
// شيله بعد ما تاخد الرقم وتثبته في Offsets.h
INLINE void DrawOffsetDebug() {
    ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.85f);
    if (Begin(O("##OffsetDebug"), nullptr,
              ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
              ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
              ImGuiWindowFlags_AlwaysAutoResize)) {

        Text(O("libmain: 0x%llx"), (unsigned long long)libmain);

        ptr gm = sharedGameManager.instance;
        Text(O("GameManager: 0x%llx"), (unsigned long long)gm);

        if (gm && libmain && gm > libmain) {
            ptr offset = gm - libmain;
            TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), O("Offset: 0x%llx"), (unsigned long long)offset);
        } else {
            TextColored(ImVec4(1.0f, 0.5f, 0.3f, 1.0f), O("Offset: waiting for shot..."));
        }
    }
    End();
}

INLINE void DrawMenu(ImGuiIO& io) {
    if ((g_ExpiryTime.load() > 0) || DEBUG_BYPASS_LOGIN) {
        if (is_segv_handler_active()) {
            jump_buffer_active = 1;
            if (!sigsetjmp(jump_buffer, 1)) DrawESP(GetBackgroundDrawList());
            jump_buffer_active = 0;
        }

        DrawOffsetDebug();

        float targetAlpha = g_menu.isOpen ? 1.0f : 0.0f;
        if (g_menu.isOpen) {
            g_menu.menuAlpha += (1.0f - g_menu.menuAlpha) * io.DeltaTime * 12.0f;
        } else {
            g_menu.menuAlpha = 0.0f;
        }

        if (g_menu.menuAlpha > 0.01f) {
            float sizeScale = 1.0f + (float)persistent_int[O("iMenuSizeOffset")] * 0.03f;
            if (sizeScale < 0.3f) sizeScale = 0.3f;
            float winW = g_menu.sidebarWidth * sizeScale;
            float winH = 560.0f * sizeScale;
            
            SetNextWindowSize(ImVec2(winW, winH), ImGuiCond_Always);
            SetNextWindowPos(ImVec2(Width / 2.0f, Height / 2.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            
            PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.13f, 0.f));
            PushStyleVar(ImGuiStyleVar_WindowRounding, 16.0f);
            PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            PushStyleVar(ImGuiStyleVar_Alpha, g_menu.menuAlpha);
            
            ImGuiWindowFlags winFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
                                        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            
            if (Begin(O("##MainMenu"), &g_menu.isOpen, winFlags)) {
                ImDrawList* dlPanel = GetWindowDrawList();
                ImVec2 wpPanel = GetWindowPos();
                ImVec2 wsPanel = GetWindowSize();
                DrawGlassFrameBorder(dlPanel, wpPanel, ImVec2(wpPanel.x + wsPanel.x, wpPanel.y + wsPanel.y), 16.0f, g_menu.menuAlpha);

                DrawSidebar(winW, winH);
                DrawContentArea(winW, winH);
            }
            End();
            
            PopStyleVar(4);
            PopStyleColor();
        }
    }
}

// ــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــ //

static void DrawToggleButton(bool cancelMode) {
    // الزرار يظهر فقط لما الاوتو بلاي switch مفعل أو في cancel mode
    if (g_menu.isOpen) return;
    // was gated on AutoPlay::bAutoPlaySwitch, which is declared in AutoPlay.h
    // but never assigned true anywhere in the project — the button (and its
    // play_on/play_off icon) could never appear. The actual menu toggle is
    // persistent_bool["AutoPlay"]; check that instead, matching what DrawESP
    // already uses to decide whether to call this function in the first place.
    if (!cancelMode && !persistent_bool[O("AutoPlay")]) return;

    ImGuiIO& io = GetIO();
    static GLuint play_on_tex  = LoadTextureFromMemory(play_on_png,  play_on_png_len);
    static GLuint play_off_tex = LoadTextureFromMemory(play_off_png, play_off_png_len);
    static GLuint queue_cancel_tex = LoadTextureFromMemory(play_on_png, play_on_png_len);

    // أبعاد الزرار المستطيل (ON/OFF)
    const float BTN_W       = 220.0f;
    const float BTN_H       = 85.0f;
    const float rightMargin = 20.0f;
    const float padding     = GetStyle().WindowPadding.x * 2.0f;

    float winW = BTN_W + padding;
    float winH = BTN_H + GetStyle().WindowPadding.y * 2.0f;

    // يتبع اللوجو: تحته مباشرة بمسافة 10px
    // g_sideBtnsY هو موقع زرار القديم — نحط الزرار الجديد تحت اللوجو
    float logoWinSize = (65.0f * 2.0f) + 10.0f;
    float logoFixedX  = io.DisplaySize.x - rightMargin - (130.f + GetStyle().WindowPadding.x * 2.0f)
                        + ((130.f + GetStyle().WindowPadding.x * 2.0f) - logoWinSize) * 0.5f;
    float logoPosY    = g_sideBtnsY - 140.0f;

    // الزرار تحت اللوجو علطول
    float posY = logoPosY + logoWinSize + 8.0f;
    float fixedX = io.DisplaySize.x - rightMargin - winW;

    SetNextWindowSize(ImVec2(winW, winH), ImGuiCond_Always);
    SetNextWindowPos(ImVec2(fixedX, posY), ImGuiCond_Always);

    PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 0));
    PushStyleColor(ImGuiCol_Border,   IM_COL32(0, 0, 0, 0));
    PushStyleVar(ImGuiStyleVar_WindowRounding, BTN_H * 0.5f);
    PushStyleVar(ImGuiStyleVar_WindowPadding,  ImVec2(4, 4));

    if (Begin(O("##ToggleBtn"), nullptr,
              ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
              ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove)) {

        ImVec2 pos  = GetCursorScreenPos();
        ImVec2 size(BTN_W, BTN_H);

        if (InvisibleButton(O("##TglBtnHit"), size)) {
            if (cancelMode) {
                persistent_bool[O("AutoQueue")] = false;
                g_aqCounting = false;
            } else {
                AutoPlay::bAutoPlaying = !AutoPlay::bAutoPlaying;
                if (AutoPlay::bAutoPlaying) AutoPlay::ClearState();
                else AutoPlay::currentMode = AutoPlay::MODE_OFF;
            }
        }

        GLuint tex;
        if (cancelMode)                       tex = queue_cancel_tex;
        else if (AutoPlay::bAutoPlaying)      tex = play_on_tex;
        else                                  tex = play_off_tex;

        ImDrawList* dl = GetWindowDrawList();
        dl->AddImage((void*)(intptr_t)tex,
                     ImVec2(pos.x, pos.y),
                     ImVec2(pos.x + BTN_W, pos.y + BTN_H),
                     ImVec2(0,0), ImVec2(1,1));
    }
    End();
    PopStyleVar(2);
    PopStyleColor(2);
}

// ــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــ //

static void DrawFloatingButton(ImGuiIO& io) {
    if (g_menu.isOpen) return;

    static GLuint logo_tex = LoadTextureFromMemory(logo_png, logo_png_len);
    
    float buttonRadius = 65.0f;
    float winSize = (buttonRadius * 2.0f) + 10.0f;
    const float rightMargin = 20.0f;

    // القيمة الافتراضية للزر السفلي
    if (g_sideBtnsY <= 0.0f) g_sideBtnsY = io.DisplaySize.y - 150.0f;

    float toggleWidth = 130.f + (GetStyle().WindowPadding.x * 2.0f);
    float fixedX = io.DisplaySize.x - rightMargin - toggleWidth + (toggleWidth - winSize) * 0.5f;
    
    // الأيقونة دائماً فوق الزر بمسافة 140 بكسل بالضبط
    float posY = g_sideBtnsY - 140.0f; 

    SetNextWindowPos(ImVec2(fixedX, posY), ImGuiCond_Always);
    SetNextWindowSize(ImVec2(winSize, winSize), ImGuiCond_Always);
    
    PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    if (Begin(O("##FloatBtn"), nullptr,
              ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar |
              ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {

        ImDrawList* dl = GetWindowDrawList();
        ImVec2 center = ImVec2(fixedX + (winSize * 0.5f), posY + (winSize * 0.5f));

        InvisibleButton(O("##FloatBtnHit"), ImVec2(winSize, winSize));

        // منطق السحب الذكي: يحرك القيمة g_sideBtnsY فقط
        if (IsItemActive() && IsMouseDragging(ImGuiMouseButton_Left)) {
            g_sideBtnsY += io.MouseDelta.y;
            // حصر الحركة لضمان عدم خروج المجموعة من الشاشة
            g_sideBtnsY = ImClamp(g_sideBtnsY, 160.0f, io.DisplaySize.y - 150.0f);
        }

        // منطق الضغط (فتح المنيو)
        if (IsItemHovered() && IsMouseReleased(0) && ImGui::GetMouseDragDelta(0).y == 0) {
            g_menu.isOpen = true;
        }

        dl->AddImage((void*)(intptr_t)logo_tex,
                     ImVec2(center.x - buttonRadius, center.y - buttonRadius),
                     ImVec2(center.x + buttonRadius, center.y + buttonRadius));
    }
    End();
    PopStyleVar(2);
    PopStyleColor();
}

// ــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــ //

static bool first_time = true;
INLINE void DrawLogin(ImGuiIO& io) {
    if (logged_in) return DrawMenu(io);

    SetNextWindowPos(ImVec2(0, 0));
    SetNextWindowSize(io.DisplaySize);
    PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.04f, 0.04f, 0.06f, 0.96f));
    Begin(O("##Overlay"), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBringToFrontOnFocus);
    PopStyleColor();

    float cardW = 580;
    float cardH = 420;

    SetNextWindowSize(ImVec2(cardW, cardH), ImGuiCond_Always);
    SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    PushStyleColor(ImGuiCol_WindowBg,
    ImVec4(0.18f, 0.12f, 0.25f, 1.0f));
    PushStyleVar(ImGuiStyleVar_WindowRounding, 35.0f);
    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    Begin(O("##LoginCard"), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);

    ImDrawList* dl = GetWindowDrawList();
    ImVec2 winPos = GetWindowPos();
    
    DrawGradientRect(dl, winPos, ImVec2(winPos.x + cardW, winPos.y + 110), Theme::CrimsonDark, Theme::Crimson, true);
    dl->AddRectFilled(winPos, ImVec2(winPos.x + cardW, winPos.y + 20), Theme::CrimsonDark, 20.0f, ImDrawFlags_RoundCornersTop);

    SetWindowFontScale(1.4f);
    ImVec2 titleSize = CalcTextSize(O("TIGER"));
    dl->AddText(ImVec2(winPos.x + (cardW - titleSize.x) * 0.5f, winPos.y + 30), IM_COL32(255, 255, 255, 255), O("TIGER"));
    SetWindowFontScale(1.0f);
    
    ImVec2 subSize = CalcTextSize(O("Premium Mod"));
    dl->AddText(ImVec2(winPos.x + (cardW - subSize.x) * 0.5f, winPos.y + 70), IM_COL32(255, 210, 215, 200), O("Premium Mod"));

    SetCursorPosY(130);

    if (!ERROR_MESSAGE.empty()) {
        SetCursorPosX(30);
        PushTextWrapPos(cardW - 30);
        TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", ERROR_MESSAGE.c_str());
        PopTextWrapPos();
        Dummy(ImVec2(0, 15));
    }

    if (is_logging_in) {
        SetCursorPosY(180);
        
        static float spinner_angle = 0.0f;
        spinner_angle += io.DeltaTime * 5.0f;

        float spinner_size = 40.0f;
        ImVec2 spinnerCenter = ImVec2(winPos.x + cardW * 0.5f, winPos.y + 220);

        for (int i = 0; i < 12; i++) {
            float angle = spinner_angle + (i * PI * 2.0f / 12.0f);
            float alpha = (float)(12 - i) / 12.0f;
            ImVec2 dotPos = ImVec2(
                spinnerCenter.x + cosf(angle) * spinner_size,
                spinnerCenter.y + sinf(angle) * spinner_size
            );
            dl->AddCircleFilled(dotPos, 6.0f, IM_COL32(255, 90, 100, (int)(alpha * 255)));
        }

        ImVec2 loadingSize = CalcTextSize(O("Authenticating..."));
        SetCursorPosX((cardW - loadingSize.x) * 0.5f);
        SetCursorPosY(290);
        TextColored(ImVec4(0.6f, 0.6f, 0.65f, 1.0f), O("Authenticating..."));
    } else {
        SetCursorPosY(150);

        ImVec2 infoSize = CalcTextSize(O("Paste your license key below"));
        SetCursorPosX((cardW - infoSize.x) * 0.5f);
        TextColored(ImVec4(0.85f, 0.65f, 0.67f, 1.0f), O("Paste your license key below"));

        Dummy(ImVec2(0, 18));

        bool AutoLogin = first_time && !persistent_string["key"].empty();

        // ── License key field + paste button ──
        static char s_keyBuf[256] = {0};
        if (s_keyBuf[0] == '\0' && !persistent_string["key"].empty()) {
            strncpy(s_keyBuf, persistent_string["key"].c_str(), sizeof(s_keyBuf) - 1);
        }

        float pasteBtnW = 60.0f;
        float fieldGap  = 10.0f;

        SetCursorPosX(40);
        PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.10f, 0.02f, 0.04f, 1.0f));
        PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.14f, 0.03f, 0.06f, 1.0f));
        PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.90f, 0.90f, 1.0f));
        PushStyleVar(ImGuiStyleVar_FrameRounding, 14.0f);
        PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(14.0f, 14.0f));
        SetNextItemWidth(cardW - 80 - pasteBtnW - fieldGap);
        InputTextWithHint(O("##LicenseKey"), O("License key"), s_keyBuf, sizeof(s_keyBuf));
        PopStyleVar(2);
        PopStyleColor(3);

        SameLine(0, fieldGap);
        PushStyleColor(ImGuiCol_Button, ImVec4(0.55f, 0.05f, 0.10f, 1.0f));
        PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.68f, 0.08f, 0.14f, 1.0f));
        PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.45f, 0.03f, 0.08f, 1.0f));
        PushStyleVar(ImGuiStyleVar_FrameRounding, 14.0f);
        if (Button(O("PASTE##KeyPaste"), ImVec2(pasteBtnW, 45.0f))) {
            JNIEnv* env;
            jint pasteEnvResult = VM->GetEnv((void**)&env, JNI_VERSION_1_6);
            if (pasteEnvResult == JNI_EDETACHED) VM->AttachCurrentThread(&env, nullptr);
            if (env) {
                std::string clip = getClipboard(env);
                strncpy(s_keyBuf, clip.c_str(), sizeof(s_keyBuf) - 1);
                s_keyBuf[sizeof(s_keyBuf) - 1] = '\0';
            }
        }
        PopStyleVar();
        PopStyleColor(3);

        Dummy(ImVec2(0, 22));

        SetCursorPosX(40);
        PushStyleColor(ImGuiCol_Button, ImVec4(0.55f, 0.05f, 0.10f, 1.0f));
        PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.68f, 0.08f, 0.14f, 1.0f));
        PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.45f, 0.03f, 0.08f, 1.0f));
        PushStyleVar(ImGuiStyleVar_FrameRounding, 14.0f);

        if (AutoLogin || Button("LGNBTN", ImVec2(cardW - 80, 65))) {
            if (DEBUG_BYPASS_LOGIN) {
                // Debug bypass: open menu immediately
                logged_in = true;
                g_menu.isOpen = true;
            } else {
                std::string keyToUse = AutoLogin ? persistent_string["key"] : std::string(s_keyBuf);
                JNIEnv* env;
                jint getEnvResult = VM->GetEnv((void**)&env, JNI_VERSION_1_6);
                if (getEnvResult == JNI_EDETACHED) {
                    if (VM->AttachCurrentThread(&env, nullptr) != 0) ERROR_MESSAGE = O("Failed to attach thread to JVM");
                } else if (getEnvResult != JNI_OK) {
                    ERROR_MESSAGE = O("Failed to get JNIEnv");
                } else {
                    // Fall back to clipboard only if the field was left empty.
                    // keylogin.h ships its own async wrapper (LoginAsync) that
                    // handles the background thread itself -- use it directly
                    // instead of spawning our own thread around Login().
                    if (keyToUse.empty()) keyToUse = getClipboard(env);
                    persistent_string["key"] = keyToUse;
                    save_persistence();
                    LoginAsync(getAndroidID(env), keyToUse);
                }
                first_time = false;
            }
        }

        PopStyleVar();
        PopStyleColor(3);
    }

    End();
    PopStyleVar(3);
    PopStyleColor();
    
    End();
}


INLINE void SetupImgui() {
    PACKAGE_NAME = string(getcmdline());

    ImGui::CreateContext();

    auto& style = ImGui::GetStyle();
    auto& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;

    switch_theme(current_theme);

    load_persistence();
    svConfig_Load();
    load_imgui_style();

    static string INI_PATH = O("/data/user_de/0/") + PACKAGE_NAME + O("/no_backup/.ini");
    io.IniFilename = persistent_bool["bImguiAutoSave"] ? INI_PATH.c_str() : nullptr;
    io.ConfigWindowsMoveFromTitleBarOnly = persistent_bool["bMoveOnlyWithTitleBar"];

    ImFontConfig font_cfg;
    font_cfg.SizePixels = persistent_float["fFontScale"];
    io.Fonts->AddFontDefault(&font_cfg);

    ImGui_ImplAndroid_Init();
    ImGui_ImplOpenGL3_Init(O("#version 300 es"));

    bImguiSetup = true;
}

DEFINES(EGLBoolean, Draw, EGLDisplay dpy, EGLSurface surface) {
    eglQuerySurface(dpy, surface, EGL_WIDTH, &Width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &Height);

    if (Width <= 0 || Height <= 0) return _Draw(dpy, surface);

    screenCenter = Vector2(Width / 2, Height / 2);

    if (!bImguiSetup) SetupImgui();

    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame(Width, Height);
    ImGui::NewFrame();

    if (!is_segv_handler_active()) setup_global_segv_handler();

    // ── TIGER: one-shot initialisation ──────────────────────────────────────
    ReadMenuToggle();
    TigerAutoLogin();

    // ── Menu toggle: if loader says off, skip ALL rendering ─────────────────
    if (!g_menu_enabled.load()) {
        ImGui::EndFrame();
        return _Draw(dpy, surface);
    }

    if (IsExpired()) {
        DrawExpired(io);
    } else if (logged_in || (g_ExpiryTime.load() > 0) || DEBUG_BYPASS_LOGIN) {
        DrawFloatingButton(io);
        DrawMenu(io);

{
    SetNextWindowPos(ImVec2(Width * 0.5f, Height - 60.0f), ImGuiCond_Always, ImVec2(0.5f, 1.0f));
    
    // Fereastră fără fundal, fără margini, care se redimensionează singură
    Begin(O("##PoweredBy"), nullptr, 
          ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
          ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | 
          ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize | 
          ImGuiWindowFlags_NoInputs);
    
    TextColored(ImColor(180, 0, 0, 255), O("Telegram @AKOJO"));
    
    End();
}

        if (g_autoPlayCalculating) DrawCalculating(io);
    }
    // TIGER: No login screen inside game — loader handles all auth.
    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    ImGui_ClearHoverEffect();

    return _Draw(dpy, surface);
}

void __IMGUI__() {
    create_directory_recursive(CONC(O("/data/user_de/0/"), PACKAGE_NAME.c_str(), O("/no_backup")));
}
