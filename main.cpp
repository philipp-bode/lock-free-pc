
#include <iostream>
#include <vector>
#include <algorithm>
#include <bits/stdc++.h>

#include <gsl/gsl_vector.h>
#include <gsl/gsl_statistics.h>

#include <armadillo>

#define rep(a, b)   for(int a = 0; a < (b); ++a)
#define debug(x)    clog << #x << " = " << x << endl;
#define all(a)      (a).begin(),(a).end()
#define endl '\n'

using namespace std;

std::vector<std::vector<double>> read_data() {
    std::freopen("cooling_house.data", "r", stdin);
    int variables, observations;
    double next_val;


    cin >> variables >> observations;
    std::vector<std::vector<double>> data(variables, std::vector<double>(observations));

    rep(o, observations) {
        rep(v, variables) {
            cin >> next_val;
            data[v][o] = next_val;
        }
    }

    return data;

}

arma::Mat<double> build_correlation_matrix(std::vector<std::vector<double>> &data) {
    size_t size = data.size();

    arma::Mat<double> correlation(size, size, arma::fill::eye);
    const int stride = 1;

    rep(i, size) {
        rep(j, i) {
            gsl_vector_const_view gsl_x = gsl_vector_const_view_array( &data[i][0], data[i].size());
            gsl_vector_const_view gsl_y = gsl_vector_const_view_array( &data[j][0], data[j].size());

            double pearson = gsl_stats_correlation(
                gsl_x.vector.data, stride,
                gsl_y.vector.data, stride,
                size
            );
            correlation(i,j) = correlation(j,i) = pearson;
        }
    }

    return correlation;
}

int main(int argc, char* argv[])
{
    std::ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.precision(10);

    auto data = read_data();
    auto cor = build_correlation_matrix(data);

    cor.print(cout);
    cout.flush();
    return 0;
}
