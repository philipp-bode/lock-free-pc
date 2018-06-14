
#include <iostream>
#include <vector>
#include <algorithm>
#include <bits/stdc++.h>

#include <gsl/gsl_vector.h>
#include <gsl/gsl_statistics.h>

#include <armadillo>

#include <boost/math/special_functions/log1p.hpp>
#include <boost/math/distributions/normal.hpp>
#include <boost/math/special_functions/fpclassify.hpp>
#include <boost/dynamic_bitset.hpp>

// #include "../pcalg/inst/include/pcalg/constraint.hpp"

#define rep(a, b)   for(int a = 0; a < (b); ++a)
#define debug(x)    clog << #x << " = " << x << endl;
#define all(a)      (a).begin(),(a).end()
#define endl '\n'

using namespace std;

class PCAlgorithm {

  public:
    uint nr_variables, level;
    const int STRIDE = 1;

    arma::Mat<unsigned char> _adjacencies;
    arma::Mat<unsigned char> _updated_adjacencies;
    arma::Mat<double>_correlation;
    
    PCAlgorithm(int vars) {
        nr_variables = vars;
        level = 0;
        _adjacencies = arma::Mat<unsigned char>(nr_variables, nr_variables, arma::fill::ones);
        _updated_adjacencies = arma::Mat<unsigned char>(nr_variables, nr_variables, arma::fill::ones);
        _correlation = arma::Mat<double>(nr_variables, nr_variables, arma::fill::eye);
    }

    std::vector<uint> adjacent_vertices(const uint vertex) {
        std::vector<uint> adj;
        adj.reserve(nr_variables);
        rep(i, nr_variables) {
            if (_adjacencies(i, vertex)) {
                adj.push_back(i);
            }
        }
        return adj;
    }

    void remove_edge(const uint x, const uint y) {
        _updated_adjacencies(x, y) = 0;
        _updated_adjacencies(y, x) = 0;
    }

    void sync_graph() {
        _adjacencies = _updated_adjacencies;
    }

    void build_correlation_matrix(std::vector<std::vector<double>> &data) {
        int n = data[0].size();
        rep(i, nr_variables) {
            rep(j, i) {
                gsl_vector_const_view gsl_x = gsl_vector_const_view_array( &data[i][0], n);
                gsl_vector_const_view gsl_y = gsl_vector_const_view_array( &data[j][0], n);
                double pearson = gsl_stats_correlation(
                    gsl_x.vector.data, STRIDE,
                    gsl_y.vector.data, STRIDE,
                    n
                );
                _correlation(i,j) = _correlation(j,i) = pearson;
            }
        }
    }

    double test(uint u, uint v, std::vector<uint> S) const
    {
        // Return NaN if any of the correlation coefficients needed for calculation is NaN
        arma::mat C_sub;
        arma::uvec ind(S.size() + 2);
        ind(0) = u;
        ind(1) = v;
        uint i, j;
        for (i = 0; i < S.size(); ++i) ind(i + 2) = S[i];
        C_sub = _correlation.submat(ind, ind);
        for (i = 0; i < C_sub.n_rows; ++i)
            for (j = 0; j < C_sub.n_cols; ++j)
                if ((boost::math::isnan)(C_sub(i, j)))
                    return std::numeric_limits<double>::quiet_NaN();

        // Calculate (absolute value of) z statistic
        #define CUT_THR 0.9999999
        double r, absz;
        //dout.level(3) << " Performing independence test for conditioning set of size " << S.size() << std::endl;
        if (S.empty())
            r = _correlation(u, v);
        else if (S.size() == 1)
            r = (C_sub(0, 1) - C_sub(0, 2) * C_sub(1, 2))/sqrt((1 - C_sub(1, 2)*C_sub(1, 2)) * (1 - C_sub(0, 2)*C_sub(0, 2)));
        else {
            arma::mat PM;
            pinv(PM, C_sub);
            // TODO include error handling
            r = - PM(0, 1)/sqrt(PM(0, 0) * PM(1, 1));
        }
        // Absolute value of r, respect cut threshold
        r = std::min(CUT_THR, std::abs(r));

        // Absolute value of z statistic
        // Note: log1p for more numerical stability, see "Aaux.R"; log1p is also available in
        // header <cmath>, but probably only on quite up to date headers (C++11)?
        absz = sqrt(nr_variables - S.size() - 3.0) * 0.5 * boost::math::log1p(2*r/(1 - r));

        // Calculate p-value to z statistic (based on standard normal distribution)
        boost::math::normal distN;
        return (2*boost::math::cdf(boost::math::complement(distN, absz)));
    }

};

std::vector<std::vector<double>> read_data() {
    // std::freopen("Scerevisiae.csv", "r", stdin);
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


int main(int argc, char* argv[])
{
    std::ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.precision(10);


    auto data = read_data();

    PCAlgorithm alg(data.size());

    alg.build_correlation_matrix(data);

    alg._correlation.print(cout);

    // std::vector<uint> sep1 = {5};
    // std::vector<uint> sep2 = {3};
    // std::vector<uint> sep3 = {5};

    // cout << alg.test(3,4, sep1) << endl;
    // cout << alg.test(4,5, sep2) << endl;
    // cout << alg.test(0,1, sep3) << endl;



    cout.flush();
    return 0;
}
