#ifndef _RedBlack_H
#define _RedBlack_H

#include <stdlib.h>
#include <stdint.h>

#define RBC_RED 1
#define RBC_BLACK 2

typedef struct RedBlackNode_s
{
    struct RedBlackNode_s       *parent;
    struct RedBlackNode_s       *left;
    struct RedBlackNode_s       *right;
    int8_t                       color;
    uint16_t                    *height;
    
    void                        *data;
    void                        *key;
} *RedBlackNode_p;

typedef struct RedBlackHeader_s
{     
    RedBlackNode_p              root;                 /* header pointer */
    RedBlackNode_p              last_founded;         /* increases search speed if we askone element twice and more */
    uint32_t                    node_count;           /* node count in tree */
    uint16_t                    height;               /* tree's height */
    int (*rb_compLT)(void *key1, void *key2);
    int (*rb_compEQ)(void *key1, void *key2);
    void (*rb_free_data)(void *data);
} RedBlackHeader_t, *RedBlackHeader_p;

void rotateLeft(RedBlackHeader_p header, RedBlackNode_p x);
void rotateRight(RedBlackHeader_p header, RedBlackNode_p x);
RedBlackNode_p TreeSuccessor(RedBlackNode_p n);

RedBlackNode_p RB_SearchNode(void *key, RedBlackHeader_p header);
RedBlackNode_p RB_InsertIgnore(void *key, void *data, RedBlackHeader_p header);
RedBlackNode_p RB_InsertReplace(void *key, void *data, RedBlackHeader_p header);
void RB_Delete(RedBlackHeader_p p, RedBlackNode_p z);
void RB_MakeEmpty(RedBlackHeader_p header);
void RB_Free(RedBlackHeader_p p);
RedBlackHeader_p RB_Init();

#endif  /* _RedBlack_H */

/* END */
