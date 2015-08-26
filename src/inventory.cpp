
#include <stdint.h>

#include "inventory.h"
#include "engine.h"
#include "world.h"

int32_t Character_AddItem(struct inventory_node_s **root, uint32_t item_id, int32_t count)// returns items count after in the function's end
{
    if(!root)
    {
        return 0;
    }

    base_item_p item = World_GetBaseItemByID(&engine_world, item_id);
    if(!item)
    {
        return 0;
    }

    inventory_node_p last, i  = *root;

    count = (count == -1)?(item->count):(count);
    last = i;
    while(i)
    {
        if(i->id == item_id)
        {
            i->count += count;
            return i->count;
        }
        last = i;
        i = i->next;
    }

    i = (inventory_node_p)malloc(sizeof(inventory_node_t));
    i->id = item_id;
    i->count = count;
    i->next = NULL;
    if(last != NULL)
    {
        last->next = i;
    }
    else
    {
        *root = i;
    }

    return count;
}

// returns items count after in the function's end
int32_t Character_RemoveItem(struct inventory_node_s **root, uint32_t item_id, int32_t count)
{
    if(!root || !*root)
    {
        return 0;
    }

    if((*root)->id == item_id)
    {
        inventory_node_p i = *root;
        if(i->count > count)
        {
            i->count -= count;
            return i->count;
        }
        else if(i->count == count)
        {
            *root = i->next;
            free(i);
            return 0;
        }
        else // count_to_remove > current_items_count
        {
            return (int32_t)i->count - (int32_t)count;
        }
    }

    inventory_node_p prev_item = *root;
    for(inventory_node_p i = prev_item->next; i; i = i->next)
    {
        if(i->id == item_id)
        {
            if(i->count > count)
            {
                i->count -= count;
                return i->count;
            }
            else if(i->count == count)
            {
                prev_item->next = i->next;
                free(i);
                return 0;
            }
            else // count_to_remove > current_items_count
            {
                return (int32_t)i->count - (int32_t)count;
            }
        }
        prev_item = i;
    }

    return -count;
}


int32_t Character_RemoveAllItems(struct inventory_node_s **root)
{
    if(!root || !*root)
    {
        return 0;
    }

    inventory_node_p curr_i = *root;
    int32_t ret = 0;

    while(curr_i)
    {
        inventory_node_p next_i = curr_i->next;
        free(curr_i);
        curr_i = next_i;
        ret++;
    }
    *root = NULL;

    return ret;
}


int32_t Character_GetItemsCount(struct inventory_node_s *root, uint32_t item_id)
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