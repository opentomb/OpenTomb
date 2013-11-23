#ifndef _RedBlack_H
#define _RedBlack_H

#include <stdlib.h>
#include <stdint.h>
//typedef int DataType;
//#define compEQ(x, y) ((x) == (y))
//#define compLT(x, y) ((x) < (y))

#define RBC_RED 1
#define RBC_BLACK 2

typedef struct RedBlackNode_s
{
    struct RedBlackNode_s       *parent;
    struct RedBlackNode_s       *left;
    struct RedBlackNode_s       *right;
    int8_t                       color;
    void                        *data;
} *RedBlackNode_p;

typedef struct RedBlackHeader_s
{     
    RedBlackNode_p              root;                 /* указатель на заголовок */
    unsigned int                node_count;           /* число узлов в дереве */
    unsigned int                height;               /* Высота дерева */
    int (*rb_compLT)(void *data1, void *data2);
    int (*rb_compEQ)(void *data1, void *data2);
    void (*rb_free_data)(void *data);
} RedBlackHeader_t, *RedBlackHeader_p;

void rotateLeft(RedBlackHeader_p header, RedBlackNode_p x);
void rotateRight(RedBlackHeader_p header, RedBlackNode_p x);
void updateHeight(RedBlackHeader_p header);
RedBlackNode_p TreeSuccessor(RedBlackNode_p n);

RedBlackNode_p RB_SearchNode(void *data, RedBlackHeader_p header);
RedBlackNode_p RB_InsertIgnore(void *data, RedBlackHeader_p header);
RedBlackNode_p RB_InsertReplace(void *data, RedBlackHeader_p header);
void RB_Delete(RedBlackHeader_p p, RedBlackNode_p z);
void RB_MakeEmpty(RedBlackHeader_p header);
void RB_Free(RedBlackHeader_p p);
RedBlackHeader_p RB_Init();

#endif  /* _RedBlack_H */

/* END */
