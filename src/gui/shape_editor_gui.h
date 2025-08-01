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
    float newShapeColor[3] = {1.0f, 0.0f, 0.0f}; // RGB as floats (red by default)
    char newShapeNameBuffer[128] = ""; // For C-style string input

public:
    ShapeEditorGUI() {
        // Add some initial shapes (positions are canvas-relative now) to test shape code
        shapes.push_back(std::make_unique<Circle>(ImVec2(100, 100), 50.0f, IM_COL32(0, 255, 0, 255), "Green Circle"));
        shapes.push_back(std::make_unique<Rectangle>(ImVec2(200, 50), ImVec2(100, 70), IM_COL32(0, 0, 255, 255), "Blue Rect"));
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
};