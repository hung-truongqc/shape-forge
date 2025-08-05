//========================================================================
// Copyright (c) 2025 hung-truong

#include "shape_editor_gui.h"
#include <cmath>

void ShapeEditorGUI::render()
{
    // --- Add the Menu Bar at the top of the entire window ---
    ImGuiIO& io = ImGui::GetIO();
    // Check for a single press of the Alt key to toggle the menu bar visibility
    // ImGuiKey_LeftAlt and ImGuiKey_RightAlt are used for specific alt keys
    if (ImGui::IsKeyPressed(ImGuiKey_LeftAlt, false) || ImGui::IsKeyPressed(ImGuiKey_RightAlt, false)) {
        showMenuBar = !showMenuBar;
    }

    float menu_bar_height = 0.0f;
    if(showMenuBar) {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Import Shapes JSON")) {
                    // Call the import function when selected
                    importJson();
                }
                if (ImGui::MenuItem("Export Shapes JSON")) {
                    // Call the export function when selected
                    exportJson();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        // Get the height of the menu bar only when it's rendered
        menu_bar_height = ImGui::GetFrameHeight();
    }

    // --- Make the ImGui window fill the entire GLFW window ---
    ImGui::SetNextWindowPos(ImVec2(0, menu_bar_height));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y - menu_bar_height));

    // --- Remove window decorations and make it non-movable/resizable ---
    ImGui::Begin("Shape Forge", nullptr,
                    ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground); // NoBackground to fully cover OpenGL clear color

    // Calculate available space for the two main panels
    ImVec2 main_content_size = ImGui::GetContentRegionAvail();
    float left_panel_width = 300.0f; // Fixed width for controls

    // Left Panel (Controls) - occupies a fixed width and fills vertical space
    // ImGui::BeginChild is used to create sub-regions that manage their own layout and scrolling
    ImGui::BeginChild("ControlsPanel",
                    ImVec2(left_panel_width, 0), true, ImGuiWindowFlags_AlwaysUseWindowPadding); // 0 height means fill available vertical
    renderControlsPanel();
    ImGui::EndChild();

    ImGui::SameLine(); // Place the next item on the same line

    // Right Panel (Drawing Canvas) - fills remaining horizontal and vertical space
    ImGui::BeginChild("DrawingCanvasPanel",
                    ImVec2(main_content_size.x - left_panel_width - ImGui::GetStyle().ItemSpacing.x, 0),
                    false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse); // Fill remaining horizontal, 0 height means fill available vertical
    renderCanvasPanel();
    ImGui::EndChild();

    ImGui::End(); // End ImGui window
}

