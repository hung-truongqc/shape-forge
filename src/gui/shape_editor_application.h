//========================================================================
// Copyright (c) 2025 hung-truong
// --- Simple Application class to manage GLFW and ImGui lifecycle ---

#pragma once
#include <iostream>

#include "shape_editor_gui.h"
#include "log/interface_logger.h"

class ShapeEditorApplication {
private:
    GLFWwindow* window;
    ShapeEditorGUI editorGUI;


    // logger is non-owning
    ILogger* logger_;

public:

    explicit ShapeEditorApplication(ILogger* logger) 
        : logger_(logger), editorGUI(logger) {}

    bool initialize();

    void run();

    void cleanup();
};