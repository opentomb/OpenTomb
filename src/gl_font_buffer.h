#pragma once
//
//  font_buffer.h
//  OpenTomb
//
//  Created by Torsten Kammer on 18.06.15.
//  Contains code to manage VBO and VAO for the font renderer, presenting
//  a standard C interface.
//

#include <GL/glew.h>

/*!
 * Sets the minimum size of the font buffer that is needed, in bytes, and
 * returns a write-only pointer to the new data.
 *
 * At least the given amount of bytes will be available after. The initial
 * contents will be undefined.
 *
 * If the buffer's internal structures don't exist yet, they are created.
 *
 * Nothing may be read from the pointer, not even data that was previously
 * written! The writes (may, depending on driver) go more ore less
 * directly to the GPU, and the reverse path may not be set up, so reading
 * will result in undefined behavior.
 */
GLfloat* FontBuffer_ResizeAndMap(size_t bytes);

/*!
 * Stops mapping. After this is called, the pointer created by MapForWriting
 * may no longer be used. Must be called before any other operation on the
 * buffer, including draws from it, or before some other VBO gets mapped.
 */
void FontBuffer_Unmap();

/*!
 * Sets the buffer as the current draw buffer. Must not be called between
 * ResizeAndMap and Unmap. Creates the internal structures if necessary.
 */
void FontBuffer_Bind();
