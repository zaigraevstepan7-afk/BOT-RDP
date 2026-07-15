#include <map>
#include "imgui.h"
#include <cmath>  // Для lroundf (или lround)

void TextCentered(const char* text, ImColor textColor = ImColor(255, 255, 255), double yOffset = 0.0)
{
    float availableWidth = ImGui::GetContentRegionAvail().x;
    float textWidth = ImGui::CalcTextSize(text).x;
    float xOffset = (availableWidth - textWidth) * 0.5f;
    if (xOffset < 0.0f)
    {
        xOffset = 0.0f;
    }
    ImVec2 cursorPos = ImGui::GetCursorPos();
    ImGui::SetCursorPos(ImVec2(cursorPos.x + xOffset, cursorPos.y + yOffset));
    ImGui::TextColored(textColor, text);
}

//static float accentColor[3] = { 0.0f, 1.0f, 0.0f };

namespace menuConfig {
    inline ImVec4 accent = ImColor(0, 255, 0);

    namespace background {
        ImVec2 size = ImVec2(1000, 660);
    }
}

static void SectionHeader(const char* text)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return;

    ImDrawList* draw = window->DrawList;

    ImVec2 pos = window->DC.CursorPos;
    float width = ImGui::GetContentRegionAvail().x;

    ImVec2 text_size = ImGui::CalcTextSize(text);

    const float pad_x = 18.0f;
    const float pad_y = 9.0f;
    float height = text_size.y + pad_y * 2.0f;

    ImRect bb(pos, ImVec2(pos.x + width, pos.y + height));
    ImGui::ItemSize(bb);
    if (!ImGui::ItemAdd(bb, 0)) return;

    // ===== ФОН (чуть темнее, ровный)
    draw->AddRectFilled(
        bb.Min,
        bb.Max,
        ImGui::GetColorU32(ImVec4(0.12f, 0.13f, 0.15f, 1.0f)),
        0.0f
    );

    ImVec4 accent = menuConfig::accent;
    ImVec2 text_pos(bb.Min.x + pad_x, bb.Min.y + pad_y);

    // ===== ОЧЕНЬ МЯГКИЙ АККУРАТНЫЙ GLOW
    for (int i = 1; i <= 5; i++)
    {
        float alpha = 0.035f * (1.0f - i / 5.0f);
        float radius = i * 0.6f;

        draw->AddText(
            ImVec2(text_pos.x - radius, text_pos.y),
            ImGui::GetColorU32(ImVec4(accent.x, accent.y, accent.z, alpha)),
            text
        );
        draw->AddText(
            ImVec2(text_pos.x + radius, text_pos.y),
            ImGui::GetColorU32(ImVec4(accent.x, accent.y, accent.z, alpha)),
            text
        );
        draw->AddText(
            ImVec2(text_pos.x, text_pos.y - radius),
            ImGui::GetColorU32(ImVec4(accent.x, accent.y, accent.z, alpha)),
            text
        );
        draw->AddText(
            ImVec2(text_pos.x, text_pos.y + radius),
            ImGui::GetColorU32(ImVec4(accent.x, accent.y, accent.z, alpha)),
            text
        );
    }

    // ===== ПЕРЕЛИВАЮЩИЙСЯ ГРАДИЕНТ ПО БУКВАМ
    float advance = 0.0f;
    const char* p = text;

    while (*p)
    {
        char c[2] = { *p, 0 };
        ImVec2 cs = ImGui::CalcTextSize(c);

        float t = advance / ImMax(text_size.x, 1.0f);

        ImVec4 col;
        col.x = ImLerp(accent.x * 0.75f, ImMin(accent.x + 0.25f, 1.0f), t);
        col.y = ImLerp(accent.y * 0.75f, ImMin(accent.y + 0.25f, 1.0f), t);
        col.z = ImLerp(accent.z * 0.75f, ImMin(accent.z + 0.25f, 1.0f), t);
        col.w = 1.0f;

        draw->AddText(
            ImVec2(text_pos.x + advance, text_pos.y),
            ImGui::GetColorU32(col),
            c
        );

        advance += cs.x;
        p++;
    }

    ImGui::Dummy(ImVec2(0.0f, 12.0f));
}

namespace ImGui
{
    // Структура для анимации кнопки
    struct ButtonAnimationData
    {
        float clickAnim = 0.0f;
        float hoverAnim = 0.0f;
    };

    static std::unordered_map<ImGuiID, ButtonAnimationData> buttonAnimCache;

    bool CustomButton(const char* label, const ImVec2& size_arg = ImVec2(0, 0), const char* description = nullptr)
    {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems) return false;

        ImGuiContext& g = *GImGui;
        const ImGuiID id = window->GetID(label);
        
        // Акцентный цвет
        ImVec4 accentColor = menuConfig::accent;

        // Масштаб текста
        const float label_scale = 0.92f;
        const float desc_scale = 0.85f;

        ImVec2 normal_label_size = CalcTextSize(label, nullptr, true);
        ImVec2 normal_desc_size = (description && description[0]) ? CalcTextSize(description, nullptr, true) : ImVec2(0, 0);
        
        ImVec2 scaled_label_size = ImVec2(normal_label_size.x * label_scale, normal_label_size.y * label_scale);
        ImVec2 scaled_desc_size = ImVec2(normal_desc_size.x * desc_scale, normal_desc_size.y * desc_scale);

        // Общая высота текста
        float scaled_gap = 6.0f * label_scale;
        float total_scaled_text_height = scaled_label_size.y + (normal_desc_size.y > 0 ? scaled_desc_size.y + scaled_gap : 0.0f);

        // Высота кнопки
        float total_height = 64.0f;

        ImVec2 pos = window->DC.CursorPos;

        // Ширина
        float available_width = ImGui::GetContentRegionAvail().x;
        float button_width = (size_arg.x <= 0.0f) ? available_width : size_arg.x;
        ImVec2 size(button_width, total_height);

        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        
        ItemSize(bb, 0.0f);
        if (!ItemAdd(bb, id)) return false;

        bool hovered, held;
        bool pressed = ButtonBehavior(bb, id, &hovered, &held);
        
        // Анимации
        ButtonAnimationData& anim = buttonAnimCache[id];
        float delta_time = g.IO.DeltaTime;
        
        float hover_target = hovered ? 1.0f : 0.0f;
        anim.hoverAnim = ImClamp(anim.hoverAnim + (hover_target - anim.hoverAnim) * delta_time * 3.0f, 0.0f, 1.0f);
        
