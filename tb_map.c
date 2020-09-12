#include "tb_map.h"

#include <stdlib.h>

#include <stdint.h>

/* ----------------------------| map entry |------------------------------------------- */
struct tb_map_entry
{
    void* key;
    void* value;
    tb_map_entry* parent;
    tb_map_entry* left;
    tb_map_entry* right;
    uint32_t height;
};

/* Allocates a new map entry. */
static tb_map_entry* tb_map_entry_alloc(const tb_map* map, const void* key, void* value, tb_map_entry* parent)
{
    tb_map_entry* entry = malloc(sizeof(tb_map_entry));

    if (entry)
    {
        entry->key = map->key_alloc ? map->key_alloc(key) : key;
        entry->value = map->value_alloc ? map->value_alloc(value) : value;
        entry->parent = parent;
        entry->left = NULL;
        entry->right = NULL;
        entry->height = 1; /* new entry is initially added at leaf */
    }

    return entry;
}

/* Frees a map entry. */
static void tb_map_entry_free(const tb_map* map, tb_map_entry* entry)
{
    if (!entry) return;

    if (map->key_free)
        map->key_free(entry->key);

    if (map->value_free)
        map->value_free(entry->value);

    free(entry);
}

/* Default compare function. */
static int tb_map_entry_cmp(const void* left, const void* right)
{
    return *(uint32_t*)left - *(uint32_t*)right;
}

/* ----------------------------| AVL-Tree functions |---------------------------------- */

/* Utility function to get height of the tree */
static uint32_t tb_map_avl_node_height(tb_map_entry* node)
{
    return node ? node->height : 0;
}

/* Utility function to update the height of an entry node */
static void tb_map_avl_update_height(tb_map_entry* node)
{
    if (node)
    {
        int l = tb_map_avl_node_height(node->left);
        int r = tb_map_avl_node_height(node->right);
        node->height = ((l > r) ? l : r) + 1;
    }
}

/* Get balance of an entry node */
static int8_t tb_map_avl_get_balance(tb_map_entry* node)
{
    return node ? tb_map_avl_node_height(node->left) - tb_map_avl_node_height(node->right) : 0;
}

/* Returns the node with minimum key value found in a tree with given root. */
static tb_map_entry* tb_map_avl_min_entry(const tb_map_entry* root)
{
    if (!root) return NULL;

    tb_map_entry* current = (tb_map_entry*)root;

    /* loop down to find the leftmost leaf */
    while (current->left) current = current->left;

    return current;
}

/* Returns the inorder successor of the node or NULL if there is none. */
static tb_map_entry* tb_map_avl_next_inorder(const tb_map_entry* node)
{
    if (node)
    {
        if (node->right) return tb_map_avl_min_entry(node->right);

        tb_map_entry* next = node->parent;
        while (next && node == next->right)
        {
            node = next;
            next = next->parent;
        }
        return next;
    }
    return NULL;
}

/* A utility function to right rotate subtree rooted with node */
static tb_map_entry* tb_map_avl_right_rotate(tb_map_entry* node)
{
    tb_map_entry* left = node->left;

    /* Perform rotation */
    node->left = left->right;
    left->right = node;

    /* Update parents */
    if (node->left) node->left->parent = node;
    left->parent = node->parent;
    node->parent = left;

    /* Update heights */
    tb_map_avl_update_height(node);
    tb_map_avl_update_height(left);

    return left;
}

/* A utility function to left rotate subtree rooted with node */
static tb_map_entry* tb_map_avl_left_rotate(tb_map_entry* node)
{
    tb_map_entry* right = node->right;

    /* Perform rotation */
    node->right = right->left;
    right->left = node;

    /* Update parents */
    if (node->right) node->right->parent = node;
    right->parent = node->parent;
    node->parent = right;

    /* Update heights */
    tb_map_avl_update_height(node);
    tb_map_avl_update_height(right);

    return right;
}

/* A utility function to rebalance an entry node */
static tb_map_entry* tb_map_avl_rebalance(tb_map_entry* node)
{
    int8_t balance = tb_map_avl_get_balance(node);

    if (balance > 1)
    {
        if (tb_map_avl_get_balance(node->left) < 0)
            node->left = tb_map_avl_left_rotate(node->left);

        if (node->parent)
        {
            if (node == node->parent->left)
                node->parent->left = tb_map_avl_right_rotate(node);
            else
                node->parent->right = tb_map_avl_right_rotate(node);
        }
        else
        {
            node = tb_map_avl_right_rotate(node);
        }
    }
    else if (balance < -1)
    {
        if (tb_map_avl_get_balance(node->right) > 0)
            node->right = tb_map_avl_right_rotate(node->right);

        if (node->parent)
        {
            if (node == node->parent->left)
                node->parent->left = tb_map_avl_left_rotate(node);
            else
                node->parent->right = tb_map_avl_left_rotate(node);
        }
        else
        {
            node = tb_map_avl_left_rotate(node);
        }
    }

    return node;
}

/* ------------------------------------------------------------------------------------ */

static void tb_map_reset(tb_map* map)
{
    map->root = NULL;
    map->cmp_func = NULL;
    map->key_alloc = NULL;
    map->key_free = NULL;
    map->value_alloc = NULL;
    map->value_free = NULL;
}

