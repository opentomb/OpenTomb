#include <cassert>
#include <cstring>

#include "vertex_array.h"

/*
 * Definitions for VAO-based vertex array manager.
 */
class vao_vertex_array;

class vao_vertex_array_manager : public vertex_array_manager {
    GLuint currentVertexArrayObject;
    
public:
    vao_vertex_array_manager();
    virtual vertex_array *createArray(GLuint element_vbo, size_t numAttributes, struct vertex_array_attribute *attributes);
    virtual void unbind();
    
    friend class vao_vertex_array;
};

class vao_vertex_array : public vertex_array {
    GLuint vertexArrayObject;
    vao_vertex_array_manager *manager;
    
public:
    vao_vertex_array(vao_vertex_array_manager *manager, GLuint element_vbo, size_t numAttributes, struct vertex_array_attribute *attributes);
    virtual ~vao_vertex_array();
    
    virtual void use();
};

/*
 * Definitions for manual vertex array manager, where all vertex attributes are set by hand.
 */
class manual_vertex_array;

class manual_vertex_array_manager : public vertex_array_manager {
    uint64_t activeAttribsBitset;
    manual_vertex_array *activeArray;
    GLuint currentArrayBuffer;
    
public:
    manual_vertex_array_manager();
    virtual vertex_array *createArray(GLuint element_vbo, size_t numAttributes, struct vertex_array_attribute *attributes);
    virtual void unbind();
    
    friend class manual_vertex_array;
};

class manual_vertex_array : public vertex_array {
    GLuint elementBuffer;
    size_t numAttributes;
    struct vertex_array_attribute *attributes;
    manual_vertex_array_manager *manager;
    
public:
    manual_vertex_array(manual_vertex_array_manager *manager, GLuint element_vbo, size_t numAttributes, struct vertex_array_attribute *attributes);
    virtual ~manual_vertex_array();
    
    virtual void use();
};

/*** Implementation choosing. ***/

vertex_array_manager * vertex_array_manager::createManager()
{
    if (glGenVertexArrays)
    {
        return new vao_vertex_array_manager();
    }
    else
    {
        return new manual_vertex_array_manager();
    }
}

/*** VAO implementation. ***/

vao_vertex_array_manager::vao_vertex_array_manager()
: currentVertexArrayObject(0)
{}

vertex_array *vao_vertex_array_manager::createArray(GLuint element_vbo, size_t numAttributes, struct vertex_array_attribute *attributes)
{
    return new vao_vertex_array(this, element_vbo, numAttributes, attributes);
}

void vao_vertex_array_manager::unbind()
{
    glBindVertexArray(0);
    currentVertexArrayObject = 0;
}

vao_vertex_array::vao_vertex_array(vao_vertex_array_manager *manager, GLuint element_vbo, size_t numAttributes, struct vertex_array_attribute *attributes)
{
    this->manager = manager;
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);
    
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, element_vbo);
    
    for (size_t i = 0; i < numAttributes; i++)
    {
        assert(attributes[i].vbo != 0);
        assert(attributes[i].stride != 0);
        
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, attributes[i].vbo);
        glEnableVertexAttribArrayARB(attributes[i].index);
        glVertexAttribPointerARB(attributes[i].index, attributes[i].size,
                                 attributes[i].type, attributes[i].normalized,
                                 attributes[i].stride,
                                 (const GLvoid *) attributes[i].offset);
    }
    
    glBindVertexArray(0);
}

void vao_vertex_array::use()
{
    if (manager->currentVertexArrayObject != vertexArrayObject)
    {
        glBindVertexArray(vertexArrayObject);
        manager->currentVertexArrayObject = vertexArrayObject;
    }
}

vao_vertex_array::~vao_vertex_array()
{
    glDeleteVertexArrays(1, &vertexArrayObject);
}

/*** Manual implementation. ***/

manual_vertex_array_manager::manual_vertex_array_manager()
: activeAttribsBitset(0), activeArray(0)
{}

vertex_array *manual_vertex_array_manager::createArray(GLuint element_vbo, size_t numAttributes, struct vertex_array_attribute *attributes)
{
    return new manual_vertex_array(this, element_vbo, numAttributes, attributes);
}

void manual_vertex_array_manager::unbind()
{
    for (int i = 0; i < 64; i++) {
        if (activeAttribsBitset & (1 << i))
            glDisableVertexAttribArrayARB(i);
    }
    activeArray = 0;
    activeAttribsBitset = 0;
    currentArrayBuffer = 0;
}

manual_vertex_array::manual_vertex_array(manual_vertex_array_manager *manager, GLuint element_vbo, size_t numAttributes, struct vertex_array_attribute *attributes)
{
    this->manager = manager;
    elementBuffer = element_vbo;
    this->numAttributes = numAttributes;
    this->attributes = new vertex_array_attribute[numAttributes];
    memcpy(this->attributes, attributes, sizeof(vertex_array_attribute) * numAttributes);
}

void manual_vertex_array::use()
{
    uint64_t toDisable = manager->activeAttribsBitset;
    manager->activeAttribsBitset = 0;
    
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, elementBuffer);
    
    for (size_t i = 0; i < numAttributes; i++)
    {
        if (attributes[i].vbo != manager->currentArrayBuffer)
            glBindBufferARB(GL_ARRAY_BUFFER_ARB, attributes[i].vbo);
        if (!(manager->activeAttribsBitset & (1 << attributes[i].index)))
            glEnableVertexAttribArrayARB(i);
        toDisable &= ~(1 << attributes[i].index);
        glVertexAttribPointerARB(attributes[i].index, attributes[i].size,
                                 attributes[i].type, attributes[i].normalized,
                                 attributes[i].stride,
                                 (const GLvoid *) attributes[i].offset);
    }
    
    for (size_t i = 0; i < 64; i++) {
        if (toDisable & (1 << i))
            glDisableClientState(i);
    }
    
    manager->activeArray = this;
}

manual_vertex_array::~manual_vertex_array()
{
    numAttributes = 0;
    delete [] attributes;
}