        if (held || pressed) {
            anim.clickAnim = 1.0f;
        } else {
            anim.clickAnim = ImClamp(anim.clickAnim - delta_time * 8.0f, 0.0f, 1.0f);
        }

        // Рендеринг
        ImDrawList* draw = window->DrawList;
        const float rounding = 4.0f;
        
        // Полупрозрачный фон (10% видимости)
        ImVec4 original_bg_color = ImVec4(0.09f, 0.09f, 0.11f, 0.1f);
        // Полупрозрачная обводка
        ImVec4 original_border_color = ImVec4(0.22f, 0.22f, 0.24f, 0.4f);
        
        ImVec4 bg_color = original_bg_color;
        
        if (anim.clickAnim > 0.0f) {
            // Акцентный цвет с низкой прозрачностью
            ImVec4 click_bg_color = ImVec4(accentColor.x * 0.15f, accentColor.y * 0.15f, accentColor.z * 0.15f, 0.2f);
            bg_color = ImLerp(bg_color, click_bg_color, anim.clickAnim);
        }
        else if (anim.hoverAnim > 0.0f) {
            // Легкое затемнение при наведении
            ImVec4 hover_bg_color = ImVec4(0.11f, 0.11f, 0.13f, 0.15f);
            bg_color = ImLerp(bg_color, hover_bg_color, anim.hoverAnim);
        }
        
        ImVec4 border_color = original_border_color;
        
        if (held) {
            // Полупрозрачный акцентный цвет при удержании
            border_color = ImVec4(accentColor.x, accentColor.y, accentColor.z, 0.6f);
        }
        else if (anim.clickAnim > 0.0f) {
            float t = anim.clickAnim;
            float fast_return = t * t;
            ImVec4 accent_border = ImVec4(accentColor.x, accentColor.y, accentColor.z, 0.4f);
            border_color = ImLerp(accent_border, original_border_color, 1.0f - fast_return);
        }
        else if (hovered) {
            border_color = ImVec4(0.25f, 0.25f, 0.27f, 0.5f);
        }

        // Рисуем фон
        draw->AddRectFilled(bb.Min, bb.Max, GetColorU32(bg_color), rounding);
        
        // Тонкая обводка (1.0f вместо 1.8f)
        float border_thickness = 1.0f;
        if (held) border_thickness = 1.2f;
        else if (hovered) border_thickness = 1.1f;
        
        draw->AddRect(bb.Min, bb.Max, GetColorU32(border_color), rounding, 0, border_thickness);

        // Точное центрирование
        float text_block_y = bb.Min.y + (total_height - total_scaled_text_height) * 0.5f;

        // Основной текст
        float text_x = bb.Min.x + (button_width - scaled_label_size.x) * 0.5f;
        float text_y = text_block_y;
        
        // Полупрозрачный текст
        ImU32 text_col = GetColorU32(ImVec4(0.90f, 0.90f, 0.90f, 0.9f));
        draw->AddText(g.Font, g.FontSize * label_scale, ImVec2(text_x, text_y), text_col, label);

        // Описание
        if (description && description[0] != '\0')
        {
            float desc_x = bb.Min.x + (button_width - scaled_desc_size.x) * 0.5f;
            float desc_y = text_y + scaled_label_size.y + scaled_gap;
            
            // Более прозрачное описание
            ImU32 desc_col = GetColorU32(ImVec4(0.70f, 0.70f, 0.70f, 0.7f));
            draw->AddText(g.Font, g.FontSize * desc_scale, ImVec2(desc_x, desc_y), desc_col, description);
        }

        Dummy(ImVec2(0.0f, 12.0f));

