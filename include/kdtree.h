//
// Created by iceytan on 18-12-3.
//

#ifndef KDTREE_KDTREE_H
#define KDTREE_KDTREE_H

#include <vector>
#include <limits>

class kdtree {

    using IndiceIter = std::vector<int>::iterator;

    struct _node
    {
        int id;
        int split_dim;
        double distance;
        _node *left, *right;
        _node()
        :
        left(nullptr),
        right(nullptr),
        distance(std::numeric_limits<double>::max()),
        split_dim(-1),
        id(-1){}

        bool operator < (_node& o)
        {
            return this->distance < o.distance;
        }
    };

    struct node_ptr_cmp
    {
        bool operator () (const _node* n1, const _node* n2)
        {
            return n1->distance < n2->distance;
        }
    };

    _node* _root_;

    unsigned long mSampleNum;

    unsigned long mFeatureNum; // assume that all samples have the same feature number.

    bool* is_visit;

    _node* BuildTree(IndiceIter begin, IndiceIter end, int depth);

    int FindTheSplitDim(IndiceIter begin, IndiceIter end, int depth);

    IndiceIter GetMidNum(IndiceIter begin, IndiceIter end, int dim);

    std::vector<std::vector<double>> * data;

public:
    kdtree(std::vector<std::vector<double>>& data);
    ~kdtree();
    std::vector<int> RadiusSearch(const std::vector<double>& input, const double& distance);
    std::vector<int> NearestSearch(const std::vector<double>& input, const int& K = 1);

};


#endif //KDTREE_KDTREE_H
