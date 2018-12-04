//
// Created by iceytan on 18-12-3.
//

#ifndef KDTREE_KDTREE_H
#define KDTREE_KDTREE_H

#include <vector>

class kdtree {
public:
    enum SPLIT_MODE {DEPTH, VARIANCE};
private:

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

    SPLIT_MODE mSplitMode;

public:

    /**
     * create a kdtree
     * @param data which is used to create kdtree, row stands for a sample and col stands for feature.
     * @param mode method to split the hyperplane, DEPTH or VARIANCE
     */
    explicit kdtree(std::vector<std::vector<double>>& data, SPLIT_MODE mode = DEPTH);

    /**
     * find all samples within certain distance from input target.
     * @param input target data, must has the same feature dimension with sample data.
     * @param distance
     * @return index of all samples within certain distance from input target.
     */
    std::vector<int> RadiusSearch(const std::vector<double>& input, const double& distance);

    /**
     * find the K nearest data samples from input.
     * @param input target data, must has the same feature dimension with sample data.
     * @param K number of nearest data samples you want to find.
     * @return index of K nearest data samples from input
     */
    std::vector<int> NearestSearch(const std::vector<double>& input, const int& K = 1);

    ~kdtree();

};


#endif //KDTREE_KDTREE_H