        return pressed;
    }

    bool CustomButtonEx(const char* label, const ImVec2& size = ImVec2(0, 0))
    {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems) return false;

        ImGuiContext& g = *GImGui;
        const ImGuiID id = window->GetID(label);
        
        ImVec4 accentColor = menuConfig::accent;

        const float text_scale = 0.92f;

        ImVec2 normal_label_size = CalcTextSize(label, nullptr, true);
        ImVec2 scaled_label_size = ImVec2(normal_label_size.x * text_scale, normal_label_size.y * text_scale);
        
        float total_height = 52.0f;

        ImVec2 pos = window->DC.CursorPos;
        
        float available_width = ImGui::GetContentRegionAvail().x;
        float button_width = (size.x <= 0.0f) ? available_width : size.x;
        ImVec2 actual_size(button_width, total_height);
        
        const ImRect bb(pos, ImVec2(pos.x + actual_size.x, pos.y + actual_size.y));
        
        ItemSize(bb, 0.0f);
        if (!ItemAdd(bb, id)) return false;

        bool hovered, held;
        bool pressed = ButtonBehavior(bb, id, &hovered, &held);
        
        ButtonAnimationData& anim = buttonAnimCache[id];
        float delta_time = g.IO.DeltaTime;
        
        float hover_target = hovered ? 1.0f : 0.0f;
        anim.hoverAnim = ImClamp(anim.hoverAnim + (hover_target - anim.hoverAnim) * delta_time * 3.0f, 0.0f, 1.0f);
        
        if (held || pressed) {
            anim.clickAnim = 1.0f;
        } else {
            anim.clickAnim = ImClamp(anim.clickAnim - delta_time * 10.0f, 0.0f, 1.0f);
        }

        ImDrawList* draw = window->DrawList;
        const float rounding = 4.0f;
        
        // Полупрозрачные цвета
        ImVec4 original_bg = ImVec4(0.09f, 0.09f, 0.11f, 0.1f);
        ImVec4 original_border = ImVec4(0.22f, 0.22f, 0.24f, 0.4f);
        
        ImVec4 bg_color = original_bg;
        if (anim.clickAnim > 0.0f) {
            ImVec4 click_bg = ImVec4(accentColor.x * 0.15f, accentColor.y * 0.15f, accentColor.z * 0.15f, 0.2f);
            bg_color = ImLerp(bg_color, click_bg, anim.clickAnim);
        } else if (anim.hoverAnim > 0.0f) {
            ImVec4 hover_bg = ImVec4(0.11f, 0.11f, 0.13f, 0.15f);
            bg_color = ImLerp(bg_color, hover_bg, anim.hoverAnim);
        }
        
        ImVec4 border_color = original_border;
        
        if (held) {
            border_color = ImVec4(accentColor.x, accentColor.y, accentColor.z, 0.6f);
        } else if (anim.clickAnim > 0.0f) {
            float t = anim.clickAnim;
            float fast_t = t * t;
            ImVec4 accent_border = ImVec4(accentColor.x, accentColor.y, accentColor.z, 0.4f);
            border_color = ImLerp(accent_border, original_border, 1.0f - fast_t);
        } else if (hovered) {
            border_color = ImVec4(0.24f, 0.24f, 0.26f, 0.5f);
        }
        
        // Фон
        draw->AddRectFilled(bb.Min, bb.Max, GetColorU32(bg_color), rounding);
        
        // Тонкая обводка
        float border_thickness = 1.0f;
        if (held) border_thickness = 1.1f;
        else if (hovered) border_thickness = 1.05f;
        
        draw->AddRect(bb.Min, bb.Max, GetColorU32(border_color), rounding, 0, border_thickness);
        
        // Текст
        float text_x = bb.Min.x + (button_width - scaled_label_size.x) * 0.5f;
        float text_y = bb.Min.y + (total_height - scaled_label_size.y) * 0.5f;
        
        ImU32 text_col = GetColorU32(ImVec4(0.90f, 0.90f, 0.90f, 0.9f));
        draw->AddText(g.Font, g.FontSize * text_scale, ImVec2(text_x, text_y), text_col, label);
        
        Dummy(ImVec2(0.0f, 10.0f));

        return pressed;
    }

    bool CustomButtonSmall(const char* label, const ImVec2& size = ImVec2(0, 0))
    {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems) return false;

        ImGuiContext& g = *GImGui;
        const ImGuiID id = window->GetID(label);
        
        ImVec4 accentColor = menuConfig::accent;

        const float text_scale = 0.90f;

        ImVec2 normal_label_size = CalcTextSize(label, nullptr, true);
        ImVec2 scaled_label_size = ImVec2(normal_label_size.x * text_scale, normal_label_size.y * text_scale);
        
        float total_height = 42.0f;

        ImVec2 pos = window->DC.CursorPos;
        
        float available_width = ImGui::GetContentRegionAvail().x;
        float button_width = (size.x <= 0.0f) ? available_width : size.x;
        
        const ImRect bb(pos, ImVec2(pos.x + button_width, pos.y + total_height));
        
        ItemSize(bb, 0.0f);
        if (!ItemAdd(bb, id)) return false;

        bool hovered, held;
        bool pressed = ButtonBehavior(bb, id, &hovered, &held);

        ImDrawList* draw = window->DrawList;
        const float rounding = 4.0f;
        
        // Полупрозрачные цвета
        ImVec4 original_bg = ImVec4(0.09f, 0.09f, 0.11f, 0.1f);
        ImVec4 original_border = ImVec4(0.22f, 0.22f, 0.24f, 0.4f);
        
        ImU32 box_bg = GetColorU32(original_bg);
        ImU32 box_border = GetColorU32(original_border);
        
        if (held) {
            box_border = GetColorU32(ImVec4(accentColor.x, accentColor.y, accentColor.z, 0.6f));
            box_bg = GetColorU32(ImVec4(accentColor.x * 0.15f, accentColor.y * 0.15f, accentColor.z * 0.15f, 0.2f));
        } else if (hovered) {
            box_border = GetColorU32(ImVec4(0.25f, 0.25f, 0.27f, 0.5f));
            box_bg = GetColorU32(ImVec4(0.11f, 0.11f, 0.13f, 0.15f));
        }
        
        // Фон
        draw->AddRectFilled(bb.Min, bb.Max, box_bg, rounding);
        // Тонкая обводка
        draw->AddRect(bb.Min, bb.Max, box_border, rounding, 0, 1.0f);
        
        // Текст
        float text_x = bb.Min.x + (button_width - scaled_label_size.x) * 0.5f;
        float text_y = bb.Min.y + (total_height - scaled_label_size.y) * 0.5f;
        
        ImU32 text_col = GetColorU32(ImVec4(0.90f, 0.90f, 0.90f, 0.9f));
        draw->AddText(g.Font, g.FontSize * text_scale, ImVec2(text_x, text_y), text_col, label);
        
        SameLine();
        Dummy(ImVec2(4.0f, 0.0f));

        return pressed;
    }
}

namespace ImGui
{
    bool CustomColorEdit3(const char* label, float col[3], const char* description = nullptr)
    {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems) return false;

        ImGuiContext& g = *GImGui;
        const ImGuiID id = window->GetID(label);

        ImVec2 label_size = CalcTextSize(label);
        ImVec2 desc_size = (description && description[0]) ? CalcTextSize(description) : ImVec2(0, 0);

        float line_h = GetTextLineHeight();
        float text_height = line_h + (desc_size.y > 0 ? line_h + 8.0f : 0.0f);
        
        // УВЕЛИЧИВАЕМ высоту элемента для отступов сверху и снизу
        float total_height = text_height + 40.0f + 8.0f; // +8 для отступа снизу

        const float rounding = 4.0f;
        const float preview_size = 44.0f;
        const float circle_radius = preview_size * 0.5f; // Радиус круга

        ImVec2 pos = window->DC.CursorPos;
        float width = window->WorkRect.GetWidth();

        ImRect bb(pos.x, pos.y, pos.x + width, pos.y + total_height);

        // Отступ до элемента
        ItemSize(bb, 12.0f); // Увеличил до 12
        if (!ItemAdd(bb, id)) return false;

        ImDrawList* draw = GetWindowDrawList();

        // ФОН С ПРОЗРАЧНОСТЬЮ И ОБВОДКОЙ КАК У СВИЧА
        ImRect content_bb = bb;
        content_bb.Max.y -= 8.0f; // Уменьшаем высоту для отступа
        
        // Прозрачный фон (10% видимости)
        draw->AddRectFilled(content_bb.Min, content_bb.Max, GetColorU32(ImVec4(0.09f, 0.09f, 0.11f, 0.1f)), rounding);
        // Тонкая полупрозрачная обводка
        draw->AddRect(content_bb.Min, content_bb.Max, GetColorU32(ImVec4(0.22f, 0.22f, 0.24f, 0.4f)), rounding, 0, 1.0f);