// Modified renderControlsPanel() method:
void ShapeEditorGUI::renderControlsPanel()
{
    ImGui::Text("Shape Creation");
    ImGui::Separator();

    ImGui::InputText("Name###New", newShapeNameBuffer, sizeof(newShapeNameBuffer));
    // Edit color as RGB
    ImGui::ColorEdit3("Color###New", newShapeColor.data()); // Convert to float* raw pointer for IMGUI

    ImGui::Separator();

    // 3D Mode Toggle Button
    if (ImGui::Button(is3DMode ? "2D Mode" : "3D Mode", ImVec2(120, 0))) {
        is3DMode = !is3DMode;
        // Reset rotation when switching modes
        if (!is3DMode) {
            rotationAngle = 0.0f;
        }
    }

    // Show rotation speed control only in 3D mode
    if (is3DMode) {
        static float rotationSpeedX = 0.8f;  // Up-down rotation speed
        static float rotationSpeedY = 1.2f;  // Left-right rotation speed

        ImGui::SliderFloat("Horizontal Speed", &rotationSpeedY, 0.1f, 3.0f, "%.1f");
        ImGui::SliderFloat("Vertical Speed", &rotationSpeedX, 0.1f, 3.0f, "%.1f");

        // Update rotation angles based on time and speed
        rotationAngleY += rotationSpeedY * ImGui::GetIO().DeltaTime;
        rotationAngleX += rotationSpeedX * ImGui::GetIO().DeltaTime;

        // Keep angles in reasonable range
        if (rotationAngleY > 2.0f * M_PI) {
            rotationAngleY -= 2.0f * M_PI;
        }
        if (rotationAngleX > 2.0f * M_PI) {
            rotationAngleX -= 2.0f * M_PI;
        }
    }

    ImGui::Separator();

    // Circle creation
    ImGui::Text("Circle Properties:");
    ImGui::SliderFloat("Radius", &newCircleRadius, 10.0f, 150.0f, "%.1f");
    // Disable shape creation buttons in 3D mode
    ImGui::BeginDisabled(is3DMode);
    if (ImGui::Button("Add Circle", ImVec2(120, 0))) {
        addShape<Circle>(ImVec2(100,100), newCircleRadius); // Default pos
    }
    ImGui::EndDisabled();

    ImGui::Separator();

    // Rectangle creation
    ImGui::Text("Rectangle Properties:");
    ImGui::SliderFloat2("Size", (float*)&newRectSize, 10.0f, 200.0f, "%.1f");
    ImGui::BeginDisabled(is3DMode);
    if (ImGui::Button("Add Rectangle", ImVec2(120, 0))) {
        addShape<Rectangle>(ImVec2(150,150), newRectSize); // Default pos
    }
    ImGui::EndDisabled();

    ImGui::Separator();

    // Shape List and Properties
    ImGui::Text("Shapes (%zu):", shapes.size());
    if (is3DMode) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "3D Mode: Shapes are rotating");
    }

    // Use ImGui::GetContentRegionAvail().y to make the child window fill remaining vertical space
    // Subtract space for the "Quit Application" button and its spacing
    float remaining_height_for_list = ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing() - ImGui::GetStyle().ItemSpacing.y;
    ImGui::BeginChild("ShapeList", ImVec2(0, remaining_height_for_list), true);

    // Disable shape editing in 3D mode
    ImGui::BeginDisabled(is3DMode);

    for (int i = 0; i < shapes.size(); ++i) {
        ImGui::PushID(i);
        bool isCurrentSelected = (selectedShapeIndex == i);

        // Using a char buffer and sprintf for string formatting
        char label_buffer[256]; // Sufficiently large buffer
        const char* shape_type = (dynamic_cast<Circle*>(shapes[i].get()) ? "Circle" : "Rect");
        snprintf(label_buffer, sizeof(label_buffer),"%s (%s @ %.0f,%.0f)",
                shapes[i]->name.c_str(), shape_type,
                shapes[i]->position.x, shapes[i]->position.y);

        if (ImGui::Selectable(label_buffer, isCurrentSelected)) { // Use the buffer here
            if (!is3DMode) { // Only allow selection in 2D mode
                if (selectedShapeIndex != -1) {
                    shapes[selectedShapeIndex]->isSelected = false; // Deselect previous
                }
                selectedShapeIndex = i;
                shapes[selectedShapeIndex]->isSelected = true; // Select new
            }
        }

        if (isCurrentSelected && !is3DMode) {
            ImGui::Indent();
            ImGui::Text("Properties:");
            ImGui::ColorEdit3("Color##Edit", shapes[i]->color.data()); // Convert to float* raw pointer for IMGUI
            ImGui::InputFloat2("Position##Edit", (float*)&shapes[i]->position);

            // Specific properties for Circle
            if (auto* circle = dynamic_cast<Circle*>(shapes[i].get())) {
                ImGui::SliderFloat("Radius##Edit", &circle->radius, 10.0f, 150.0f, "%.1f");
            }
            // Specific properties for Rectangle
            else if (auto* rect = dynamic_cast<Rectangle*>(shapes[i].get())) {
                ImGui::SliderFloat2("Size##Edit", (float*)&rect->size, 10.0f, 200.0f, "%.1f");
            }

            if (ImGui::Button("Delete", ImVec2(80, 0))) {
                shapes.erase(shapes.begin() + i);
                selectedShapeIndex = -1;
                ImGui::PopID();
                break; // Break loop as vector size changed
            }
            ImGui::Unindent();
        }
        ImGui::PopID();
    }

    ImGui::EndDisabled(); // End disable for 3D mode
    ImGui::EndChild();

    ImGui::Separator();
    // Exit Button
    if (ImGui::Button("Quit Application", ImVec2(0, 0))) {
        // Signal GLFW to close the window
        glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
    }
}

