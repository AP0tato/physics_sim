#include "ntree.hpp"

N_Tree::N_Tree(size_t n)
{
    children.resize(n);
    this->x = x;
    this->y = y;
    this->z = z;
}

N_Tree* create_tree(float x, float y, float z, size_t n, size_t curr_depth)
{
    if(curr_depth > MAX_DEPTH)
        return nullptr;

    N_Tree *head = new N_Tree(n);
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
    
}
