//========================================================================
// Copyright (c) 2025 hung-truong
#include "gui/rectangle.h"
#include "gui/circle.h"
#include "gui/shape_editor_gui.h"
#include "gui/shape_editor_application.h"
#include <iostream>

// --- Main Entry Point ---
int main() {
    ShapeEditorApplication app;

    if (!app.initialize()) {
        return -1;
    }

    app.run();
    app.cleanup();

    return 0;
}
