//
//  font_buffer.cpp
//  OpenTomb
//
//  Created by Torsten Kammer on 18.06.15.
//

#include "gl_font_buffer.h"

#include "render/render.h"
#include "render/shader_description.h"
#include "render/vertex_array.h"

namespace
{
    GLuint fontBufferVBO = 0;
    std::unique_ptr<render::VertexArray> fontBufferVAO;
    size_t currentSize = 0;
}

void fontBuffer_ensureAvailable()
{
    if(fontBufferVAO && fontBufferVBO != 0)
    {
        return;
    }

    glGenBuffers(1, &fontBufferVBO);

    render::VertexArrayAttribute attribs[] = {
        render::VertexArrayAttribute(render::TextShaderDescription::vertex_attribs::position, 2, GL_FLOAT, false, fontBufferVBO, sizeof(GLfloat [8]), 0),
        render::VertexArrayAttribute(render::TextShaderDescription::vertex_attribs::tex_coord, 2, GL_FLOAT, false, fontBufferVBO, sizeof(GLfloat [8]), sizeof(GLfloat [2])),
        render::VertexArrayAttribute(render::TextShaderDescription::vertex_attribs::color, 4, GL_FLOAT, false, fontBufferVBO, sizeof(GLfloat [8]), sizeof(GLfloat [4])),
    };

    fontBufferVAO.reset(new render::VertexArray(0, 3, attribs));
}

GLfloat *FontBuffer_ResizeAndMap(size_t bytes)
{
    fontBuffer_ensureAvailable();

    glBindBuffer(GL_ARRAY_BUFFER, fontBufferVBO);
    if(bytes > currentSize)
    {
        currentSize = bytes;
    }
    glBufferData(GL_ARRAY_BUFFER, currentSize, nullptr, GL_STREAM_DRAW);
    return static_cast<GLfloat*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
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
