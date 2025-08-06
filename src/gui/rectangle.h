//========================================================================
// Copyright (c) 2025 hung-truong

#pragma once
#include "shape.h"
class RectangleShape : public Shape {
public:
    ImVec2 size;

    RectangleShape(ImVec2 pos, ImVec2 s, const std::array<float, 3>& col, const std::string& n = "Rectangle")
        : Shape(pos, col, n), size(s) {}

    // Override draw function for RectangleShape
    void draw(ImDrawList* draw_list, ImVec2 canvas_origin_screen_pos) const override {
        // Calculate absolute screen positions for drawing
        ImVec2 p_min_screen = ImVec2(canvas_origin_screen_pos.x + position.x, canvas_origin_screen_pos.y + position.y);
        ImVec2 p_max_screen = ImVec2(p_min_screen.x + size.x, p_min_screen.y + size.y);
        ImU32 drawColor = IM_COL32(
            (int)(color[0] * 255),
            (int)(color[1] * 255),
            (int)(color[2] * 255),
            255
        );        
        draw_list->AddRectFilled(p_min_screen, p_max_screen, drawColor);
        // Draw a border if selected
        if (isSelected) {
            draw_list->AddRect(p_min_screen, p_max_screen, IM_COL32(255, 255, 0, 255), 0, 2.0f); // Yellow border
        }
    }

    // Override contains function for RectangleShape
    bool contains(ImVec2 point_in_canvas_coords) const override {
        return point_in_canvas_coords.x >= position.x && point_in_canvas_coords.x <= (position.x + size.x) &&
               point_in_canvas_coords.y >= position.y && point_in_canvas_coords.y <= (position.y + size.y);
    }

    void clampPosition(const ImVec2& canvas_size) override {

        ImVec2 newPosition = ImVec2(
            position.x + ImGui::GetIO().MouseDelta.x,
            position.y + ImGui::GetIO().MouseDelta.y
        );
        position.x = std::max(0.0f, std::min(canvas_size.x - size.x, newPosition.x));
        position.y = std::max(0.0f, std::min(canvas_size.y - size.y, newPosition.y));
    }

    //Override function for cloning
    std::unique_ptr<Shape> clone() const override {
        return std::make_unique<RectangleShape>(position, size, color, name);
    }
};