#include "gui/rectangle.h"
#include "gui/circle.h"
#include "gui/shape_editor_gui.h"
#include "gui/shape_editor_application.h"

#include <iostream>
// Define M_PI if not available (e.g., on MSVC by default)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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
