#pragma once
#include "imgui.h"

namespace Nexus {

class ModernTheme {
public:
    // Color schemes
    enum class ColorScheme {
        Dark,
        Light,
        CyberPunk,
        Nord,
        Dracula,
        Monokai
    };

    static void ApplyTheme(ColorScheme scheme = ColorScheme::Dark) {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // Modern rounded corners
        style.WindowRounding = 8.0f;
        style.FrameRounding = 6.0f;
        style.PopupRounding = 6.0f;
        style.ScrollbarRounding = 8.0f;
        style.GrabRounding = 6.0f;
        style.TabRounding = 6.0f;
        style.ChildRounding = 6.0f;

        // Modern padding and spacing
        style.WindowPadding = ImVec2(12, 12);
        style.FramePadding = ImVec2(10, 6);
        style.ItemSpacing = ImVec2(8, 6);
        style.ItemInnerSpacing = ImVec2(8, 6);
        style.IndentSpacing = 20.0f;
        style.ScrollbarSize = 14.0f;
        style.GrabMinSize = 12.0f;

        // Modern borders
        style.WindowBorderSize = 1.0f;
        style.ChildBorderSize = 1.0f;
        style.PopupBorderSize = 1.0f;
        style.FrameBorderSize = 0.0f;

        switch (scheme) {
            case ColorScheme::Dark:
                ApplyDarkTheme(colors);
                break;
            case ColorScheme::CyberPunk:
                ApplyCyberPunkTheme(colors);
                break;
            case ColorScheme::Nord:
                ApplyNordTheme(colors);
                break;
            case ColorScheme::Dracula:
                ApplyDraculaTheme(colors);
                break;
            case ColorScheme::Monokai:
                ApplyMonokaiTheme(colors);
                break;
            default:
                ApplyDarkTheme(colors);
                break;
        }
    }

private:
    static void ApplyDarkTheme(ImVec4* colors) {
        colors[ImGuiCol_Text]                   = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
        colors[ImGuiCol_TextDisabled]           = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
        colors[ImGuiCol_WindowBg]               = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
        colors[ImGuiCol_ChildBg]                = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
        colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        colors[ImGuiCol_Border]                 = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
        colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg]                = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
        colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
        colors[ImGuiCol_FrameBgActive]          = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
        colors[ImGuiCol_TitleBg]                = ImVec4(0.09f, 0.12f, 0.14f, 0.65f);
        colors[ImGuiCol_TitleBgActive]          = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
        colors[ImGuiCol_MenuBarBg]              = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
        colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
        colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.09f, 0.21f, 0.31f, 1.00f);
        colors[ImGuiCol_CheckMark]              = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
        colors[ImGuiCol_SliderGrab]             = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
        colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
        colors[ImGuiCol_Button]                 = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
        colors[ImGuiCol_ButtonHovered]          = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
        colors[ImGuiCol_ButtonActive]           = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
        colors[ImGuiCol_Header]                 = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
        colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_Separator]              = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
        colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
        colors[ImGuiCol_SeparatorActive]        = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
        colors[ImGuiCol_ResizeGrip]             = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
        colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        colors[ImGuiCol_Tab]                    = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
        colors[ImGuiCol_TabHovered]             = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        colors[ImGuiCol_TabActive]              = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
        colors[ImGuiCol_TabUnfocused]           = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
        colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    }

    static void ApplyCyberPunkTheme(ImVec4* colors) {
        colors[ImGuiCol_Text]                   = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_TextDisabled]           = ImVec4(0.00f, 0.40f, 0.41f, 1.00f);
        colors[ImGuiCol_WindowBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.94f);
        colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_PopupBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.94f);
        colors[ImGuiCol_Border]                 = ImVec4(0.00f, 1.00f, 1.00f, 0.65f);
        colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg]                = ImVec4(0.44f, 0.80f, 0.80f, 0.18f);
        colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.44f, 0.80f, 0.80f, 0.27f);
        colors[ImGuiCol_FrameBgActive]          = ImVec4(0.44f, 0.81f, 0.86f, 0.66f);
        colors[ImGuiCol_TitleBg]                = ImVec4(0.14f, 0.18f, 0.21f, 0.73f);
        colors[ImGuiCol_TitleBgActive]          = ImVec4(0.00f, 1.00f, 1.00f, 0.27f);
        colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.54f);
        colors[ImGuiCol_MenuBarBg]              = ImVec4(0.00f, 0.00f, 0.00f, 0.20f);
        colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.22f, 0.29f, 0.30f, 0.71f);
        colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.00f, 1.00f, 1.00f, 0.44f);
        colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.00f, 1.00f, 1.00f, 0.74f);
        colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_CheckMark]              = ImVec4(0.00f, 1.00f, 1.00f, 0.68f);
        colors[ImGuiCol_SliderGrab]             = ImVec4(0.00f, 1.00f, 1.00f, 0.36f);
        colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.00f, 1.00f, 1.00f, 0.76f);
        colors[ImGuiCol_Button]                 = ImVec4(0.00f, 0.65f, 0.65f, 0.46f);
        colors[ImGuiCol_ButtonHovered]          = ImVec4(0.01f, 1.00f, 1.00f, 0.43f);
        colors[ImGuiCol_ButtonActive]           = ImVec4(0.00f, 1.00f, 1.00f, 0.62f);
        colors[ImGuiCol_Header]                 = ImVec4(0.00f, 1.00f, 1.00f, 0.33f);
        colors[ImGuiCol_HeaderHovered]          = ImVec4(0.00f, 1.00f, 1.00f, 0.42f);
        colors[ImGuiCol_HeaderActive]           = ImVec4(0.00f, 1.00f, 1.00f, 0.54f);
        colors[ImGuiCol_ResizeGrip]             = ImVec4(0.00f, 1.00f, 1.00f, 0.54f);
        colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.00f, 1.00f, 1.00f, 0.74f);
        colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_PlotLines]              = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered]       = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_PlotHistogram]          = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.00f, 1.00f, 1.00f, 0.22f);
    }

    static void ApplyNordTheme(ImVec4* colors) {
        colors[ImGuiCol_Text]                   = ImVec4(0.85f, 0.87f, 0.91f, 1.00f);
        colors[ImGuiCol_WindowBg]               = ImVec4(0.18f, 0.20f, 0.25f, 1.00f);
        colors[ImGuiCol_ChildBg]                = ImVec4(0.16f, 0.17f, 0.21f, 1.00f);
        colors[ImGuiCol_Border]                 = ImVec4(0.27f, 0.31f, 0.38f, 1.00f);
        colors[ImGuiCol_FrameBg]                = ImVec4(0.26f, 0.30f, 0.37f, 1.00f);
        colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.33f, 0.38f, 0.46f, 1.00f);
        colors[ImGuiCol_TitleBg]                = ImVec4(0.16f, 0.17f, 0.21f, 1.00f);
        colors[ImGuiCol_TitleBgActive]          = ImVec4(0.18f, 0.20f, 0.25f, 1.00f);
        colors[ImGuiCol_Button]                 = ImVec4(0.35f, 0.47f, 0.65f, 1.00f);
        colors[ImGuiCol_ButtonHovered]          = ImVec4(0.51f, 0.63f, 0.76f, 1.00f);
        colors[ImGuiCol_ButtonActive]           = ImVec4(0.56f, 0.74f, 0.73f, 1.00f);
        colors[ImGuiCol_Header]                 = ImVec4(0.35f, 0.47f, 0.65f, 0.76f);
        colors[ImGuiCol_HeaderHovered]          = ImVec4(0.51f, 0.63f, 0.76f, 0.86f);
        colors[ImGuiCol_HeaderActive]           = ImVec4(0.56f, 0.74f, 0.73f, 1.00f);
    }

    static void ApplyDraculaTheme(ImVec4* colors) {
        colors[ImGuiCol_Text]                   = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
        colors[ImGuiCol_WindowBg]               = ImVec4(0.16f, 0.16f, 0.21f, 1.00f);
        colors[ImGuiCol_ChildBg]                = ImVec4(0.18f, 0.18f, 0.23f, 1.00f);
        colors[ImGuiCol_Border]                 = ImVec4(0.27f, 0.28f, 0.35f, 1.00f);
        colors[ImGuiCol_Button]                 = ImVec4(0.74f, 0.58f, 0.98f, 1.00f);
        colors[ImGuiCol_ButtonHovered]          = ImVec4(0.80f, 0.70f, 1.00f, 1.00f);
        colors[ImGuiCol_ButtonActive]           = ImVec4(0.68f, 0.50f, 0.90f, 1.00f);
        colors[ImGuiCol_Header]                 = ImVec4(0.74f, 0.58f, 0.98f, 0.76f);
    }

    static void ApplyMonokaiTheme(ImVec4* colors) {
        colors[ImGuiCol_Text]                   = ImVec4(0.97f, 0.97f, 0.95f, 1.00f);
        colors[ImGuiCol_WindowBg]               = ImVec4(0.16f, 0.16f, 0.14f, 1.00f);
        colors[ImGuiCol_Button]                 = ImVec4(0.98f, 0.15f, 0.45f, 1.00f);
        colors[ImGuiCol_ButtonHovered]          = ImVec4(1.00f, 0.35f, 0.60f, 1.00f);
    }
};

} // namespace Nexus
