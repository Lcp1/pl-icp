#include <time.h>
#include <string.h>
#include <libgen.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <dirent.h>
#include <ctime>
#include <cstdlib>


#include "csm/laser_data_yogo.h"
#include "icp/pl_icp.h"

using namespace std;

int main(int argc, const char*argv[]) {
    // generate random data for test
    int num = 1081;
    int  range_ref[num];
    char flags_ref[num];
    int  range_curr[num];
    char flags_curr[num];
    double delta_odom_ref[3] = {0., 0., 0.};
    double delta_odom_curr[3] = {.8, .1, .05};
    srand((unsigned)time(0) );
    for (int i=0; i<num; ++i) {
        range_ref[i] = rand()%9000;
        range_curr[i] = range_ref[i] + 1000;
        flags_ref[i] = (char)abs(rand()%2);
        flags_curr[i] = flags_ref[i];
    }

    // input data
    LDP laser_ref = set_laser_frame(range_ref, flags_ref,
                                    num, delta_odom_ref);
    LDP laser_curr = set_laser_frame(range_curr, flags_curr,
                                     num, delta_odom_curr);
    // check data
    if(!ld_valid_fields(laser_ref))  {
        sm_error(" -icp- Invalid laser data in first scan.\n");
        return -2;
    }
    if(!ld_valid_fields(laser_curr))  {
        sm_error(" -icp- Invalid laser data in second scan.\n");
        return -2;
    }
    if(	any_nan(laser_ref->odometry,3) ||
        any_nan(laser_curr->odometry,3) ) {
        printf("odometry NAN.!!\n");
        return -3;
    }

//    printf(" -icp- set input data sucessed!\n");

    set_plicp_params(laser_ref, laser_curr);

    vector<double> results(5);
    Eigen::Matrix3d covariance;
    do_plicp(results, covariance);

    double transform[3] = {results[0], results[1], results[2]};
    // output result
    printf(" -icp- delta transform : [%f, %f, %f]\n", transform[0], transform[1], transform[2]);
    printf(" -icp- cost time: %f ms\n", results[3]);
    printf(" -icp- iterations: %d\n", int(results[4]));

    vector<double> gp = get_global_position(transform);

    printf(" -icp- global positions: [%f, %f, %f]\n", gp[0], gp[1], gp[2]);
    printf(" -icp- covariance matrix:\n");
    cout << covariance << endl;

    // free
    ld_free(laser_ref);
    ld_free(laser_curr);

    return 0;
}