void ShapeEditorGUI::handleMouseShape(const bool& is_canvas_hovered, const ImVec2& mouse_pos_in_canvas)
{
    // --- Cursor logic for shapes ---
    if (is_canvas_hovered) {
        bool shapeHovered = false;
        // Iterate through shapes in reverse order to check for the topmost shape
        for (int i = shapes.size() - 1; i >= 0; --i) {
            if (shapes[i]->contains(mouse_pos_in_canvas)) {
                // If a shape is being dragged, show the grab cursor
                if (selectedShapeIndex != -1 && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
                } else {
                    // Otherwise, show the hand cursor for hovering
                    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                }
                shapeHovered = true;
                break; // Found the topmost shape, no need to check others
            }
        }
        // If nothing is hovered, revert to the default arrow
        if (!shapeHovered) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
        }
    }
}

void ShapeEditorGUI::renderCanvasContextMenu(const bool& is_canvas_hovered)
{
    // Open context menu on right-click
    if (is_canvas_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        ImGui::OpenPopup("CanvasContextMenu");
    }

    // Render the context menu popup
    if (ImGui::BeginPopup("CanvasContextMenu")) {
        if (ImGui::MenuItem("Cut", "Ctrl+X", false, selectedShapeIndex != -1)) {
            cutShape();
        }

        if (ImGui::MenuItem("Copy", "Ctrl+C", false, selectedShapeIndex != -1)) {
            copyShape();
        }

        if (ImGui::MenuItem("Paste", "Ctrl+V", false, clipboardSystem.hasContent())) {
            pasteShape();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Delete", "Del", false, selectedShapeIndex != -1)) {
            deleteShape();
        }

        ImGui::EndPopup();
    }
}

