//========================================================================
// Copyright (c) 2025 hung-truong

#include "shape_editor_gui.h"
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
    ImGui::Begin("Simple Shape Editor", nullptr,
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

void ShapeEditorGUI::renderControlsPanel()
{
    ImGui::Text("Shape Creation");
    ImGui::Separator();

    ImGui::InputText("Name###New", newShapeNameBuffer, sizeof(newShapeNameBuffer));
    // Edit color as RGB
    ImGui::ColorEdit3("Color###New", newShapeColor.data()); // Convert to float* raw pointer for IMGUI

    ImGui::Separator();

    // Circle creation
    ImGui::Text("Circle Properties:");
    ImGui::SliderFloat("Radius", &newCircleRadius, 10.0f, 150.0f, "%.1f");
    // These buttons now add shapes at a default position if not clicked on canvas
    if (ImGui::Button("Add Circle", ImVec2(120, 0))) {
        addShape<Circle>(ImVec2(100,100), newCircleRadius); // Default pos
    }

    ImGui::Separator();

    // Rectangle creation
    ImGui::Text("Rectangle Properties:");
    ImGui::SliderFloat2("Size", (float*)&newRectSize, 10.0f, 200.0f, "%.1f");
    if (ImGui::Button("Add Rectangle", ImVec2(120, 0))) {
        addShape<Rectangle>(ImVec2(150,150), newRectSize); // Default pos
    }

    ImGui::Separator();

    // Shape List and Properties
    ImGui::Text("Shapes (%zu):", shapes.size());
    // Use ImGui::GetContentRegionAvail().y to make the child window fill remaining vertical space
    // Subtract space for the "Quit Application" button and its spacing
    float remaining_height_for_list = ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing() - ImGui::GetStyle().ItemSpacing.y;
    ImGui::BeginChild("ShapeList", ImVec2(0, remaining_height_for_list), true);
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
            if (selectedShapeIndex != -1) {
                shapes[selectedShapeIndex]->isSelected = false; // Deselect previous
            }
            selectedShapeIndex = i;
            shapes[selectedShapeIndex]->isSelected = true; // Select new
        }

        if (isCurrentSelected) {
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

void ShapeEditorGUI::handleShapeDragging(const ImVec2& canvas_size)
{   
    // No shape is selected
    if (selectedShapeIndex == -1) return;

    auto& shapeObject = shapes[selectedShapeIndex];
    ImVec2 newPosition = ImVec2(
        shapeObject->position.x + ImGui::GetIO().MouseDelta.x,
        shapeObject->position.y + ImGui::GetIO().MouseDelta.y
    );

    // Specific properties for Circle
    if (auto* circle = dynamic_cast<Circle*>(shapeObject.get())) {
         // For circles, clamp the center position considering radius
        newPosition.x = std::max(circle->radius, std::min(canvas_size.x - circle->radius, newPosition.x));
        newPosition.y = std::max(circle->radius, std::min(canvas_size.y - circle->radius, newPosition.y));
    }
    // Specific properties for Rectangle
    else if (auto* rect = dynamic_cast<Rectangle*>(shapeObject.get())) {
        // For rectangles, clamp the top-left position considering size
        newPosition.x = std::max(0.0f, std::min(canvas_size.x - rect->size.x, newPosition.x));
        newPosition.y = std::max(0.0f, std::min(canvas_size.y - rect->size.y, newPosition.y));
    }
    else
    {
        newPosition.x = std::max(0.0f, std::min(canvas_size.x, newPosition.x));
        newPosition.y =  std::max(0.0f, std::min(canvas_size.y, newPosition.y));
    }
      // Apply the clamped position
    shapeObject->position = newPosition;

}

void ShapeEditorGUI::renderCanvasPanel() {
        ImGui::Text("Drawing Canvas");
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

        // --- Cursor logic for shapes ---
        handleMouseShape(is_canvas_hovered, mouse_pos_in_canvas);

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
        // Only drag if a shape is selected, the canvas is active (meaning the invisible button was clicked and held),
        // AND the mouse is currently dragging.
        // ImGui::IsItemActive() is crucial here: it will only be true if the "Canvas" invisible button is the active item
        // (i.e., the mouse was pressed down over it). This prevents dragging when interacting with other widgets.
        if (selectedShapeIndex != -1 && is_canvas_active && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            handleShapeDragging(canvas_size);
        }

        // Draw all shapes
        for (const auto& shape_ptr : shapes) {
            shape_ptr->draw(draw_list, canvas_pos); // Pass canvas_pos for correct drawing
        }
    }

template<typename T, typename... Args>
void ShapeEditorGUI::addShape(Args&&... args) {
    shapes.push_back(std::make_unique<T>(std::forward<Args>(args)... , newShapeColor, newShapeNameBuffer));
    // Reset name buffer after adding
    newShapeNameBuffer[0] = '\0';
    // Select the newly added shape
    if (selectedShapeIndex != -1) {
        shapes[selectedShapeIndex]->isSelected = false;
    }
    selectedShapeIndex = shapes.size() - 1;
    shapes[selectedShapeIndex]->isSelected = true;
}

void ShapeEditorGUI::importJson() 
{

}

void ShapeEditorGUI::exportJson()
{

}