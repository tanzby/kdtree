# kdtree

Simple implementation of kdtree


## require

* c++11

## usage

```c++
#include <iostream>
#include <vector>
#include <kdtree.h>

using namespace std;

int main() {

    std::vector<std::vector<double >> test_data
    {
        {2,3},{5,4},{9,6},{4,7},{8,1},{7,2}
    };

    kdtree kd(test_data);
    auto idx = kd.NearestSearch( {2.1,3.1} ) [0];

    cout << test_data[idx][0]<<" " << test_data[idx][0]<< endl;
}
``` 