        // Текст с прозрачностью
        float text_y = content_bb.Min.y + 20.0f;
        // Основной текст
        PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.95f, 0.95f, 0.9f));
        RenderText(ImVec2(content_bb.Min.x + 26.0f, text_y), label);
        PopStyleColor();

        if (description && description[0] != '\0') {
            PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.70f, 0.70f, 0.7f));
            RenderText(ImVec2(content_bb.Min.x + 26.0f, text_y + line_h + 8.0f), description);
            PopStyleColor();
        }

        // Центр круга справа
        ImVec2 circle_center(content_bb.Max.x - 26.0f - circle_radius, content_bb.GetCenter().y);
        ImVec2 circle_min(circle_center.x - circle_radius, circle_center.y - circle_radius);
        ImVec2 circle_max(circle_center.x + circle_radius, circle_center.y + circle_radius);

        ImU32 color_u32 = ColorConvertFloat4ToU32(ImVec4(col[0], col[1], col[2], 1.0f));

        // Тень для круга
        draw->AddCircleFilled(ImVec2(circle_center.x + 0.5f, circle_center.y + 1.5f), 
                             circle_radius, 
                             GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 0.15f)));

        // Сам цвет КРУГЛЫЙ
        draw->AddCircleFilled(circle_center, circle_radius, color_u32);

        // Тонкая обводка круга
        draw->AddCircle(circle_center, circle_radius, GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 0.15f)), 0, 0.8f);

        // КРУГЛАЯ зона клика
        SetCursorScreenPos(circle_min);
        
        // СОЗДАЕМ УНИКАЛЬНЫЙ ID ДЛЯ КНОПКИ И ПОПАПА
        char button_id[64];
        char popup_id[64];
        snprintf(button_id, sizeof(button_id), "##color_preview_%d", id);
        snprintf(popup_id, sizeof(popup_id), "##color_picker_%d", id);
        
        // Квадратная кнопка для клика (проще, чем круг)
        InvisibleButton(button_id, ImVec2(preview_size, preview_size));

        bool changed = false;
        bool hovered = IsItemHovered();
        
        // Эффект при наведении на круг
        if (hovered) {
            ImU32 hover_color = GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.08f));
            draw->AddCircleFilled(circle_center, circle_radius, hover_color);
        }

        if (IsItemClicked()) {
            g.ColorEditCurrentID = id;
            OpenPopup(popup_id);
        }

        // Позиционируем попап СПРАВА (от круга) с отступом
        SetNextWindowPos(ImVec2(circle_max.x + 5.0f, circle_min.y), ImGuiCond_Appearing);
        
        if (BeginPopup(popup_id)) {
            // ФИКСИРУЕМ РАЗМЕР ОКНА чтобы оно не менялось
            const ImVec2 pickerSize(300.0f, 200.0f);
            
            // Устанавливаем фиксированный размер окна
            SetNextWindowSize(pickerSize, ImGuiCond_Always);
            
            // Также фиксируем минимальный и максимальный размер
            SetNextWindowSizeConstraints(pickerSize, pickerSize);
            
            // Сохраняем оригинальный стиль
            ImGuiStyle& style = GetStyle();
            ImVec2 originalWindowPadding = style.WindowPadding;
            ImVec2 originalFramePadding = style.FramePadding;
            ImVec2 originalItemSpacing = style.ItemSpacing;
            float originalWindowRounding = style.WindowRounding;
            
            // Устанавливаем компактный стиль (уменьшаем в 1.5-2 раза)
            style.WindowPadding = ImVec2(8.0f, 8.0f);
            style.FramePadding = ImVec2(4.0f, 2.0f);
            style.ItemSpacing = ImVec2(6.0f, 4.0f);
            style.WindowRounding = 6.0f;
            
            // Компактный цветовой пикер
            float col_with_alpha[4] = { col[0], col[1], col[2], 1.0f };
            
            // Флаги для компактного пикера
            ImGuiColorEditFlags flags = 
                ImGuiColorEditFlags_NoSidePreview | 
                ImGuiColorEditFlags_NoTooltip |
                ImGuiColorEditFlags_NoAlpha |
                ImGuiColorEditFlags_NoOptions |
                ImGuiColorEditFlags_NoLabel;
            
            // Устанавливаем ширину элементов
            PushItemWidth(pickerSize.x - 20.0f);
            
            // Используем компактный ColorPicker3
            if (ColorPicker3("##picker", col_with_alpha, flags))
            {
                changed = true;
                col[0] = col_with_alpha[0];
                col[1] = col_with_alpha[1];
                col[2] = col_with_alpha[2];
            }
            
            PopItemWidth();
            
            // Восстанавливаем стиль
            style.WindowPadding = originalWindowPadding;
            style.FramePadding = originalFramePadding;
            style.ItemSpacing = originalItemSpacing;
            style.WindowRounding = originalWindowRounding;
            
            EndPopup();
        }

        // УВЕЛИЧЕННЫЙ отступ после элемента
        Dummy(ImVec2(0.0f, 28.0f)); // Было 12.0f, теперь 28.0f

        return changed;
    }
}

namespace ImGui
{
bool CustomTab(bool selected, const char* icon, const char* label, float base_tab_width)
{
    float tab_width = base_tab_width * 2.4f;
    const float tab_height = 60.0f;
    const float rounding = 5.0f;

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext& g = *GImGui;
    ImGuiID id = window->GetID(label);

    ImVec2 pos = window->DC.CursorPos;
    ImRect bb(pos, ImVec2(pos.x + tab_width, pos.y + tab_height));

    SetCursorScreenPos(pos);
    InvisibleButton(label, ImVec2(tab_width, tab_height));
    bool pressed = IsItemClicked();

    static std::unordered_map<ImGuiID, float> anim;
    float& t = anim[id];
    t = ImLerp(t, selected ? 1.0f : 0.0f, g.IO.DeltaTime * 12.0f);

    ImDrawList* draw = window->DrawList;

    // === ФОН (ТОЛЬКО АКТИВНЫЙ)
    if (selected)
    {
        draw->AddRectFilled(
            bb.Min,
            bb.Max,
            GetColorU32(ImVec4(0.12f, 0.13f, 0.15f, 0.95f)),
            rounding
        );

        // === СВЕТЛАЯ ОБВОДКА (НОРМ UI, НЕ КИСЛОТНАЯ)
        draw->AddRect(
            bb.Min,
            bb.Max,
            GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.08f)),
            rounding,
            0,
            1.0f
        );
    }

    // === РАЗМЕРЫ
    PushFont(font::icomoon);
    ImVec2 icon_size = CalcTextSize(icon);
    PopFont();

    ImVec2 text_size = CalcTextSize(label);

    float center_y = bb.Min.y + tab_height * 0.5f;
    float icon_x = bb.Min.x + 26.0f;
    float text_x = icon_x + icon_size.x + 16.0f;

    // === ИКОНКА
    ImVec4 icon_col = ImVec4(0.6f, 0.6f, 0.6f, 0.85f);

    if (selected)
    {
        ImVec4 accent = menuConfig::accent;

        PushFont(font::icomoon);
        for (int i = 1; i <= 3; i++)
        {
            float a = 0.07f * (1.0f - i / 3.0f) * t;
            draw->AddText(
                ImVec2(icon_x, center_y - icon_size.y * 0.5f),
                GetColorU32(ImVec4(accent.x, accent.y, accent.z, a)),
                icon
            );
        }
        PopFont();

        icon_col = ImVec4(accent.x, accent.y, accent.z, 1.0f);
    }

    PushFont(font::icomoon);
    PushStyleColor(ImGuiCol_Text, icon_col);
    RenderText(
        ImVec2(icon_x, center_y - icon_size.y * 0.5f),
        icon
    );
    PopStyleColor();
    PopFont();

    // === ТЕКСТ
    ImVec4 text_col = selected
        ? ImVec4(1.0f, 1.0f, 1.0f, 0.95f)
        : ImVec4(0.65f, 0.65f, 0.65f, 0.85f);

    PushStyleColor(ImGuiCol_Text, text_col);
    RenderText(
        ImVec2(text_x, center_y - text_size.y * 0.5f),
        label
    );
    PopStyleColor();

    return pressed;
}
}

