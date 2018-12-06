//
// Created by iceytan on 18-12-5.
//

#include <iostream>
#include <kdtree.h>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include <set>

using namespace std;

typedef  std::chrono::steady_clock::time_point tp;
#define  Now std::chrono::steady_clock::now()


int main()
{
    const int M = 5000, N = 2;

    // make some noise
    std::mt19937 mt = std::mt19937 (std::default_random_engine()());
    auto get_random = [&]() -> int { return static_cast<int>(mt() % 1000); };

    vector<vector<int>> data(M,vector<int>(N));
    for (int i=0; i<M; ++i)
    {
        for(int j=0; j<N; ++j)
        {
            data[i][j] = get_random();
        }
    }

    vector<int> test_input(N);


    /* find K nearest */
    int K = 10;
    kdtree<int> kd(data, VARIANCE);


    vector<double> distance(M,0);
    vector<int> idx(M);

    // native way method
    auto native_find_K = [&distance,&idx, &data](vector<int>& input, int K)
    {
        iota(idx.begin(),idx.end(),0);

        for(int i=0; i<M; ++i)
        {
            distance[i] = 0;
            for(int j = 0; j < N; ++j)
            {
                distance[i] += pow(input[j]-data[i][j],2);
            }
            distance[i] = sqrt(distance[i]);
        }
        sort(idx.begin(),idx.end(),
                [&distance](int i, int ii)->bool
                {
                    if (distance[i]==distance[ii]) return i < ii;
                    return distance[i]<distance[ii];
                }
        );
    };

    double total_time_native = 0;
    double total_time_kdtree = 0;

    for (int t=0; t<1000; ++t)
    {
        // generate test input
        for(int j=0; j<N; ++j)
        {
            test_input[j] = get_random();
        }
        tp t1 = Now;
        //native
        native_find_K(test_input,K);

        tp t2 = Now;

        // kdtree
        auto res = kd.NearestSearch(test_input,K);

        tp t3 = Now;
        total_time_native += std::chrono::duration<double>(t2-t1).count();
        total_time_kdtree += std::chrono::duration<double>(t3-t2).count();

        // compare
        int k = 0;
        for (; k<K; ++k)
        {
            if(res[k]!=idx[k])
            {
                break;
            }
        }
        if (k != K)
        {
            cout << t << ":\tfault" <<k<< endl;
            //kd.ToDot();
            cout  << " " << endl;
        }

    }

    cout << "Nearest Search: \n";
    cout << "total_time_native: " << total_time_native << endl;
    cout << "total_time_kdtree: " << total_time_kdtree << endl;


    /* radius search */
    double min_distance = 10000;
    total_time_native=total_time_kdtree = 0;
    for (int t=0; t<1000; ++t)
    {
        // generate test input
        for(int j=0; j<N; ++j)
        {
            test_input[j] = get_random();
        }

        set<int> native_set;

        // native
        tp t1 = Now;
        for (int i = 0; i < M; ++i)
        {
            double dis = 0;
            for(int j = 0; j < N; ++j)
            {
                dis += pow(test_input[j]-data[i][j],2);
            }
            dis = sqrt(dis);
            if (dis<=min_distance)
            {
                native_set.insert(i);
            }
        }

        // kdtree
        tp t2 = Now;
        auto r = kd.RadiusSearch(test_input,min_distance);
        tp t3 = Now;

        total_time_native += std::chrono::duration<double>(t2-t1).count();
        total_time_kdtree += std::chrono::duration<double>(t3-t2).count();

        set<int> kd_set(r.begin(),r.end());
        if (kd_set!=native_set)
        {
            cout << "fail" << endl;
        }
    }

    cout << "Radius Search: \n";
    cout << "total_time_native: " << total_time_native << endl;
    cout << "total_time_kdtree: " << total_time_kdtree << endl;

}