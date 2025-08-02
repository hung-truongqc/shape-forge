//========================================================================
// Copyright (c) 2025 hung-truong
// --- Main GUI Class for Shape Editor ---

#pragma once
#include "circle.h"
#include "rectangle.h"
class ShapeEditorGUI {
private:
    // Using std::vector of std::unique_ptr for owning polymorphic shapes
    std::vector<std::unique_ptr<Shape>> shapes;
    int selectedShapeIndex = -1; // Index of the currently selected shape

    // For new shape creation (these are now defaults for the "Add" buttons, not click-to-add)
    float newCircleRadius = 50.0f;
    ImVec2 newRectSize = ImVec2(80, 60);
    std::array<float, 3> newShapeColor = {1.0f, 1.0f, 1.0f}; // RGB as floats (white by default)
    char newShapeNameBuffer[128] = ""; // For C-style string input
    bool showMenuBar = false;

public:
    ShapeEditorGUI() {
        // Add some initial shapes (positions are canvas-relative now) to test shape code
        const std::array<float, 3> green = {0.0f, 1.0f, 0.0f};
        shapes.push_back(std::make_unique<Circle>(ImVec2(100, 100), 50.0f, green, "Green Circle"));
        const std::array<float, 3> blue = {0.0f, 0.0f, 1.0f};
        shapes.push_back(std::make_unique<Rectangle>(ImVec2(200, 50), ImVec2(100, 70),blue, "Blue Rect"));
    }
    void render();

private:
    // The Function render the control panel on the left side of the application
    void renderControlsPanel();

    // The function render the canvas panel on the right side of the application
    void renderCanvasPanel();

    // Generic function to add a shape
    template<typename T, typename... Args>
    void addShape(Args&&... args);

    // The function handle the logic for changing shape of cursor, when hover or dragging shape object
    void handleMouseShape(const bool& is_canvas_hovered, const ImVec2& mouse_pos_in_canvas);

    // TBD - For features to export and import a JSON contains all Shapes of the current canvas
    void importJson();
    void exportJson();
};