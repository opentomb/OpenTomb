#ifndef __OpenTomb__vertex_array__
#define __OpenTomb__vertex_array__

/*!
 * This class encapsulates a set of vertex state. Depending on whether the 
 * current OpenGL context supports it or not (at creation time), this is
 * stored in a VAO, or not.
 */

#include "gl_util.h"

class vertex_array;

/*!
 * Description of a single vertex array attribute.
 */
struct vertex_array_attribute {
    /*! The number of the attribute, used to match it to the shader. */
    unsigned index;
    /*! The number of components of the attribute. */
    unsigned size;
    /*! The type of the attribute. */
    GLenum type;
    /*! Only for integer types: Whether the attribute is normalized to [0; 1]
     * range (unsigned) or [-1; 1] range (signed). */
    bool normalized;
    /*! The VBO in which the attribute is stored. It is not legal to delete this
     * vbo while it is referenced here, and it will not be deleted when the VAO
     * is. */
    GLuint vbo;
    /*! The stride of the attribute in bytes. 0 is not allowed. */
    size_t stride;
    /*! The start offset of the attribute within its vbo in bytes. */
    size_t offset;
    
    vertex_array_attribute() = default;
    vertex_array_attribute(unsigned index, unsigned size, GLenum type, bool normalized, GLuint vbo, size_t stride, size_t offset)
    : index(index), size(size), type(type), normalized(normalized), vbo(vbo), stride(stride), offset(offset)
    {}
};

/*!
 * The vertex array manager is in charge of creating new vertex arrays and
 * switching between them.
 */
class VertexArrayManager {
public:
    /*!
     * Creates a new vertex array manager that matches the current renderer's
     * hardware capabilities.
     */
    static VertexArrayManager *createManager();
    
    /*!
     * Creates a new vertex array object for the given elements and attributes.
     */
    virtual vertex_array *createArray(GLuint element_vbo, size_t numAttributes, struct vertex_array_attribute *attributes) = 0;
};

/*!
 * An individual vertex array, containing the logic necessary to setup the
 * vertex state. This class is abstract; a concrete version is created by the
 * vertex array manager based on the current hardware's capabilities.
 *
 * A vertex array can be deleted, but only if it is not currently bound.
 * Otherwise results are undefined (even if it is not used for drawing
 * afterwards).
 */
class vertex_array {
public:
    /*!
     * Sets the receiving vertex array to be the current one for all rendering
     * on the context, until the next one is set. This method does a limited
     * amount of checks to make sure no duplicate work is being done.
     */
    virtual void use() = 0;
    virtual ~vertex_array() {}
};

#endif /* defined(__OpenTomb__vertex_array__) */