tb_map_error tb_map_alloc(tb_map* map, int(*cmp_func)(const void*,const void*))
{
    tb_map_reset(map);

    map->cmp_func = cmp_func ? cmp_func : tb_map_entry_cmp;

    return TB_MAP_OK;
}

void tb_map_free(tb_map* map)
{
    tb_map_clear(map);
    tb_map_reset(map);
}

void tb_map_set_key_alloc_funcs(tb_map* map, void* (*alloc_func)(const void*), void (*free_func)(void*))
{
    map->key_alloc = alloc_func;
    map->key_free = free_func;
}

void tb_map_set_value_alloc_funcs(tb_map* map, void* (*alloc_func)(const void*), void (*free_func)(void*))
{
    map->value_alloc = alloc_func;
    map->value_free = free_func;
}

void tb_map_clear(tb_map* map)
{
    tb_map_entry* node = tb_map_avl_min_entry(map->root);

    while(node)
    {
        tb_map_entry* next = tb_map_avl_next_inorder(node);
        tb_map_entry_free(map, node);

        node = next;
    }

    map->root = NULL;
}

tb_map_error tb_map_insert(tb_map* map, const void* key, void* value)
{
    tb_map_entry* node = NULL;
    tb_map_entry* temp = map->root;

    /* find position to insert */
    while (temp)
    {
        if (map->cmp_func(key, temp->key) == 0) return TB_MAP_KEY_DUPLICATE; /* key already in tree */

        node = temp;
        if (map->cmp_func(key, temp->key) < 0)
            temp = temp->left;
        else 
            temp = temp->right;
    }

    /* insert node */
    if (!node)
        map->root = tb_map_entry_alloc(map, key, value, NULL);  /* allocate root */
    else
    {
        if (map->cmp_func(key, node->key) < 0) 
            node->left = tb_map_entry_alloc(map, key, value, node);
        else
            node->right = tb_map_entry_alloc(map, key, value, node);

        /* rebalance tree */
        while (node)
        {
            tb_map_avl_update_height(node);

            temp = tb_map_avl_rebalance(node);
            node = temp->parent;
        }
        map->root = temp;
    }

    return TB_MAP_OK;
}

tb_map_error tb_map_remove(tb_map* map, const void* key)
{
    tb_map_entry* node = map->root;

    while (node && map->cmp_func(key, node->key) != 0)
    {
        if (map->cmp_func(key, node->key) < 0)
            node = node->left;
        else
            node = node->right;
    }
  
    if (!node) return TB_MAP_KEY_NOT_FOUND; /* key not found */

    /* node with only one child or no child */
    if ((node->left == NULL) || (node->right == NULL))
    {
        tb_map_entry* child = node->left ? node->left : node->right;

        /* update parents link to child */
        if (node->parent)
        {
            if (node == node->parent->left)
                node->parent->left = child;
            else
                node->parent->right = child;
        }
        else
        {
            /* node to be removed is root, so update root pointer */
            map->root = child;
        }

        /* update parent pointer if node has one child */
        if (child) child->parent = node->parent;

        tb_map_entry_free(map, node);
        node = child;
    }
    else
    {
        /* node with two children: Get the inorder successor */
        tb_map_entry* temp = tb_map_avl_min_entry(node->right);
  
        /* Copy the inorder successor's data to this node */
        node->value = temp->value;

        /* 
         * update successor's parents link
         * successor can only have a right child or no child
         */
        if (temp->parent)
        {
            if (temp == temp->parent->left)
                temp->parent->left = temp->right;
            else
                temp->parent->right = temp->right;
        }
        
        tb_map_entry_free(map, temp);
    }
    
    /* rebalance tree */
    tb_map_entry* temp = NULL;
    while (node)
    {
        tb_map_avl_update_height(node);

        temp = tb_map_avl_rebalance(node);
        node = temp->parent;
    }
    
    map->root = temp;
    return TB_MAP_OK;
}
 
void* tb_map_find(const tb_map* map, const void* key)
{
    const tb_map_entry* node = map->root;
    while (node && map->cmp_func(key, node->key) != 0)
    {
        if (map->cmp_func(key, node->key) < 0)
            node = node->left;
        else 
            node = node->right;
    }
    return node ? node->value : NULL;
}

tb_map_iter* tb_map_iterator(const tb_map* map)
{
    return (tb_map_iter*)tb_map_avl_min_entry(map->root);
}

tb_map_iter* tb_map_iter_next(const tb_map_iter* iter)
{
    return (tb_map_iter*)tb_map_avl_next_inorder((tb_map_entry*)iter);
}

tb_map_iter* tb_map_iter_remove(tb_map* map, const tb_map_iter* iter)
{
    tb_map_entry* next = tb_map_avl_next_inorder((tb_map_entry*)iter);

    // TODO

    return (tb_map_iter*)next;
}

const void* tb_map_iter_get_key(const tb_map_iter* iter)
{
    return iter ? ((tb_map_entry*)iter)->key : -1;
}

void* tb_map_iter_get_value(const tb_map_iter* iter)
{
    return iter ? ((tb_map_entry*)iter)->value : NULL;
}