namespace ImGui
{
    bool CustomSliderInt(const char* label, int* v, int v_min, int v_max, const char* format = "%d", const char* description = nullptr)
    {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems) return false;

        ImGuiContext& g = *GImGui;
        const ImGuiID id = window->GetID(label);

        PushID(id);

        char value_buf[64];
        snprintf(value_buf, sizeof(value_buf), format, *v);

        ImVec2 value_size = CalcTextSize(value_buf);

        float box_height = 120.0f;
        float width = GetContentRegionAvail().x;

        ImVec2 cursor_pos = GetCursorScreenPos();

        ImVec2 box_min = cursor_pos;
        ImVec2 box_max(cursor_pos.x + width, cursor_pos.y + box_height);

        ImDrawList* draw = GetWindowDrawList();

        const float rounding = 4.0f;

        // Полупрозрачный фон и обводка как у свича
        ImU32 box_bg = GetColorU32(ImVec4(0.09f, 0.09f, 0.11f, 0.1f));
        ImU32 box_border = GetColorU32(ImVec4(0.22f, 0.22f, 0.24f, 0.4f));

        draw->AddRectFilled(box_min, box_max, box_bg, rounding);
        draw->AddRect(box_min, box_max, box_border, rounding, 0, 1.0f);

        // Полупрозрачный текст
        PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.95f, 0.95f, 0.9f));
        RenderText(ImVec2(box_min.x + 26.0f, box_min.y + 24.0f), label);
        PopStyleColor();
        
        PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.95f, 0.95f, 0.9f));
        RenderText(ImVec2(box_max.x - value_size.x - 26.0f, box_min.y + 24.0f), value_buf);
        PopStyleColor();

        if (description && description[0] != '\0') {
            PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.70f, 0.70f, 0.7f));
            RenderText(ImVec2(box_min.x + 26.0f, box_min.y + 24.0f + GetTextLineHeight() + 10.0f), description);
            PopStyleColor();
        }

        ImVec2 slider_min(box_min.x + 26.0f, box_min.y + 78.0f);
        ImVec2 slider_max(box_max.x - 26.0f, slider_min.y + 18.0f);
        ImVec2 slider_size(slider_max.x - slider_min.x, 18.0f);

        SetCursorScreenPos(slider_min);
        InvisibleButton("##slider", slider_size);

        bool hovered = IsItemHovered();
        bool active = IsItemActive();
        bool changed = false;

        float t = (v_max > v_min) ? (float)(*v - v_min) / (float)(v_max - v_min) : 0.0f;
        t = ImClamp(t, 0.0f, 1.0f);

        if (active) {
            float mouse_x = g.IO.MousePos.x;
            float new_t = (mouse_x - slider_min.x) / slider_size.x;
            new_t = ImClamp(new_t, 0.0f, 1.0f);
            *v = v_min + (int)lroundf((float)(v_max - v_min) * new_t);
            t = new_t;
            changed = true;
        }

        // Полупрозрачные цвета для трека
        ImU32 track_bg = GetColorU32(ImVec4(0.18f, 0.18f, 0.20f, 0.3f));
        ImU32 track_border = GetColorU32(ImVec4(0.30f, 0.30f, 0.32f, 0.4f));
        ImU32 fill_col = GetColorU32(ImVec4(menuConfig::accent.x, menuConfig::accent.y, menuConfig::accent.z, 0.6f));
        ImU32 knob_col = GetColorU32(ImVec4(0.98f, 0.98f, 0.98f, 0.9f));
        ImU32 glow_col = GetColorU32(ImVec4(menuConfig::accent.x, menuConfig::accent.y, menuConfig::accent.z, (hovered || active) ? 0.2f : 0.1f));

        // Трек слайдера с тонкой обводкой
        draw->AddRectFilled(slider_min, slider_max, track_bg, 9.0f);
        draw->AddRect(slider_min, slider_max, track_border, 9.0f, 0, 1.0f);

        // Заполнение
        ImVec2 fill_max(slider_min.x + slider_size.x * t, slider_max.y);
        draw->AddRectFilled(slider_min, fill_max, fill_col, 9.0f);

        // Кнопка слайдера
        float knob_radius = 13.0f;
        float knob_x = slider_min.x + slider_size.x * t;
        ImVec2 knob_center(knob_x, slider_min.y + slider_size.y * 0.5f);

        // Свечение
        draw->AddCircleFilled(knob_center, knob_radius + 8.0f, glow_col);

        // Легкая тень
        ImVec2 shadow_center(knob_center.x + 0.3f, knob_center.y + 0.3f);
        draw->AddCircleFilled(shadow_center, knob_radius, GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 0.04f)));

        // Кнопка
        draw->AddCircleFilled(knob_center, knob_radius, knob_col);

        SetCursorScreenPos(ImVec2(cursor_pos.x, cursor_pos.y + box_height + 12.0f));

        PopID();

        return changed;
    }
}

namespace ImGui
{
    bool CustomSliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.2f", const char* description = nullptr)
    {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems) return false;

        ImGuiContext& g = *GImGui;
        const ImGuiID id = window->GetID(label);

        PushID(id);

        char value_buf[64];
        snprintf(value_buf, sizeof(value_buf), format, *v);

        ImVec2 value_size = CalcTextSize(value_buf);

        float box_height = 120.0f;
        float width = GetContentRegionAvail().x;

        ImVec2 cursor_pos = GetCursorScreenPos();

