#pragma once

/*!
 * This class encapsulates a set of vertex state. Depending on whether the 
 * current OpenGL context supports it or not (at creation time), this is
 * stored in a VAO, or not.
 */

#include "gl_util.h"

/*!
 * Description of a single vertex array attribute.
 */
struct VertexArrayAttribute {
    /*! The number of the attribute, used to match it to the shader. */
    size_t m_index;
    /*! The number of components of the attribute. */
    size_t m_size;
    /*! The type of the attribute. */
    GLenum m_type;
    /*! Only for integer types: Whether the attribute is normalized to [0; 1]
     * range (unsigned) or [-1; 1] range (signed). */
    bool m_normalized;
    /*! The VBO in which the attribute is stored. It is not legal to delete this
     * vbo while it is referenced here, and it will not be deleted when the VAO
     * is. */
    GLuint m_vbo;
    /*! The stride of the attribute in bytes. 0 is not allowed. */
    size_t m_stride;
    /*! The start offset of the attribute within its vbo in bytes. */
    size_t m_offset;
    
    VertexArrayAttribute() = default;
    VertexArrayAttribute(unsigned index, unsigned size, GLenum type, bool normalized, GLuint vbo, size_t stride, size_t offset)
        : m_index(index), m_size(size), m_type(type), m_normalized(normalized), m_vbo(vbo), m_stride(stride), m_offset(offset)
    {}
};

class VertexArray {
    GLuint m_vertexArrayObject = 0;

public:
    VertexArray(GLuint element_vbo, size_t numAttributes, const VertexArrayAttribute *attributes);
    ~VertexArray();

    void bind();
};
