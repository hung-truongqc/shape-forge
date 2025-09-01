//========================================================================
// Copyright (c) 2025 hung-truong
#include "gui/rectangle.h"
#include "gui/circle.h"
#include "gui/shape_editor_gui.h"
#include "gui/shape_editor_application.h"
#include "log/logger.h"
#include <iostream>

// --- Main Entry Point ---
int main() {
    // Create the Logger object on the stack.
    // Its constructor initializes spdlog programmatically.
    // When this object goes out of scope at the end of main(),
    // its destructor will automatically call spdlog::shutdown().
    auto logger_instance = Logger::create();

    // Check if creation was successful
    if (!logger_instance) {
        std::cerr << "Failed to create logger system - another instance may already exist" << std::endl;
        return 1;
    }

    // Retrieve a pointer to the logger we created.
    auto main_logger = spdlog::get("shape-forge-logger");
    if (!main_logger) {
        std::cerr << "Failed to get logger 'shape-forge-logger'. Exiting." << std::endl;
        return 1;
    }

    //Create the adapter (wraps spdlog in ILogger interface)
    auto logger_adapter = std::make_unique<SpdlogAdapter>(main_logger);

    // --- Demonstrate Logging ---
    logger_adapter->info("Application starting up...");

    ShapeEditorApplication app(logger_adapter.get());

    if (!app.initialize()) {
        return -1;
    }

    app.run();
    app.cleanup();

    return 0;
}
