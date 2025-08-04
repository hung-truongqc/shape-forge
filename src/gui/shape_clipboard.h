//========================================================================
// Copyright (c) 2025 hung-truong

#pragma once
#include "shape.h"
#include <memory>
#include <string>

class ShapeClipboard {
private:
    // This variable hold a single polymorphic Shape object
    // The current design aim to perfect copy/cut/paste/delete on one object at a time first
    // In the future it may extend to handling multiple objects at the same time using vector
    std::unique_ptr<Shape> clipboardShape;

public:
    ShapeClipboard() = default;

    // Single copy method that works for any Shape
    void copyShape(const Shape* shape);

    // Check clipboard state
    bool hasContent() const;
    bool isEmpty() const;

    // Create a new shape from clipboard data using the clone() method
    std::unique_ptr<Shape> createPastedShape(const ImVec2& offset = ImVec2(20.0f, 20.0f)) const;

    // Clear clipboard
    void clear();
};