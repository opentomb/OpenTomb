//
//  font_buffer.cpp
//  OpenTomb
//
//  Created by Torsten Kammer on 18.06.15.
//

#include "gl_font_buffer.h"

#include "gl_util.h"
#include "render.h"
#include "vertex_array.h"
#include "shader_description.h"

namespace
{
    GLuint fontBufferVBO = 0;
    std::unique_ptr<VertexArray> fontBufferVAO;
    size_t currentSize = 0;
}

void fontBuffer_ensureAvailable()
{
    if (fontBufferVAO && fontBufferVBO != 0)
    {
        return;
    }
    
    glGenBuffers(1, &fontBufferVBO);
    
    VertexArrayAttribute attribs[] = {
        VertexArrayAttribute(TextShaderDescription::vertex_attribs::position, 2, GL_FLOAT, false, fontBufferVBO, sizeof(GLfloat [8]), 0),
        VertexArrayAttribute(TextShaderDescription::vertex_attribs::tex_coord, 2, GL_FLOAT, false, fontBufferVBO, sizeof(GLfloat [8]), sizeof(GLfloat [2])),
        VertexArrayAttribute(TextShaderDescription::vertex_attribs::color, 4, GL_FLOAT, false, fontBufferVBO, sizeof(GLfloat [8]), sizeof(GLfloat [4])),
    };
    
    fontBufferVAO.reset( new VertexArray(0, 3, attribs) );
}

GLfloat *FontBuffer_ResizeAndMap(size_t bytes)
{
    fontBuffer_ensureAvailable();
    
    glBindBuffer(GL_ARRAY_BUFFER, fontBufferVBO);
    if (bytes > currentSize)
    {
        currentSize = bytes;
    }
    glBufferData(GL_ARRAY_BUFFER, currentSize, 0, GL_STREAM_DRAW);
    return static_cast<GLfloat*>( glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY) );
}

void FontBuffer_Unmap()
{
    glUnmapBuffer(GL_ARRAY_BUFFER);
}

void FontBuffer_Bind()
{
    fontBuffer_ensureAvailable();
    
    fontBufferVAO->bind();
}
