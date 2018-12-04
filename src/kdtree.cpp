//
// Created by iceytan on 18-12-3.
//

#include <kdtree.h>
#include <cassert>
#include <algorithm>
#include <queue>
#include <stack>
#include <unordered_map>
#include <cmath>
#include <iostream>

#include "kdtree.h"

std::vector<int> kdtree::RadiusSearch(const std::vector<double> &input, const double& distance) {
    assert(distance>=0);
    assert(input.size() == mFeatureNum);

    std::fill(is_visit,is_visit+mSampleNum,false);
    std::stack<_node*> path;
    std::priority_queue<_node*, std::vector<_node*>, node_ptr_cmp > heap;

    _node* _cur = _root_;

    static auto Push = [&](_node* ptr){

        if (is_visit[ptr->id]) return;
        else is_visit[ptr->id] = true;

        ptr->distance = 0;
        for (int feat_i = 0;feat_i < mFeatureNum; ++feat_i)
            ptr->distance += pow(input[feat_i] - (*data)[ptr->id][feat_i],2.0);
        ptr->distance = sqrt(ptr->distance);

        path.push(ptr);

        if(ptr->distance <= distance) heap.push(ptr);
    };

    while (_cur)
    {
        Push(_cur);
        auto& dim = _cur->split_dim;
        _cur = input[dim] <= (*data)[_cur->id][_cur->split_dim]? _cur->left: _cur->right;
    }

    while(!path.empty())
    {
        _cur = path.top(); path.pop();

        double vi = input[_cur->split_dim];

        double vn = (*data)[_cur->id][_cur->split_dim];

        if (vi > vn)
        {
            if( _cur->left  and vi - vn < distance) Push(_cur->left);
        }
        else
        {
            if( _cur->right and vn - vi < distance) Push(_cur->right);
        }
    }

    std::vector<int> res;
    while (!heap.empty())
    {
        res.emplace_back(heap.top()->id);
        heap.pop();
    }


    return std::vector<int>(res.rbegin(),res.rend());
}

std::vector<int> kdtree::NearestSearch(const std::vector<double> &input, const int& K) {

    assert(input.size() == mFeatureNum);

    std::fill(is_visit,is_visit+mSampleNum,false);
    std::stack<_node*> path;
    std::priority_queue<_node*, std::vector<_node*>, node_ptr_cmp > k_heap;

    static auto Push = [&](_node* ptr){

        if (is_visit[ptr->id]) return;
        else is_visit[ptr->id] = true;

        path.push(ptr);

        ptr->distance = 0;
        for (int feat_i = 0;feat_i < mFeatureNum; ++feat_i)
            ptr->distance += pow(input[feat_i] - (*data)[ptr->id][feat_i],2.0);
        ptr->distance = sqrt(ptr->distance);

        if (k_heap.size()<K) k_heap.push(ptr);
        else if((*ptr) < (*k_heap.top()))
        {
            k_heap.pop();
            k_heap.push(ptr);
        }
    };

    _node* _cur = _root_;

    // find closest nearest point
    while (_cur)
    {
        Push(_cur);
        auto& dim = _cur->split_dim;
        _cur = input[dim] <= (*data)[_cur->id][_cur->split_dim]? _cur->left: _cur->right;
    }

    // find the nearest point

    while (!path.empty())
    {

        _cur = path.top(); path.pop();

        if (k_heap.size()<K)
        {
            if (_cur->left)  Push(_cur->left);
            if (_cur->right) Push(_cur->right);
        }
        else
        {
            auto vi = input[_cur->split_dim];
            auto vn = (*data)[_cur->id][_cur->split_dim];
            auto vh = k_heap.top()->distance;

            // Search the other side of the splitting plane if it may contain
            // points closer than the farthest point in the current results.

            if (vi > vn)
            {
                if (_cur->right) Push(_cur->right);
                if (_cur->left  && (vi - vn) < vh ) Push(_cur->left);
            }
            else
            {
                if (_cur->left)  Push(_cur->left);
                if (_cur->right && (vn - vi) < vh ) Push(_cur->right);
            }
        }
    }

    std::vector<int> res;
    while (!k_heap.empty())
    {
        res.emplace_back(k_heap.top()->id);
        k_heap.pop();
    }

    return std::vector<int>(res.rbegin(),res.rend());
}

kdtree::kdtree(std::vector<std::vector<double>> &data, SPLIT_MODE mode) {
    assert(!data.empty());

    this->data = &data;
    this->mFeatureNum = data[0].size();
    this->mSampleNum  = data.size();
    this->is_visit = new bool[data.size()];
    this->mSplitMode = mode;

    std::vector<int> indices(mSampleNum);
    for (int i = 0; i < mSampleNum; ++i) indices[i] = i;
    _root_ = BuildTree(indices.begin(), indices.end(), 0);
}

kdtree::~kdtree() {
    delete (is_visit);
    is_visit = nullptr;
    data = nullptr;

    std::stack<_node* > stack;
    stack.push(_root_);
    _node* _cur;
    while(!stack.empty())
    {
        _cur = stack.top(); stack.pop();

        if (_cur->left)  stack.push(_cur->left);
        if (_cur->right) stack.push(_cur->right);

        free(_cur);
    }
    _root_ = nullptr;
}

kdtree::_node *kdtree::BuildTree(IndiceIter begin, IndiceIter end, int depth) {
    auto _root = new _node;

    // find the dimension used to split.
    _root->split_dim = FindTheSplitDim(begin, end, depth);

    // indices will be sorted a bit. indices will be left of mid_id which are
    auto mid_id = GetMidNum(begin, end, _root->split_dim);
    _root->id = *mid_id;

    // split indices into left and right, and do building
    if (std::distance(begin, mid_id)>0)
        _root->left  = BuildTree(begin, mid_id, depth + 1);
    if (std::distance(mid_id+1, end)>0)
        _root->right = BuildTree(mid_id+1, end, depth + 1);

    return _root;
}

int kdtree::FindTheSplitDim(IndiceIter begin, IndiceIter end, int depth)
{

    int best_dim = depth % mFeatureNum;

    if ( this->mSplitMode == DEPTH )
    {
        // DO NOTHING
    }
    else if( this->mSplitMode == VARIANCE )
    {
        double avg[mFeatureNum];

        std::fill(avg,avg+mFeatureNum,0);

        for(auto it = begin; it!=end; ++it)
        {
            for(int f = 0; f < mFeatureNum; ++f)
            {
                avg[f] += (*data)[*it][f];
            }
        }


        double max_var = std::numeric_limits<double >::min();

        auto _sample_num = std::distance(begin,end);
        for(int f=0; f< mFeatureNum; ++f)
        {
            avg[f]/=_sample_num;
            double _var = 0;
            for(auto it = begin; it!=end; ++it)
                _var += pow(avg[f]-(*data)[*it][f],2);

            if(max_var < _var)
            {
                best_dim = f;
                max_var = _var;
            }
        }
    }

    return best_dim;
}

kdtree::IndiceIter kdtree::GetMidNum(IndiceIter begin, IndiceIter end, int dim) {

    auto idx = (end-begin)/2;

    std::nth_element(begin, begin+idx, end,
        [&](int idx1, int idx2) -> bool
        {
            return (*data)[idx1][dim] < (*data)[idx2][dim];
        });
    return begin + idx;
}
