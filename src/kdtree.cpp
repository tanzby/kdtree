//
// Created by iceytan on 18-12-3.
//

#include <kdtree.h>
#include <cassert>
#include <algorithm>
#include <stack>
#include <unordered_map>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <sys/time.h>

#include "kdtree.h"

using namespace kt;


std::string getCurrentTimeStr()
{
    struct timeval tv{};
    gettimeofday(&tv, nullptr);
    return std::to_string(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

inline std::vector<int> _queue2vector(NodeQueue &queue)
{
    unsigned long s = queue.size();
    assert(s > 0);
    std::vector<int> res(s);
    for(int i = 1; i <= s; ++i)
    {
        res[s-i] = queue.top()->id;
        queue.pop();
    }
    return res;
}

inline void _initialNodes(_node* root)
{
    std::stack<_node* > stack;
    stack.push(root);
    while(!stack.empty())
    {
        _node* _cur = stack.top(); stack.pop();

        if (_cur->left)  stack.push(_cur->left);
        if (_cur->right) stack.push(_cur->right);

        _cur->is_visit = false;
        _cur->distance = -1;
    }
}

template <typename T>
std::vector<int> kdtree<T>::RadiusSearch(const std::vector<T> &input, const double& distance)
{
    assert(distance>=0);
    assert(input.size() == mFeatureNum);
    this->input = &input;
    _initialNodes(_root_);

    std::stack<_node*> path;
    std::vector<int> res;  // initial a nice size for res

    static auto Push = [&](_node* ptr){

        if (ptr->is_visit) return;
        else ptr->is_visit = true;

        ptr->distance = 0;
        for (int feat_i = 0;feat_i < mFeatureNum; ++feat_i)
            ptr->distance += pow(input[feat_i] - (*data)[ptr->id][feat_i],2.0);
        ptr->distance = sqrt(ptr->distance);

        path.push(ptr);

        if(ptr->distance <= distance) res.emplace_back(ptr->id);
    };

    static auto Search = [&](_node* node)
    {
        while (node)
        {
            Push(node);
            auto& dim = node->split_dim;
            node = input[dim] <= (*data)[node->id][dim]? node->left: node->right;
        }
    };

    Search(_root_);

    while(!path.empty())
    {
        _node* _cur = path.top(); path.pop();

        const double& vi = input[_cur->split_dim];
        const double& vn = (*data)[_cur->id][_cur->split_dim];

        if (vi > vn)
        {
            if (_cur->right) Push(_cur->right);
            if (_cur->left  && (vi - vn) < distance) Search(_cur->left);
        }
        else
        {
            if (_cur->left)  Push(_cur->left);
            if (_cur->right && (vn - vi) < distance) Search(_cur->right);
        }
    }
    
    return res;
}

template <typename T>
std::vector<int> kdtree<T>::NearestSearch(const std::vector<T> &input, const int& K)
{

    assert(input.size() == mFeatureNum);
    this->input = &input;

    _initialNodes(_root_);
    std::stack<_node*> path;
    NodeQueue k_heap;

    static auto Push = [&](_node* ptr)
    {
        if (ptr->is_visit) return;
        else ptr->is_visit = true;

        path.push(ptr);

        ptr->distance = 0;
        for (int feat_i = 0;feat_i < mFeatureNum; ++feat_i)
            ptr->distance += pow(input[feat_i] - (*data)[ptr->id][feat_i],2.0);
        ptr->distance = sqrt(ptr->distance);

        if (k_heap.size()<K)
        {
            k_heap.push(ptr);
        }
        else if((*ptr) < (*k_heap.top()))
        {
            k_heap.pop();
            k_heap.push(ptr);
        }
    };

    static auto Search = [&](_node* node)
    {
        while (node)
        {
            Push(node);
            auto& dim = node->split_dim;
            node = input[dim] <= (*data)[node->id][dim]? node->left: node->right;
        }
    };

    Search(_root_);
    while (!path.empty())
    {
        _node* _cur = path.top(); path.pop();

        const double& vi = input[_cur->split_dim];
        const double& vn = (*data)[_cur->id][_cur->split_dim];
        const double& vh = k_heap.top()->distance;

        // Search the other side of the splitting plane if it may contain
        // points closer than the distance.

        if (vi > vn)
        {
            if (_cur->right) Push(_cur->right);
            if (_cur->left  && (vi - vn) < vh )
            {
                Search(_cur->left);
            }
        }
        else
        {
            if (_cur->left)  Push(_cur->left);
            if (_cur->right && (vn - vi) < vh )
            {
                Search(_cur->right);
            }
        }

    }

    return _queue2vector(k_heap);
}

template <typename T>
kdtree<T>::kdtree(std::vector<std::vector<T>> &data, SPLIT_MODE mode)
{
    assert(!data.empty());

    this->data = &data;
    this->mFeatureNum = data[0].size();
    this->mSampleNum  = data.size();
    this->mSplitMode = mode;

    std::vector<int> indices(mSampleNum);
    for (int i = 0; i < mSampleNum; ++i) indices[i] = i;
    _root_ = BuildTree(indices.begin(), indices.end(), 0);
}

template <typename T>
kdtree<T>::~kdtree()
{

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

template <typename T>
_node *kdtree<T>::BuildTree(IndiceIter begin, IndiceIter end, int depth)
{
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

template <typename T>
int kdtree<T>::FindTheSplitDim(IndiceIter begin, IndiceIter end, int depth)
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

template <typename T>
IndiceIter kdtree<T>::GetMidNum(IndiceIter begin, IndiceIter end, int dim)
{

    auto idx = (end-begin)/2;

    std::nth_element(begin, begin+idx, end,
        [&](int idx1, int idx2) -> bool
        {
            return (*data)[idx1][dim] < (*data)[idx2][dim];
        });
    return begin + idx;
}

template<typename T>
std::string kdtree<T>::Node2Dot(_node *node) {
    if (node== nullptr)
        return "";

    char buf[256];
    if (node->is_visit)
    {
        sprintf(buf,
                R"([shape=circle, label="ID: %d DIM:%d\nDIS: %8.4lf\nVN: %8.4lf\nVI: %8.4lf", fillcolor="%s", style=filled])",
                node->id,
                node->split_dim,
                node->distance,
                double((*data)[node->id][node->split_dim]),
                double((*input)[node->split_dim]),
                "#FFAA22");
    }
    else
    {
        sprintf(buf,
                R"([shape=circle, label="ID: %d DIM: %d\nVN: %8.4lf\nVI: %8.4lf", fillcolor="%s", style=filled])",
                node->id,
                node->split_dim,
                double((*data)[node->id][node->split_dim]),
                double((*input)[node->split_dim]),
                "#FFFFFF");
    }


    std::string cur_node_name = "node"+std::to_string(node->id);
    std::string cur_node_info = cur_node_name+ buf+"\n";

    if (node->left == nullptr && node->right == nullptr)
    {
        return cur_node_info;
    }


    std::string _left, _right;
    if(node->left)
    {
        _left = cur_node_name + " -> node"+std::to_string(node->left->id)+"\n";
        _left += Node2Dot(node->left);
    }
    if(node->right)
    {
        _right = cur_node_name + " -> node"+std::to_string(node->right->id)+"\n";
        _right += Node2Dot(node->right);
    }

    return cur_node_info + _left + _right;
}

template<typename T>
std::string kdtree<T>::ToDot() {
    if (_root_ == nullptr )
    {
        std::cout << "No receive any data" << std::endl;
        return std::string();
    }
    std::string _tmp ="digraph kdtree {\n" + Node2Dot(_root_) +"}\n";
    std::ofstream out(".kdtree.dot");
    out << _tmp;out.close();
    char command_str[64];
    sprintf(command_str,"/usr/bin/dot .kdtree.dot -T png -o %s.png",getCurrentTimeStr().c_str());
    system(command_str);
    return _tmp;
}