        ImVec2 box_min = cursor_pos;
        ImVec2 box_max(cursor_pos.x + width, cursor_pos.y + box_height);

        ImDrawList* draw = GetWindowDrawList();

        const float rounding = 4.0f;

        // Полупрозрачный фон и обводка как у свича
        ImU32 box_bg = GetColorU32(ImVec4(0.09f, 0.09f, 0.11f, 0.1f));
        ImU32 box_border = GetColorU32(ImVec4(0.22f, 0.22f, 0.24f, 0.4f));

        draw->AddRectFilled(box_min, box_max, box_bg, rounding);
        draw->AddRect(box_min, box_max, box_border, rounding, 0, 1.0f);

        // Полупрозрачный текст
        PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.95f, 0.95f, 0.9f));
        RenderText(ImVec2(box_min.x + 26.0f, box_min.y + 24.0f), label);
        PopStyleColor();
        
        PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.95f, 0.95f, 0.9f));
        RenderText(ImVec2(box_max.x - value_size.x - 26.0f, box_min.y + 24.0f), value_buf);
        PopStyleColor();

        if (description && description[0] != '\0') {
            PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.70f, 0.70f, 0.7f));
            RenderText(ImVec2(box_min.x + 26.0f, box_min.y + 24.0f + GetTextLineHeight() + 10.0f), description);
            PopStyleColor();
        }

        ImVec2 slider_min(box_min.x + 26.0f, box_min.y + 78.0f);
        ImVec2 slider_max(box_max.x - 26.0f, slider_min.y + 18.0f);
        ImVec2 slider_size(slider_max.x - slider_min.x, 18.0f);

        SetCursorScreenPos(slider_min);
        InvisibleButton("##slider", slider_size);

        bool hovered = IsItemHovered();
        bool active = IsItemActive();
        bool changed = false;

        float t = (v_max > v_min) ? (*v - v_min) / (v_max - v_min) : 0.0f;
        t = ImClamp(t, 0.0f, 1.0f);

        if (active) {
            float mouse_x = g.IO.MousePos.x;
            float new_t = (mouse_x - slider_min.x) / slider_size.x;
            new_t = ImClamp(new_t, 0.0f, 1.0f);
            *v = v_min + (v_max - v_min) * new_t;
            t = new_t;
            changed = true;
        }

        // Полупрозрачные цвета для трека
        ImU32 track_bg = GetColorU32(ImVec4(0.18f, 0.18f, 0.20f, 0.3f));
        ImU32 track_border = GetColorU32(ImVec4(0.30f, 0.30f, 0.32f, 0.4f));
        ImU32 fill_col = GetColorU32(ImVec4(menuConfig::accent.x, menuConfig::accent.y, menuConfig::accent.z, 0.6f));
        ImU32 knob_col = GetColorU32(ImVec4(0.98f, 0.98f, 0.98f, 0.9f));
        ImU32 glow_col = GetColorU32(ImVec4(menuConfig::accent.x, menuConfig::accent.y, menuConfig::accent.z, (hovered || active) ? 0.2f : 0.1f));

        // Трек слайдера с тонкой обводкой
        draw->AddRectFilled(slider_min, slider_max, track_bg, 9.0f);
        draw->AddRect(slider_min, slider_max, track_border, 9.0f, 0, 1.0f);

        // Заполнение
        ImVec2 fill_max(slider_min.x + slider_size.x * t, slider_max.y);
        draw->AddRectFilled(slider_min, fill_max, fill_col, 9.0f);

        // Кнопка слайдера
        float knob_radius = 13.0f;
        float knob_x = slider_min.x + slider_size.x * t;
        ImVec2 knob_center(knob_x, slider_min.y + slider_size.y * 0.5f);

        // Свечение
        draw->AddCircleFilled(knob_center, knob_radius + 8.0f, glow_col);

        // Легкая тень
        ImVec2 shadow_center(knob_center.x + 0.3f, knob_center.y + 0.3f);
        draw->AddCircleFilled(shadow_center, knob_radius, GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 0.04f)));

        // Кнопка
        draw->AddCircleFilled(knob_center, knob_radius, knob_col);

        SetCursorScreenPos(ImVec2(cursor_pos.x, cursor_pos.y + box_height + 12.0f));

        PopID();

        return changed;
    }
}

namespace ImGui
{
bool CustomSwitch(const char* label, bool* v)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext& g = *GImGui;  
    const ImGuiID id = window->GetID(label);  

    float height = 72.0f;  
    float width = window->WorkRect.GetWidth();  
    ImVec2 pos = window->DC.CursorPos;  

    ImRect bb(pos, ImVec2(pos.x + width, pos.y + height));  
    ItemSize(bb);  
    if (!ItemAdd(bb, id)) return false;  

    bool hovered, held;  
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);  
    if (pressed)  
    {  
        *v = !*v;  
        MarkItemEdited(id);  
    }  

    static std::unordered_map<ImGuiID, float> anim;  
    float& t = anim[id];  
    float target = *v ? 1.0f : 0.0f;  
    t += (target - t) * ImClamp(g.IO.DeltaTime * 12.0f, 0.0f, 1.0f);  

    ImDrawList* draw = GetWindowDrawList();  

    ImU32 bg_col = GetColorU32(ImVec4(0.09f, 0.09f, 0.11f, 0.1f));  
    ImU32 border_col = GetColorU32(ImVec4(0.22f, 0.22f, 0.24f, 0.4f));  

    draw->AddRectFilled(bb.Min, bb.Max, bg_col, 4.0f);          // ← здесь изменил только это
    draw->AddRect(bb.Min, bb.Max, border_col, 4.0f, 0, 1.3f);  // рамка осталась с 16

    float switch_w = 86.0f;  
    float switch_h = 42.0f;  
    float r = switch_h * 0.5f;  

    ImVec2 sw_pos(  
        bb.Max.x - switch_w - 20.0f,  
        bb.Min.y + (height - switch_h) * 0.5f  
    );  

    ImRect sw(sw_pos, ImVec2(sw_pos.x + switch_w, sw_pos.y + switch_h));  

    ImVec4 off_col(0.18f, 0.18f, 0.20f, 0.6f);  
    ImVec4 on_col(menuConfig::accent.x, menuConfig::accent.y, menuConfig::accent.z, 0.7f);  

    draw->AddRectFilled(sw.Min, sw.Max, GetColorU32(ImLerp(off_col, on_col, t)), r);  
    draw->AddRect(sw.Min, sw.Max, border_col, r, 0, 1.1f);  

    float knob_r = 18.0f;  
    float kx = ImLerp(sw.Min.x + r, sw.Max.x - r, t);  
    ImVec2 kc(kx, sw.GetCenter().y);  

    draw->AddCircleFilled(kc, knob_r, GetColorU32(ImVec4(0.98f, 0.98f, 0.98f, 0.95f)));  

    RenderText(  
        ImVec2(  
            bb.Min.x + 22.0f,  
            bb.Min.y + (height - GetTextLineHeight()) * 0.5f  
        ),  
        label  
    );  

    Dummy(ImVec2(0.0f, 8.0f));  
    return pressed;
}
}

