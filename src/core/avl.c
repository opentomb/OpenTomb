#include <stdio.h>
#include <stdlib.h>
#include "avl.h"


static avl_node_p RotateRight(avl_node_p node);
static avl_node_p RotateLeft(avl_node_p node);
static avl_node_p BalanceNode(avl_node_p node);
static void BalanceTree(avl_header_p header, avl_node_p p);

#define AVL_UPDATE_NODE_HEIGHT(node, t) \
    (node)->height = ((node)->left) ? ((node)->left->height) : 0;\
    (t) = ((node)->right) ? ((node)->right->height) : 0;\
    (node)->height = ((t) > (node)->height) ? (t) : (node)->height;\
    ++(node)->height;\

#define AVL_GET_NODE_FACTOR(node, t) \
    t = (node->right) ? (node->right->height) : (0);\
    t -= ((node->left) ? (node->left->height) : (0));\

avl_header_p AVL_Init()
{
    avl_header_p p = (avl_header_p)malloc(sizeof(struct avl_header_s));
    if(p == NULL)
    {
        return NULL;
    }

    p->free_data = NULL;
    p->root = NULL;
    p->list = NULL;
    p->nodes_count = 0;

    return p;
}


void AVL_Free(avl_header_p p)
{
    AVL_MakeEmpty(p);
    free(p);
}


void AVL_MakeEmpty(avl_header_p header)
{
    if(header)
    {
        for(avl_node_p p = header->list; p; )
        {
            avl_node_p next = p->next;
            if(header->free_data)
            {
                header->free_data(p->data);
            }
            free(p);
            p = next;
        }
        header->nodes_count = 0;
        header->list = NULL;
        header->root = NULL;
    }
}


avl_node_p AVL_NewNode(uint32_t key, void *data)
{
    avl_node_p ret = (avl_node_p)malloc(sizeof(avl_node_t));
    ret->left = NULL;
    ret->right = NULL;
    ret->parent = NULL;
    ret->next = NULL;
    ret->prev = NULL;
    ret->height = 1;
    ret->data = data;
    ret->key = key;
    return ret;
}

// This method search the tree for key, and
// returns the node on success, otherwise NULL.
avl_node_p AVL_SearchNode(avl_header_p header, uint32_t key)
{
    avl_node_p current = header->root;
    while(current && (key != current->key))
    {
        current = (key < current->key) ? current->left : current->right;
    }
    return current;
}


avl_node_p RotateRight(avl_node_p node)
{
    avl_node_p t = node->left;
    int32_t h;
    node->left = t->right;
    if(node->left)
    {
        node->left->parent = node;
    }

    t->right = node;
    t->parent = node->parent;
    node->parent = t;
    AVL_UPDATE_NODE_HEIGHT(node, h)
    AVL_UPDATE_NODE_HEIGHT(t, h)

    return t;
}


avl_node_p RotateLeft(avl_node_p node)
{
    avl_node_p t = node->right;
    int32_t h;
    node->right = t->left;
    if(node->right)
    {
        node->right->parent = node;
    }

    t->left = node;
    t->parent = node->parent;
    node->parent = t;
    AVL_UPDATE_NODE_HEIGHT(node, h)
    AVL_UPDATE_NODE_HEIGHT(t, h)

    return t;
}


avl_node_p BalanceNode(avl_node_p node)
{
    int32_t t;
    AVL_UPDATE_NODE_HEIGHT(node, t)
    AVL_GET_NODE_FACTOR(node, t)
    if(t == 2)
    {
        AVL_GET_NODE_FACTOR(node->right, t)
        if(t < 0)
        {
            node->right = RotateRight(node->right);
            node->right->parent = node;
        }
        return RotateLeft(node);
    }
    if(t == -2)
    {
        AVL_GET_NODE_FACTOR(node->left, t)
        if(t > 0)
        {
            node->left = RotateLeft(node->left);
            node->left->parent = node;
        }
        return RotateRight(node);
    }
    return node;
}


void BalanceTree(avl_header_p header, avl_node_p p)
{
    while(p)
    {
        avl_node_p pp = p->parent;
        if(pp)
        {
            avl_node_p *p_ins = (p == pp->left) ? (&pp->left) : (&pp->right);
            avl_node_p bn = BalanceNode(p);
            if(bn != p)
            {
                *p_ins = bn;
                bn->parent = pp;
            }
        }
        else
        {
            header->root = BalanceNode(p);
            header->root->parent = NULL;
        }
        p = pp;
    }
}


avl_node_p AVL_InsertReplace(avl_header_p header, uint32_t key, void *data)
{
    avl_node_p p = header->root;
    avl_node_p i = NULL;

    if(!header->root)
    {
        header->list = header->root = AVL_NewNode(key, data);
        ++header->nodes_count;
        return header->root;
    }

    while(p)
    {
        if(key < p->key)
        {
            if(!p->left)
            {
                i = p->left = AVL_NewNode(key, data);
                ++header->nodes_count;
                p->left->parent = p;
                break;
            }
            p = p->left;
        }
        else if(p->key < key)
        {
            if(!p->right)
            {
                i = p->right = AVL_NewNode(key, data);
                ++header->nodes_count;
                p->right->parent = p;
                break;
            }
            p = p->right;
        }
        else
        {
            i = p;
            header->free_data(i->data);
            i->data = data;
            i->key = key;
            return i;
        }
    }

    i->next = header->list;
    header->list->prev = i;
    header->list = i;
    BalanceTree(header, p);

    return i;
}


void AVL_DeleteNode(avl_header_p header, avl_node_p node)
{
    if(node)
    {
        int32_t t;
        avl_node_p parent = node->parent;
        avl_node_p *p_ins = &header->root;
        if(parent)
        {
            p_ins = (node == parent->left) ? (&parent->left) : (&parent->right);
        }
        
        if(node->prev)
        {
            node->prev->next = node->next;
        }
        else
        {
            header->list = node->next;
        }
        if(node->next)
        {
            node->next->prev = node->prev;
        }
        
        if(!node->right || !node->left)
        {
            *p_ins = (node->right) ? (node->right) : (node->left);
            if(*p_ins)
            {
                (*p_ins)->parent = parent;
                AVL_UPDATE_NODE_HEIGHT(*p_ins, t)
            }
        }
        else if(!node->right->left)
        {
            node->left->parent = node->right;
            node->right->left = node->left;
            AVL_UPDATE_NODE_HEIGHT(node->right, t)
            *p_ins = node->right;
            (*p_ins)->parent = parent;
        }
        else
        {
            avl_node_p min = node->right;
            while(min->left)
            {
                min = min->left;
            }

            min->parent->left = min->right;
            if(min->right)
            {
                min->right->parent = min->parent;
            }
            AVL_UPDATE_NODE_HEIGHT(min->parent, t)

            min->left = node->left;
            min->left->parent = min;
            min->right = node->right;
            min->right->parent = min;

            min->parent = parent;
            *p_ins = min;
            parent = min;
            AVL_UPDATE_NODE_HEIGHT(min, t)
        }

        --header->nodes_count;
        header->free_data(node->data);
        free(node);
        BalanceTree(header, parent);
    }
}
