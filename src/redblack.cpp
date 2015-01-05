#include <stdio.h>
#include <stdlib.h>
#include "redblack.h"
#include "system.h"

void deleteFixup(RedBlackHeader_p header, RedBlackNode_p x, char x_null);

// This method search the tree for a node with key 'key', and
// return the node on success otherwise treeNULL.
RedBlackNode_p RB_SearchNode(void *key, RedBlackHeader_p header)
{
    RedBlackNode_p current = header->root;

    while(current != NULL)
    {
        if(header->rb_compEQ(key, current->key))                                                       //data1 == data2
        {
            return (current);
        }
        current = (header->rb_compLT(key, current->key)) ? current->left : current->right;             //data1 < data2
    }
    return NULL;
}


void rotateLeft(RedBlackHeader_p header, RedBlackNode_p x)
{

   /**************************
    *  rotate node x to left *
    **************************/
    RedBlackNode_p y = x->right;

    /* establish x->right link */
    x->right = y->left;
    if (y->left != NULL)
    {
        y->left->parent = x;
    }

    /* establish y->parent link */
    if(y != NULL)
    {
        y->parent = x->parent;
    }

    if(x->parent)
    {
        if(x == x->parent->left)
        {
            x->parent->left = y;
        }
        else
        {
            x->parent->right = y;
        }
    }
    else
    {
        header->root = y;
    }

    /* link x and y */
    y->left = x;
    if(x != NULL)
    {
        x->parent = y;
    }
}

void rotateRight(RedBlackHeader_p header, RedBlackNode_p x)
{

   /****************************
    *  rotate node x to right  *
    ****************************/

    RedBlackNode_p y = x->left;

    /* establish x->left link */
    x->left = y->right;
    if(y->right != NULL)
    {
        y->right->parent = x;
    }

    /* establish y->parent link */
    if(y != NULL)
    {
        y->parent = x->parent;
    }

    if(x->parent)
    {
        if(x == x->parent->right)
        {
            x->parent->right = y;
        }
        else
        {
            x->parent->left = y;
        }
    }
    else
    {
        header->root = y;
    }

    /* link x and y */
    y->right = x;
    if (x != NULL) x->parent = y;
}

RedBlackHeader_p RB_Init()
{
    RedBlackHeader_p p = (RedBlackHeader_p)malloc(sizeof(struct RedBlackHeader_s));
    if(p == NULL)
    {
        return NULL;
    }

    p->rb_compEQ = NULL;
    p->rb_compLT = NULL;
    p->rb_free_data = NULL;
    p->root = NULL;
    p->height = 0;
    p->node_count = 0;

    return p;
}

void RB_Free(RedBlackHeader_p p)
{
    RB_MakeEmpty(p);
    free(p);
}

void RB_EmptyTree(RedBlackNode_p n, RedBlackHeader_p p)
{
    if(n->left != NULL)
    {
        RB_EmptyTree(n->left, p);
    }
    if(n->right != NULL)
    {
        RB_EmptyTree(n->right, p);
    }
    if(p->rb_free_data)
    {
        p->rb_free_data(n->data);
    }
    p->node_count--;
}

void RB_MakeEmpty(RedBlackHeader_p header)
{
    if(header && header->root)
    {
        RB_EmptyTree(header->root, header);
    }
}

void insertFixup(RedBlackHeader_p p, RedBlackNode_p x)
{

   /*************************************
    *  maintain Red-Black tree balance  *
    *  after inserting node x           *
    *************************************/

    /* check Red-Black properties */
    while (x != p->root && x->parent != NULL && x->parent->color == RBC_RED)
    {
        /* we have a violation */
        if (x->parent == x->parent->parent->left)
        {
            RedBlackNode_p y = x->parent->parent->right;
            if (y != NULL && y->color == RBC_RED)
            {
                /* uncle is Red */
                x->parent->color = RBC_BLACK;
                y->color = RBC_BLACK;
                x->parent->parent->color = RBC_RED;
                x = x->parent->parent;
            }
            else
            {

                /* uncle is Black */
                if (x == x->parent->right)
                {
                    /* make x a left child */
                    x = x->parent;
                    rotateLeft(p, x);                          //rotateLeft(x);
                }

                /* recolor and rotate */
                x->parent->color = RBC_BLACK;
                x->parent->parent->color = RBC_RED;
                rotateRight(p, x->parent->parent);                              //rotateRight(x->parent->parent)
            }
        }
        else
        {
            /* mirror image of above code */
            RedBlackNode_p y = x->parent->parent->left;
            if (y != NULL && y->color == RBC_RED)
            {
                /* uncle is Red */
                x->parent->color = RBC_BLACK;
                y->color = RBC_BLACK;
                x->parent->parent->color = RBC_RED;
                x = x->parent->parent;
            }
            else
            {
                /* uncle is Black */
                if (x == x->parent->left)
                {
                    x = x->parent;
                    rotateRight(p, x);                                 //rotateRight(x);
                }
                x->parent->color = RBC_BLACK;
                x->parent->parent->color = RBC_RED;
                rotateLeft(p, x->parent->parent);                               //rotateLeft(x->parent->parent);
            }
        }
    }
    p->root->color = RBC_BLACK;
}