namespace ImGui
{
    bool CustomCombo(const char* label, int* current_item, const char* items[], int items_count, const char* description = nullptr)
    {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems) return false;

        ImGuiContext& g = *GImGui;
        const ImGuiID id = window->GetID(label);

        ImVec2 label_size = CalcTextSize(label);
        ImVec2 desc_size = (description && description[0]) ? CalcTextSize(description) : ImVec2(0, 0);
        
        // Высота как у слайдера и кнопки
        float line_h = GetTextLineHeight();
        float text_height = line_h + (desc_size.y > 0 ? line_h + 8.0f : 0.0f);
        float total_height = text_height + 40.0f;

        ImVec2 pos = window->DC.CursorPos;
        float width = window->WorkRect.GetWidth();
        
        ImRect bb(pos, ImVec2(pos.x + width, pos.y + total_height));
        ItemSize(bb, 0.0f);
        if (!ItemAdd(bb, id)) return false;

        bool hovered, held;
        bool pressed = ButtonBehavior(bb, id, &hovered, &held);
        
        if (pressed) {
            OpenPopup(label);
        }

        // Рендеринг
        ImDrawList* draw = window->DrawList;
        const float rounding = 4.0f;
        
        // ПОЛУПРОЗРАЧНЫЙ ФОН И ОБВОДКА КАК У СВИЧА
        ImU32 box_bg = GetColorU32(ImVec4(0.09f, 0.09f, 0.11f, 0.1f));
        ImU32 box_border = GetColorU32(ImVec4(0.22f, 0.22f, 0.24f, 0.4f));
        
        // Эффект при наведении
        if (hovered) {
            ImU32 hover_bg = GetColorU32(ImVec4(0.12f, 0.12f, 0.14f, 0.15f));
            draw->AddRectFilled(bb.Min, bb.Max, hover_bg, rounding);
        }
        
        // Фон и обводка
        draw->AddRectFilled(bb.Min, bb.Max, box_bg, rounding);
        draw->AddRect(bb.Min, bb.Max, box_border, rounding, 0, 1.0f); // Тонкая обводка
        
        // ПОЛУПРОЗРАЧНЫЙ ТЕКСТ
        float text_y = bb.Min.y + 20.0f;
        
        // Лейбл слева
        PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.95f, 0.95f, 0.9f));
        RenderText(ImVec2(bb.Min.x + 26.0f, text_y), label);
        PopStyleColor();
        
        // Описание если есть
        if (description && description[0] != '\0') {
            PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.70f, 0.70f, 0.7f));
            RenderText(ImVec2(bb.Min.x + 26.0f, text_y + line_h + 8.0f), description);
            PopStyleColor();
        }
        
        // Выбранное значение справа
        if (*current_item >= 0 && *current_item < items_count) {
            const char* selected_text = items[*current_item];
            ImVec2 text_size = CalcTextSize(selected_text);
            
            // Стрелка вниз "v"
            const char* arrow = "";
            ImVec2 arrow_size = CalcTextSize(arrow);
            float arrow_x = bb.Max.x - 26.0f - arrow_size.x;
            float arrow_y = bb.GetCenter().y - arrow_size.y * 0.5f;
            
            // Текст перед стрелкой
            float text_x = arrow_x - text_size.x - 8.0f;
            float text_y_center = bb.GetCenter().y - text_size.y * 0.5f;
            
            // Полупрозрачный цвет текста
            PushStyleColor(ImGuiCol_Text, ImVec4(0.90f, 0.90f, 0.90f, 0.9f));
            RenderText(ImVec2(text_x, text_y_center), selected_text);
            RenderText(ImVec2(arrow_x, arrow_y), arrow);
            PopStyleColor();
        }
        
        // Попап с выбором
        bool changed = false;
        if (BeginPopup(label)) {
            // Устанавливаем размер попапа
            SetNextWindowSize(ImVec2(bb.GetWidth() - 52.0f, 0), ImGuiCond_Always);
            
            // Сохраняем оригинальные стили
            ImGuiStyle& style = GetStyle();
            ImVec2 originalWindowPadding = style.WindowPadding;
            ImVec2 originalFramePadding = style.FramePadding;
            ImVec2 originalItemSpacing = style.ItemSpacing;
            
            // Устанавливаем компактные стили
            style.WindowPadding = ImVec2(8.0f, 8.0f);
            style.FramePadding = ImVec2(12.0f, 6.0f);
            style.ItemSpacing = ImVec2(8.0f, 4.0f);
            
            // ПОЛУПРОЗРАЧНЫЕ СТИЛИ ДЛЯ ПОПАПА
            PushStyleColor(ImGuiCol_Header, GetColorU32(ImVec4(0.09f, 0.09f, 0.11f, 0.1f)));
            PushStyleColor(ImGuiCol_HeaderHovered, GetColorU32(ImVec4(0.15f, 0.15f, 0.17f, 0.8f)));
            PushStyleColor(ImGuiCol_HeaderActive, GetColorU32(ImVec4(0.20f, 0.20f, 0.22f, 0.8f)));
            PushStyleColor(ImGuiCol_FrameBg, GetColorU32(ImVec4(0.09f, 0.09f, 0.11f, 0.1f)));
            PushStyleColor(ImGuiCol_FrameBgHovered, GetColorU32(ImVec4(0.12f, 0.12f, 0.14f, 0.3f)));
            PushStyleColor(ImGuiCol_FrameBgActive, GetColorU32(ImVec4(0.15f, 0.15f, 0.17f, 0.3f)));
            PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.95f, 0.95f, 0.9f));
            PushStyleColor(ImGuiCol_Border, GetColorU32(ImVec4(0.22f, 0.22f, 0.24f, 0.4f)));
            PushStyleColor(ImGuiCol_PopupBg, GetColorU32(ImVec4(0.09f, 0.09f, 0.11f, 0.95f))); // Полупрозрачный фон попапа
            
            // Простой список через Selectable
            for (int i = 0; i < items_count; i++) {
                bool is_selected = (*current_item == i);
                
                // Используем стандартный Selectable
                if (Selectable(items[i], is_selected)) {
                    *current_item = i;
                    changed = true;
                    CloseCurrentPopup();
                }
                
                // Устанавливаем фокус если выбран
                if (is_selected) {
                    SetItemDefaultFocus();
                }
            }
            
            // Восстанавливаем стили
            PopStyleColor(9);
            style.WindowPadding = originalWindowPadding;
            style.FramePadding = originalFramePadding;
            style.ItemSpacing = originalItemSpacing;
            
            EndPopup();
        }
        
        // Отступ после комбобокса
        Dummy(ImVec2(0.0f, 10.0f));

        return changed;
    }

    // Альтернативная версия с const char* для списка через разделитель
    bool CustomCombo(const char* label, int* current_item, const char* items_separated_by_zeros, const char* description = nullptr)
    {
        const char* items[256];
        int items_count = 0;
        
        // Парсим строку с элементами
        const char* p = items_separated_by_zeros;
        while (*p && items_count < 256) {
            items[items_count++] = p;
            while (*p) p++;
            p++; // Пропускаем нулевой символ
        }
        
        return CustomCombo(label, current_item, items, items_count, description);
    }

    // Версия для std::vector<std::string>
    bool CustomCombo(const char* label, int* current_item, const std::vector<std::string>& items, const char* description = nullptr)
    {
        // Конвертируем vector<string> в массив const char*
        std::vector<const char*> c_items;
        c_items.reserve(items.size());
        for (const auto& item : items) {
            c_items.push_back(item.c_str());
        }
        
        return CustomCombo(label, current_item, c_items.data(), (int)items.size(), description);
    }
}

