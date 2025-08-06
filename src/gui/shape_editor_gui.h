//========================================================================
// Copyright (c) 2025 hung-truong
// --- Main GUI Class for Shape Editor ---

#pragma once
#include "shape.h"
#include "circle.h"
#include "rectangle.h"
#include "shape_clipboard.h"

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
    // Clipboard system
    ShapeClipboard clipboardSystem;

public:
    ShapeEditorGUI() {
        // Add some initial shapes (positions are canvas-relative now) to test shape code
        const std::array<float, 3> green = {0.0f, 1.0f, 0.0f};
        shapes.push_back(std::make_unique<CircleShape>(ImVec2(100, 100), 50.0f, green, "Green Circle"));
        const std::array<float, 3> blue = {0.0f, 0.0f, 1.0f};
        shapes.push_back(std::make_unique<RectangleShape>(ImVec2(200, 50), ImVec2(100, 70),blue, "Blue Rect"));
    }
    void render();

private:
    // The Function render the control panel on the left side of the application
    void renderControlsPanel();

    // The function render the canvas panel on the right side of the application
    void renderCanvasPanel();

    // Renders a right-click context menu on the canvas area.
    // Offers options to cut, copy, paste, or delete the currently selected shape.
    void renderCanvasContextMenu(const bool& is_canvas_hovered);

    // Adds a new shape to the canvas and makes it the selected shape.
    // Any previously selected shape will be deselected.
    template<typename T, typename... Args>
    void addShape(Args&&... args);

    // Cuts the currently selected shape.
    // It copies the selected shape to the clipboard, then removes it from the canvas.
    void cutShape();

    // Copies the currently selected shape to the clipboard.
    void copyShape();

    // Pastes the shape currently stored in the clipboard onto the canvas.
    // The pasted shape is offset in position and automatically selected.
    void pasteShape();

    // Deletes the currently selected shape from the canvas.
    void deleteShape();

    // The function handle the logic for changing shape of cursor, when hover or dragging shape object
    void handleMouseShape(const bool& is_canvas_hovered, const ImVec2& mouse_pos_in_canvas);

    // TBD - For features to export and import a JSON contains all Shapes on the current canvas
    void importJson();
    void exportJson();
};