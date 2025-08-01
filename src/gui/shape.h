#pragma once
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// Cross-platform OpenGL loader
#ifdef _WIN32
    #include <GL/glew.h>
#else
    #include <GL/gl3w.h>
#endif
#include <GLFW/glfw3.h>

#include <vector>
#include <string>
#include <cmath> // For M_PI if needed, or define it
#include <memory> // For std::unique_ptr
#include <cstdio> // For sprintf

// --- Base Shape Class ---
// An abstract base class for all drawable shapes.
class Shape {
public:
    // Virtual destructor is crucial for proper cleanup of derived classes
    virtual ~Shape() = default;

    // Pure virtual function to draw the shape using ImGui's draw list
    // Now takes canvas_origin_screen_pos to draw correctly relative to the canvas
    virtual void draw(ImDrawList* draw_list, ImVec2 canvas_origin_screen_pos) const = 0;

    // Pure virtual function to check if a point is inside the shape
    // Point is expected to be in canvas-local coordinates
    virtual bool contains(ImVec2 point_in_canvas_coords) const = 0;

    // Common properties for all shapes
    ImVec2 position; // Position is now relative to the canvas's top-left corner
    ImU32 color;
    bool isSelected = false;
    std::string name;

    Shape(ImVec2 pos, ImU32 col, const std::string& n = "Shape")
        : position(pos), color(col), name(n) {}
};