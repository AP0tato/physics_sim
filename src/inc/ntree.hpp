#ifndef N_TREE_HPP

#define H_TREE_HPP

#include <stdlib.h>
#include <vector>
#include "objects/object.hpp"

#define MAX_DEPTH 2

class N_Tree
{
    public:
    std::vector<N_Tree*> children;
    float x;
    float y;
    float z;
    float side;

    N_Tree(size_t n);
};

N_Tree* create_tree(float x, float y, float z, size_t n, size_t curr_depth);

void insert(N_Tree *node, Object *obj, float min_x, float min_y, float max_x, float max_y);

#endif