RedBlackNode_p RB_InsertIgnore(void *key, void *data, RedBlackHeader_p header)
{
    RedBlackNode_p current, parent, x;

   /***********************************************
    *  allocate node for data and insert in tree  *
    ***********************************************/

    /* find where node belongs */
    current = header->root;
    parent = NULL;
    while(current != NULL)
    {
        if (header->rb_compEQ(key, current->key))
        {
            return NULL;
        }
        parent = current;
        current = (header->rb_compLT(key, current->key)) ? current->left : current->right;
    }

    /* setup new node */
    x = (RedBlackNode_p)malloc(sizeof(struct RedBlackNode_s));
    if(x == NULL)
    {
        Sys_extError("insufficient memory (insertNode)\n");
    }
    x->data = data;
    x->key = key;
    x->parent = parent;
    x->left = NULL;
    x->right = NULL;
    x->color = RBC_RED;                                                         // new node is red

    /* insert node in tree */
    if(parent)
    {
        if(header->rb_compLT(key, parent->key))
        {
            parent->left = x;
        }
        else
        {
            parent->right = x;
        }
    }
    else
    {
        header->root = x;
    }

    insertFixup(header, x);
    header->node_count ++;
    return(x);
}

RedBlackNode_p RB_InsertReplace(void *key, void *data, RedBlackHeader_p header)
{
    RedBlackNode_p current, parent, x;

   /***********************************************
    *  allocate node for data and insert in tree  *
    ***********************************************/

    /* find where node belongs */
    current = header->root;
    parent = NULL;
    while(current != NULL)
    {
        if(header->rb_compEQ(key, current->key))
        {
            void *old_data = current->data;
            current->data = data;
            current->key = key;
            if(header->rb_free_data)
            {
                header->rb_free_data(old_data);
            }
            return NULL;
        }
        parent = current;
        current = (header->rb_compLT(key, current->key)) ? current->left : current->right;
    }

    /* setup new node */
    x = (RedBlackNode_p)malloc(sizeof(struct RedBlackNode_s));
    if(x == NULL)
    {
        Sys_extError("insufficient memory (insertNode)\n");
    }
    x->data = data;
    x->key = key;
    x->parent = parent;
    x->left = NULL;
    x->right = NULL;
    x->color = RBC_RED;                                                         // new node is red

    /* insert node in tree */
    if(parent)
    {
        if(header->rb_compLT(key, parent->key))
        {
            parent->left = x;
        }
        else
        {
            parent->right = x;
        }
    }
    else
    {
        header->root = x;
        //return x;
    }

    insertFixup(header, x);
    header->node_count ++;
    return(x);
}


void deleteFixup(RedBlackHeader_p header, RedBlackNode_p x, char x_null)
{

   /*************************************
    *  maintain Red-Black tree balance  *
    *  after deleting node x            *
    *************************************/

    while(x != header->root && x->color == RBC_BLACK)
    {
        if(x->parent->left == (x_null ? NULL : x))
        {
            RedBlackNode_p w = x->parent->right;
            if(w->color == RBC_RED)
            {
                w->color = RBC_BLACK;
                x->parent->color = RBC_RED;
                rotateLeft (header, x->parent);
                w = x->parent->right;
            }
            if((w->right == NULL || w->right->color == RBC_BLACK) &&
               (w->left == NULL || w->left->color == RBC_BLACK))
            {
                w->color = RBC_RED;
                x = x->parent;
            }
            else
            {
                if(w->right == NULL || w->right->color == RBC_BLACK)
                {
                    w->left->color = RBC_BLACK;
                    w->color = RBC_RED;
                    rotateRight (header, w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = RBC_BLACK;
                w->right->color = RBC_BLACK;
                rotateLeft (header, x->parent);
                x = header->root;
            }
        }
        else
        {
            RedBlackNode_p w = x->parent->left;
            if (w->color == RBC_RED)
            {
                w->color = RBC_BLACK;
                x->parent->color = RBC_RED;
                rotateRight (header, x->parent);
                w = x->parent->left;
            }
            if ((w->right == NULL || w->right->color == RBC_BLACK) &&
                (w->left == NULL || w->left->color == RBC_BLACK))
            {
                w->color = RBC_RED;
                x = x->parent;
            }
            else
            {
                if (w->left == NULL || w->left->color == RBC_BLACK)
                {
                    w->right->color = RBC_BLACK;
                    w->color = RBC_RED;
                    rotateLeft (header, w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = RBC_BLACK;
                w->left->color = RBC_BLACK;
                rotateRight (header, x->parent);
                x = header->root;
            }
        }
        x_null = 0;
    }
    x->color = RBC_BLACK;
}


void RB_Delete(RedBlackHeader_p header, RedBlackNode_p z)
{
    RedBlackNode_p x, y, w = NULL;
    void *data = NULL;
    char x_null = 0;
   /*****************************
    *  delete node z from tree  *
    *****************************/

    if (z == NULL)
    {
        return;
    }

    data = z->data;

    if(header->node_count == 1 && z == header->root)
    {
        header->node_count --;
        if(header->rb_free_data)
        {
            header->rb_free_data(data);
        }
        header->root = NULL;
        return;
    }

    if (z->left == NULL || z->right == NULL)
    {
        /* y has a NULL node as a child */
        y = z;
    }
    else
    {
        /* find tree successor with a NULL node as a child */
        y = z->right;
        while (y->left != NULL) y = y->left;
    }

    /* x is y's only child */
    if(y->left != NULL)
    {
        x = y->left;
    }
    else
    {
        x = y->right;
    }

    /* remove y from the parent chain */

    if(y->parent)
    {
        if(y == y->parent->left)
        {
            y->parent->left = x;
            w = y->parent->right;
        }
        else
        {
            y->parent->right = x;
            w = y->parent->left;
        }
    }
    else
    {
        header->root = x;
    }

    if(x == NULL)
    {
        if(w != NULL)
        {
            x = y;
            x_null = 1;
        }
        else
        {
            x = y->parent;
        }
    }
    else
    {
        x->parent = y->parent;
    }

    if(y != z)
    {
        z->data = y->data;
    }

    if(y->color == RBC_BLACK)
    {
        deleteFixup(header, x, x_null);
    }

    header->node_count--;
    if(header->rb_free_data)
    {
        header->rb_free_data(data);
    }
}
