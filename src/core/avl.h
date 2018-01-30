#ifndef AVL_TREE_H
#define AVL_TREE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>


typedef struct avl_node_s
{
    struct avl_node_s      *parent;
    struct avl_node_s      *left;
    struct avl_node_s      *right;
    struct avl_node_s      *next;
    struct avl_node_s      *prev;
    int32_t                 height;
    uint32_t                key;
    void                   *data;
} avl_node_t, *avl_node_p;

typedef struct avl_header_s
{     
    struct avl_node_s          *root;                 /* header pointer */
    struct avl_node_s          *list;
    uint32_t                    nodes_count;          /* node count in tree */
    void (*free_data)(void *data);
} avl_header_t, *avl_header_p;

avl_header_p AVL_Init();
void AVL_Free(avl_header_p p);

avl_node_p AVL_NewNode(uint32_t key, void *data);
avl_node_p AVL_SearchNode(avl_header_p header, uint32_t key);
avl_node_p AVL_InsertReplace(avl_header_p header, uint32_t key, void *data);
void AVL_DeleteNode(avl_header_p header, avl_node_p node);
void AVL_MakeEmpty(avl_header_p header);

#endif  /* AVL_TREE_H */

#ifdef	__cplusplus
}
#endif

/* END */
