//========================================================================
// Copyright (c) 2025 hung-truong

#pragma once
#include "shape.h"
class CircleShape : public Shape {
public:
    float radius;

    CircleShape(ImVec2 pos, float r, const std::array<float, 3>& col, const std::string& n = "Circle")
        : Shape(pos, col, n), radius(r) {}

    // Override draw function for CircleShape
    void draw(ImDrawList* draw_list, ImVec2 canvas_origin_screen_pos) const override {
        // Calculate absolute screen position for drawing
        ImVec2 screen_pos = ImVec2(canvas_origin_screen_pos.x + position.x, canvas_origin_screen_pos.y + position.y);
        ImU32 drawColor = IM_COL32(
            (int)(color[0] * 255),
            (int)(color[1] * 255),
            (int)(color[2] * 255),
            255
        );
        draw_list->AddCircleFilled(screen_pos, radius, drawColor);
        // Draw a border if selected
        if (isSelected) {
            draw_list->AddCircle(screen_pos, radius + 2.0f, IM_COL32(255, 255, 0, 255), 0, 2.0f); // Yellow border
        }
    }

    // Override contains function for CircleShape
    bool contains(ImVec2 point_in_canvas_coords) const override {
        float dx = point_in_canvas_coords.x - position.x;
        float dy = point_in_canvas_coords.y - position.y;
        return (dx * dx + dy * dy) <= (radius * radius);
    }

    void clampPosition(const ImVec2& canvas_size) override {
        ImVec2 newPosition = ImVec2(
            position.x + ImGui::GetIO().MouseDelta.x,
            position.y + ImGui::GetIO().MouseDelta.y
        );

        position.x = std::max(radius, std::min(canvas_size.x - radius, newPosition.x));
        position.y = std::max(radius, std::min(canvas_size.y - radius, newPosition.y));
    }

    std::unique_ptr<Shape> clone() const override {
        return std::make_unique<CircleShape>(position, radius, color, name);
    }
};