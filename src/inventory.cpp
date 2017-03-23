
#include <cstdlib>
#include <stdint.h>

#include "inventory.h"
#include "engine.h"
#include "world.h"

int32_t Inventory_AddItem(struct inventory_node_s **root, uint32_t item_id, int32_t count)// returns items count after in the function's end
{
    base_item_p item = World_GetBaseItemByID(item_id);

    if(root && item)
    {
        inventory_node_p *i  = root;
        for(; *i; i = &((*i)->next))
        {
            if((*i)->id == item_id)
            {
                (*i)->count += count;
                return (*i)->count;
            }
        }

        *i = (inventory_node_p)malloc(sizeof(inventory_node_t));
        (*i)->id = item_id;
        (*i)->count = count;
        (*i)->max_count = 0xFFFFFFFF;
        (*i)->next = NULL;

        return count;
    }

    return 0;
}

// returns items count after in the function's end
int32_t Inventory_RemoveItem(struct inventory_node_s **root, uint32_t item_id, int32_t count)
{
    if(root)
    {
        for(inventory_node_p *i  = root; *i; i = &((*i)->next))
        {
            if((*i)->id == item_id)
            {
                if((*i)->count > count)
                {
                    (*i)->count -= count;
                    return (*i)->count;
                }
                else if((*i)->count == count)
                {
                    inventory_node_p next = (*i)->next;
                    free(*i);
                    *i = next;
                    return 0;
                }
                else // count_to_remove > current_items_count
                {
                    return (int32_t)(*i)->count - (int32_t)count;
                }
            }
        }
    }

    return -count;
}


int32_t Inventory_RemoveAllItems(struct inventory_node_s **root)
{
    int32_t ret = 0;

    if(!root)
    {
        return ret;
    }

    while(*root)
    {
        inventory_node_p next_i = (*root)->next;
        free(*root);
        *root = next_i;
        ret++;
    }

    return ret;
}


int32_t Inventory_GetItemsCount(struct inventory_node_s *root, uint32_t item_id)
{
    for(inventory_node_p i = root; i; i = i->next)
    {
        if(i->id == item_id)
        {
            return i->count;
        }
    }

    return 0;
}