// New method to draw shapes with 3D effect:
void ShapeEditorGUI::draw3DShapes(ImDrawList* draw_list, const ImVec2& canvas_pos) {
    for (const auto& shape_ptr : shapes) {
        // Calculate 3D transformation with both X and Y rotation
        float cosX = cos(rotationAngleX);
        float sinX = sin(rotationAngleX);
        float cosY = cos(rotationAngleY);
        float sinY = sin(rotationAngleY);

        // Get shape center
        ImVec2 shape_center = ImVec2(
            canvas_pos.x + shape_ptr->position.x,
            canvas_pos.y + shape_ptr->position.y
        );

        // Calculate combined 3D effect
        // The Z-depth effect is simulated by scaling and positioning
        float depthScale = 0.7f + 0.3f * (cosX * cosY);  // Combine both rotations for depth
        float shadowOffsetX = 8.0f * sinY;  // Horizontal shadow from Y rotation
        float shadowOffsetY = 6.0f * sinX;  // Vertical shadow from X rotation

        // Draw shadow/depth effect (darker version offset based on both rotations)
        ImU32 shadow_color = IM_COL32(
            (int)(shape_ptr->color[0] * 80),
            (int)(shape_ptr->color[1] * 80),
            (int)(shape_ptr->color[2] * 80),
            (int)(120 * depthScale)  // Shadow opacity varies with depth
        );

        if (auto* circle = dynamic_cast<Circle*>(shape_ptr.get())) {
            // Draw shadow circle
            draw_list->AddCircleFilled(
                ImVec2(shape_center.x + shadowOffsetX, shape_center.y + shadowOffsetY),
                circle->radius * depthScale * 0.9f,
                shadow_color
            );

            // Draw main circle with 3D scaling
            ImU32 main_color = IM_COL32(
                (int)(shape_ptr->color[0] * 255),
                (int)(shape_ptr->color[1] * 255),
                (int)(shape_ptr->color[2] * 255),
                255
            );

            draw_list->AddCircleFilled(
                shape_center,
                circle->radius * depthScale,
                main_color
            );

            // Add dynamic highlight based on rotation
            float highlightIntensity = 0.3f + 0.4f * (cosX + cosY) * 0.5f;
            ImU32 highlight_color = IM_COL32(255, 255, 255, (int)(150 * highlightIntensity));

            // Highlight position moves based on rotation
            float highlightOffsetX = circle->radius * 0.3f * cosY;
            float highlightOffsetY = circle->radius * 0.3f * cosX;

            draw_list->AddCircleFilled(
                ImVec2(shape_center.x - highlightOffsetX, shape_center.y - highlightOffsetY),
                circle->radius * 0.15f * depthScale,
                highlight_color
            );
        }
        else if (auto* rect = dynamic_cast<Rectangle*>(shape_ptr.get())) {
            // Calculate rotated rectangle corners with dual-axis rotation
            float half_width = rect->size.x * 0.5f;
            float half_height = rect->size.y * 0.5f;

            // 3D perspective scaling
            float scaleX = depthScale * (0.8f + 0.2f * cosY);
            float scaleY = depthScale * (0.8f + 0.2f * cosX);

            // Create rectangle corners
            ImVec2 corners[4] = {
                ImVec2(-half_width * scaleX, -half_height * scaleY),
                ImVec2(half_width * scaleX, -half_height * scaleY),
                ImVec2(half_width * scaleX, half_height * scaleY),
                ImVec2(-half_width * scaleX, half_height * scaleY)
            };

            // Apply 3D rotation transformation
            for (int i = 0; i < 4; i++) {
                // First rotate around Y axis (left-right)
                float tempX = corners[i].x * cosY - corners[i].y * sinY * 0.3f;  // Reduced Z-effect
                float tempZ = corners[i].x * sinY + corners[i].y * cosY * 0.3f;

                // Then rotate around X axis (up-down)
                float finalY = corners[i].y * cosX - tempZ * sinX;

                corners[i] = ImVec2(
                    shape_center.x + tempX,
                    shape_center.y + finalY
                );
            }

            // Draw shadow
            ImVec2 shadow_corners[4];
            for (int i = 0; i < 4; i++) {
                shadow_corners[i] = ImVec2(
                    corners[i].x + shadowOffsetX,
                    corners[i].y + shadowOffsetY
                );
            }
            draw_list->AddConvexPolyFilled(shadow_corners, 4, shadow_color);

            // Draw main rectangle
            ImU32 main_color = IM_COL32(
                (int)(shape_ptr->color[0] * 255),
                (int)(shape_ptr->color[1] * 255),
                (int)(shape_ptr->color[2] * 255),
                255
            );
            draw_list->AddConvexPolyFilled(corners, 4, main_color);

            // Add dynamic highlight
            float highlightIntensity = 0.4f + 0.3f * (cosX + cosY) * 0.5f;
            ImU32 highlight_color = IM_COL32(255, 255, 255, (int)(100 * highlightIntensity));

            ImVec2 highlight_corners[4];
            for (int i = 0; i < 4; i++) {
                highlight_corners[i] = ImVec2(
                    corners[i].x + (shape_center.x - corners[i].x) * 0.6f,
                    corners[i].y + (shape_center.y - corners[i].y) * 0.6f
                );
            }
            draw_list->AddConvexPolyFilled(highlight_corners, 4, highlight_color);
        }
    }
}

