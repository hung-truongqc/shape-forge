// --- Simple Application class to manage GLFW and ImGui lifecycle ---
#pragma once
#include <iostream>

#include "shape_editor_gui.h"
class ShapeEditorApplication {
private:
    GLFWwindow* window;
    ShapeEditorGUI editorGUI;

public:
    bool initialize();

    void run();

    void cleanup();
};