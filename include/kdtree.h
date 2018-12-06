//
// Created by iceytan on 18-12-3.
//

#ifndef KDTREE_KDTREE_H
#define KDTREE_KDTREE_H

#include <vector>
#include <limits>
#include <queue>
#include <string>

namespace kt
{

    enum SPLIT_MODE {DEPTH, VARIANCE};

    struct _node
    {
        int id;
        int split_dim;
        double distance;
        bool is_visit;
        _node *left, *right;
        _node()
                :
                is_visit(false),
                left(nullptr),
                right(nullptr),
                distance(-1),
                split_dim(-1),
                id(-1){}

        bool operator < (_node& o)
        {
            if (this->distance == o.distance ) return this->id < o.id;
            return this->distance < o.distance;
        }
    };

    struct node_ptr_cmp
    {
        bool operator () (const _node* n1, const _node* n2)
        {
            if (n1->distance == n2->distance) return n1->id <  n2->id;
            return n1->distance < n2->distance ;
        }
    };


    using IndiceIter = std::vector<int>::iterator;
    using NodeQueue  = std::priority_queue<_node*, std::vector<_node*>, node_ptr_cmp >;

    template <typename T>
    class kdtree
    {
    private:

        _node* _root_;

        unsigned long mSampleNum;

        unsigned long mFeatureNum; // assume that all samples have the same feature number.

        _node* BuildTree(IndiceIter begin, IndiceIter end, int depth);

        int FindTheSplitDim(IndiceIter begin, IndiceIter end, int depth);

        IndiceIter GetMidNum(IndiceIter begin, IndiceIter end, int dim);

        std::string Node2Dot(_node* node);

        const std::vector<std::vector<T>> * data;

        const std::vector<T>* input;

        SPLIT_MODE mSplitMode;

    public:

        /**
         * create a kdtree
         * @param data which is used to create kdtree, row stands for a sample and col stands for feature.
         * @param mode method to split the hyperplane, DEPTH or VARIANCE
         */
        kdtree(std::vector<std::vector<T>>& data, SPLIT_MODE mode);

        /**
         * find all samples within certain distance from input target.
         * @param input target data, must has the same feature dimension with sample data.
         * @param distance
         * @return index of all samples within certain distance from input target.
         */
        std::vector<int> RadiusSearch(const std::vector<T>& input, const double& distance);

        /**
         * find the K nearest data samples from input.
         * @param input target data, must has the same feature dimension with sample data.
         * @param K number of nearest data samples you want to find.
         * @return index of K nearest data samples from input
         */
        std::vector<int> NearestSearch(const std::vector<T>& input, const int& K = 1);


        /**
         * generate a bitmap of tree's structure. PNG file can be find in the workspace folder
         * Power by [Graphviz](https://www.graphviz.org/)
         * @return content of .dot file
         */
        std::string ToDot();


        ~kdtree();

    };


}

#include "../src/kdtree.cpp"

#endif //KDTREE_KDTREE_H
