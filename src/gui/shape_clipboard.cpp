#include "shape_clipboard.h"

void ShapeClipboard::copyShape(const Shape* shape) {
    if (!shape) {
        // Clear the clipboard if a null pointer is passed
        clipboardShape.reset();
        return;
    }
    // Use the virtual clone() method to create a deep copy
    clipboardShape = shape->clone();
}

bool ShapeClipboard::hasContent() const {
    return clipboardShape != nullptr;
}

bool ShapeClipboard::isEmpty() const {
    return !hasContent();
}

std::unique_ptr<Shape> ShapeClipboard::createPastedShape(const ImVec2& offset) const {
    if (!hasContent()) {
        return nullptr;
    }

    // Create a new copy of the shape from the clipboard
    std::unique_ptr<Shape> newShape = clipboardShape->clone();
    
    // Adjust its position and name
    newShape->position.x += offset.x;
    newShape->position.y += offset.y;
    newShape->name += " (Copy)";

    return newShape;
}

void ShapeClipboard::clear() {
    clipboardShape.reset(); // Unique_ptr automatically handles memory deallocation
}