namespace ImGui
{
    void TextGradient(const char* text)
    {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems) return;

        ImDrawList* draw = window->DrawList;
        ImFont* font = GetFont();
        float font_size = GetFontSize();

        ImVec2 cursor_pos = GetCursorScreenPos();

        int len = (int)strlen(text);

        ImU32 glow_color = ColorConvertFloat4ToU32(ImVec4(menuConfig::accent.x, menuConfig::accent.y, menuConfig::accent.z, 0.15f));
        draw->AddText(font, font_size, cursor_pos, glow_color, text); // Тень для glow

        float xOffset = 0.0f;
        for (int i = 0; i < len; i++)
        {
            char c[2] = { text[i], 0 };
            ImVec2 charSize = CalcTextSize(c);
            float t = i / (float)len;

            ImVec4 col(
                menuConfig::accent.x * (1.0f - t) + 0.95f * t,
                menuConfig::accent.y * (1.0f - t) + 0.95f * t,
                menuConfig::accent.z * (1.0f - t) + 0.95f * t,
                1.0f
            );

            draw->AddText(font, font_size, ImVec2(cursor_pos.x + xOffset, cursor_pos.y), ColorConvertFloat4ToU32(col), c);
            xOffset += charSize.x;
        }

        Dummy(ImVec2(xOffset, 0)); // Чтобы ImGui правильно сдвигал курсор
    }
}

namespace ImGui
{
    bool TextTab(bool selected, const char* label, int page_id)
    {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems) return false;

        ImGuiContext& g = *GImGui;

        // уникальный ID через статическую строку
        static std::string tempStr;
        tempStr = std::string(label) + "_texttab_" + std::to_string(page_id);
        ImGuiID id = window->GetID(tempStr.c_str());

        ImVec2 pos = window->DC.CursorPos;
        ImVec2 text_size = CalcTextSize(label);
        float height = text_size.y + 8.0f;

        ImRect bb(pos, ImVec2(pos.x + text_size.x, pos.y + height));
        ItemSize(bb);
        if (!ItemAdd(bb, id)) return false;

        bool hovered, held;
        bool pressed = ButtonBehavior(bb, id, &hovered, &held);

        static std::unordered_map<ImGuiID, float> anim;
        float& t = anim[id];
        float target = selected ? 1.0f : 0.0f;
        t = ImLerp(t, target, g.IO.DeltaTime * 15.0f);

        ImDrawList* draw = window->DrawList;
        ImVec2 text_pos(bb.Min.x, bb.Min.y);

        draw->AddText(
            text_pos,
            GetColorU32(selected ? ImVec4(1,1,1,1) : ImVec4(0.6f,0.6f,0.6f,0.85f)),
            label
        );

        if (t > 0.01f)
        {
            ImVec4 accent = menuConfig::accent;
            ImVec4 grad_l(accent.x*0.9f, accent.y*0.9f, accent.z*0.9f,1.0f);
            ImVec4 grad_r(ImMin(accent.x+0.2f,1.0f), ImMin(accent.y+0.2f,1.0f), ImMin(accent.z+0.2f,1.0f),1.0f);

            float line_thickness = 4.5f;
            float rounding = 2.5f;
            float glow_radius = 10.0f;

            ImVec2 line_start(text_pos.x, text_pos.y + text_size.y + 2.0f);
            ImVec2 line_end(text_pos.x + text_size.x * t, line_start.y + line_thickness);

            // Glow вокруг линии (мягкий, по слоям)
            for (int i=1; i<=5; i++)
            {
                float alpha = 0.06f * (1.0f - i/5.0f) * t;
                float radius = i * glow_radius * 0.5f;
                draw->AddRect(
                    ImVec2(line_start.x - radius, line_start.y - radius),
                    ImVec2(line_end.x + radius, line_end.y + radius),
                    GetColorU32(ImVec4(accent.x, accent.y, accent.z, alpha)),
                    rounding
                );
            }

            // Основная градиентная линия с закруглением
            draw->AddRectFilledMultiColor(
                line_start, line_end,
                GetColorU32(grad_l),
                GetColorU32(grad_r),
                GetColorU32(grad_r),
                GetColorU32(grad_l)
            );

            // Добавляем маленькие закругленные углы вручную (верхний и нижний)
            draw->AddCircleFilled(
                ImVec2(line_start.x, line_start.y + line_thickness*0.5f),
                line_thickness*0.5f,
                GetColorU32(grad_l)
            );
            draw->AddCircleFilled(
                ImVec2(line_end.x, line_start.y + line_thickness*0.5f),
                line_thickness*0.5f,
                GetColorU32(grad_r)
            );
        }

        return pressed;
    }
}

static void MenuText(const char* text)
{
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::TextColored(ImColor(120, 120, 120), text);
    ImGui::Dummy(ImVec2(0, 8.0f));
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Dummy(ImVec2(0, 8.0f));
}
