#include <iostream>
#include <vector>
#include <kdtree.h>

using namespace std;


int main() {

    std::vector<std::vector<double >> test_data
    {
        {2,3},{5,4},{9,6},{4,7},{8,1},{7,2}
    };

    kt::kdtree<double> kd(test_data, kt::DEPTH);

    cout  << "NearestSearch\n";
    auto idxs = kd.NearestSearch( {2, 4.5}, 2);

    for(auto idx: idxs)
    {
        cout << "id: "<< idx<<" coor: "<<test_data[idx][0]<<" " << test_data[idx][1]<< endl;
    }


    cout  << "RadiusSearch\n";
    idxs = kd.RadiusSearch( {2, 4.5}, 3.5);

    for(auto idx: idxs)
    {
        cout << "id: "<< idx<<" coor: "<<test_data[idx][0]<<" " << test_data[idx][1]<< endl;
    }

    kd.ToDot(true); // save tree
}