// Modified renderCanvasPanel() method:
void ShapeEditorGUI::renderCanvasPanel() {
    ImGui::Text("Drawing Canvas");
    if (is3DMode) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "(3D Mode Active)");
    }
    ImGui::Separator();

    // The canvas panel now occupies the full available space within its BeginChild container
    ImVec2 canvas_pos = ImGui::GetCursorScreenPos(); // Top-left of the canvas area in screen coordinates
    ImVec2 canvas_size = ImGui::GetContentRegionAvail(); // Available space for canvas
    if (canvas_size.x < 50.0f) canvas_size.x = 50.0f;
    if (canvas_size.y < 50.0f) canvas_size.y = 50.0f;

    // Draw a background for the canvas
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(50, 50, 50, 255));
    draw_list->AddRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(255, 255, 255, 255)); // Border

    // IMPORTANT: Invisible button to capture mouse input over the canvas
    ImGui::InvisibleButton("Canvas", canvas_size);
    bool is_canvas_hovered = ImGui::IsItemHovered();
    bool is_canvas_active = ImGui::IsItemActive();   // Checks if the invisible button (canvas) is clicked/active

    // Mouse position relative to the ImGui window
    ImVec2 mouse_pos_absolute = ImGui::GetIO().MousePos;
    // Mouse position relative to the canvas's top-left corner
    ImVec2 mouse_pos_in_canvas = ImVec2(mouse_pos_absolute.x - canvas_pos.x, mouse_pos_absolute.y - canvas_pos.y);

    // Only handle mouse interactions in 2D mode
    if (!is3DMode) {
        // --- Cursor logic for shapes ---
        handleMouseShape(is_canvas_hovered, mouse_pos_in_canvas);

        //  --- Right click context menu for Shapes ---
        renderCanvasContextMenu(is_canvas_hovered);

        // Handle shape selection and dragging
        if (is_canvas_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            bool foundSelection = false;
            // Iterate shapes in reverse order to select top-most shape
            for (int i = shapes.size() - 1; i >= 0; --i) {
                if (shapes[i]->contains(mouse_pos_in_canvas)) {
                    if (selectedShapeIndex != -1) {
                        shapes[selectedShapeIndex]->isSelected = false;
                    }
                    selectedShapeIndex = i;
                    shapes[selectedShapeIndex]->isSelected = true;
                    foundSelection = true;
                    break;
                }
            }
            if (!foundSelection) { // If no shape was clicked, deselect current one
                if (selectedShapeIndex != -1) {
                    shapes[selectedShapeIndex]->isSelected = false;
                }
                selectedShapeIndex = -1;
            }
        }

        // Handle shape dragging
        if (selectedShapeIndex != -1 && is_canvas_active && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            shapes[selectedShapeIndex]->clampPosition(canvas_size);
        }
    } else {
        // In 3D mode, deselect any selected shapes and show different cursor
        if (selectedShapeIndex != -1) {
            shapes[selectedShapeIndex]->isSelected = false;
            selectedShapeIndex = -1;
        }

        if (is_canvas_hovered) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
        }
    }

    // Draw all shapes (with 3D effect if enabled)
    if (is3DMode) {
        draw3DShapes(draw_list, canvas_pos);
    } else {
        for (const auto& shape_ptr : shapes) {
            shape_ptr->draw(draw_list, canvas_pos); // Pass canvas_pos for correct drawing
        }
    }
}

template<typename T, typename... Args>
void ShapeEditorGUI::addShape(Args&&... args) {
    std::string shapeName(newShapeNameBuffer);  // Create string
    shapes.push_back(std::make_unique<T>(std::forward<Args>(args)... , newShapeColor, std::move(shapeName)));
    // Reset name buffer after adding
    newShapeNameBuffer[0] = '\0';
    // Select the newly added shape
    if (selectedShapeIndex != -1) {
        shapes[selectedShapeIndex]->isSelected = false;
    }
    selectedShapeIndex = shapes.size() - 1;
    shapes[selectedShapeIndex]->isSelected = true;
}

void ShapeEditorGUI::deleteShape() {
    if (selectedShapeIndex == -1) return;
    shapes.erase(shapes.begin() + selectedShapeIndex);
    selectedShapeIndex = -1;
}

void ShapeEditorGUI::copyShape()
{
    if (selectedShapeIndex == -1) return;

    // Use the clipboard's single generic copy method
    clipboardSystem.copyShape(shapes[selectedShapeIndex].get());
}

void ShapeEditorGUI::pasteShape()
{
    auto newShape = clipboardSystem.createPastedShape();
    if (!newShape) {
        return;
    }

    // Deselect the current obj if any
    if (selectedShapeIndex != -1 )
        shapes[selectedShapeIndex]->isSelected = false;

    // Push and select the new object
    shapes.push_back(std::move(newShape));
    selectedShapeIndex = shapes.size() - 1;
    shapes[selectedShapeIndex]->isSelected = true;
}

void ShapeEditorGUI::cutShape()
{
    if (selectedShapeIndex == -1) return;

    // Copy the clipboard first
    copyShape();

    // Then delete the original
    deleteShape();
}

void ShapeEditorGUI::importJson()
{

}

void ShapeEditorGUI::exportJson()
{

}