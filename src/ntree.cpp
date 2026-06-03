#include "ntree.hpp"

N_Tree::N_Tree(size_t n, size_t depth)
{
    children.resize(n);
    this->depth = depth;
    this->n = n;
}

namespace NTree
{
    N_Tree* create_tree(float x, float y, float z, size_t n, size_t curr_depth)
    {
        if(curr_depth > MAX_DEPTH)
            return nullptr;

        N_Tree *head = new N_Tree(n, curr_depth);
        head->x = x;
        head->y = y;
        head->z = z;

        float c_side = 1.0 / pow(n, curr_depth + 1);

        for(size_t i = 0; i < n; ++i)
        {
            float cx = x + i * c_side;
            float cy = y + i * c_side;
            float cz = z + i * c_side;
            head->children[i] = create_tree(cx, cy, cz, n, curr_depth + 1);
        }

        return head;
    }

    void insert(N_Tree *node, Object *obj, float min_x, float min_y, float max_x, float max_y)
    {
        float n_side = 1.0 / pow(node->n, node->depth);

        if(!Engine::aabb_overlap(node->x, node->y, node->x + n_side, node->y + n_side, min_x, min_y, max_x, max_y))
            return;

        if(node->children[0] == nullptr)
        {
            node->objects.insert(obj);
            return;
        }

        for(auto i : node->children)
            insert(i, obj, min_x, min_y, max_x, max_y);
    }

    void insert(N_Tree *node, Object *obj, float min_x, float min_y, float min_z, float max_x, float max_y, float max_z)
    {
        float n_side = 1.0 / pow(node->n, node->depth);

        if(!Engine::aabb_overlap(node->x, node->y, node->z, node->x + n_side, node->y + n_side, node->z + n_side, min_x, min_y, min_y, max_x, max_y, max_z))
            return;

        if(node->children[0] == nullptr)
        {
            node->objects.insert(obj);
            return;
        }

        for(auto i : node->children)
            insert(i, obj, min_x, min_y, min_z, max_x, max_y, max_z);
    }

    void find_obj_nodes(N_Tree *node, const Object *obj, float min_x, float min_y, float min_z, float max_x, float max_y, float max_z, std::vector<N_Tree*> &nodes)
    {
        if(node->objects.count(obj))
            nodes.push_back(node);

        if(node->children[0] == nullptr)
            return;

        float c_side = 1.0 / pow(node->n, node->depth+1);

        for(auto &i : node->children)
        {
            if( ((i->x <= min_x) && (i->x + c_side >= min_x) && (i->y <= min_y) && (i->y + c_side >= min_y) && (i->z <= min_z) && (i->z + c_side >= min_z)) ||
                ((i->x <= max_x) && (i->x + c_side >= max_x) && (i->y <= max_y) && (i->y + c_side >= max_y) && (i->z <= max_z) && (i->z + c_side >= max_z)))
                find_obj_nodes(i, obj, min_x, min_y, min_z, max_x, max_y, max_z, nodes);
        }
    }

    void find_obj_nodes(N_Tree *node, const Object *obj, float min_x, float min_y, float max_x, float max_y, std::vector<N_Tree*> &nodes)
    {
        if(node->objects.count(obj))
            nodes.push_back(node);

        if(node->children[0] == nullptr)
            return;

        float c_side = 1.0 / pow(node->n, node->depth+1);

        for(auto &i : node->children)
        {
            if( ((i->x <= min_x) && (i->x + c_side >= min_x) && (i->y <= min_y) && (i->y + c_side >= min_y)) ||
                ((i->x <= max_x) && (i->x + c_side >= max_x) && (i->y <= max_y) && (i->y + c_side >= max_y)))
                find_obj_nodes(i, obj, min_x, min_y, max_x, max_y, nodes);
        }
    }
}