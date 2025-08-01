#pragma once
#include "shape.h"
class Circle : public Shape {
public:
    float radius;

    Circle(ImVec2 pos, float r, ImU32 col, const std::string& n = "Circle")
        : Shape(pos, col, n), radius(r) {}

    // Override draw function for Circle
    void draw(ImDrawList* draw_list, ImVec2 canvas_origin_screen_pos) const override {
        // Calculate absolute screen position for drawing
        ImVec2 screen_pos = ImVec2(canvas_origin_screen_pos.x + position.x, canvas_origin_screen_pos.y + position.y);
        draw_list->AddCircleFilled(screen_pos, radius, color);
        // Draw a border if selected
        if (isSelected) {
            draw_list->AddCircle(screen_pos, radius + 2.0f, IM_COL32(255, 255, 0, 255), 0, 2.0f); // Yellow border
        }
    }

    // Override contains function for Circle
    bool contains(ImVec2 point_in_canvas_coords) const override {
        float dx = point_in_canvas_coords.x - position.x;
        float dy = point_in_canvas_coords.y - position.y;
        return (dx * dx + dy * dy) <= (radius * radius);
    }
};