#ifndef N_TREE_HPP

#define N_TREE_HPP

#include <stdlib.h>
#include <vector>
#include <unordered_set>
#include "objects/object.hpp"

#define MAX_DEPTH 5

// Forward declaration
namespace Engine
{
    bool aabb_overlap(float min_x1, float min_y1, float min_z1, float max_x1, float max_y1, float max_z1, float min_x2, float min_y2, float min_z2, float max_x2, float max_y2, float max_z2);
    bool aabb_overlap(float min_x1, float min_y1, float min_z1, float max_x1, float max_y1, float max_z1, float min_x2, float min_y2);
}

class N_Tree
{
    public:
    std::vector<N_Tree*> children;
    std::unordered_set<const Object*> objects;
    size_t n;
    size_t depth;
    float x;
    float y;
    float z;
    float side;

    N_Tree(size_t n, size_t depth);
};

namespace NTree
{
    N_Tree* create_tree(float x, float y, float z, size_t n, size_t curr_depth);

    void insert(N_Tree *node, Object *obj, float min_x, float min_y, float max_x, float max_y);
    void insert(N_Tree *node, Object *obj, float min_x, float min_y, float min_z, float max_x, float max_y, float max_z);

    void find_obj_nodes(N_Tree *node, const Object *obj, float min_x, float min_y, float min_z, float max_x, float max_y, float max_z, std::vector<N_Tree*> &nodes);
    void find_obj_nodes(N_Tree *node, const Object *obj, float min_x, float min_y, float max_x, float max_y, std::vector<N_Tree*> &nodes);
}

#endif
