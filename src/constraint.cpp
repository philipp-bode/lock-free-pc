/**
 * Auxiliary methods for constraint-based algorithms
 *
 * @author Alain Hauser <alain.hauser@biology.unibe.ch>
 */

#include "constraint.hpp"

#include <algorithm>
#include <iterator>
#include <limits>
#include <utility>

#include "boost/dynamic_bitset.hpp"
#include "boost/math/distributions/normal.hpp"
#include "boost/math/special_functions/fpclassify.hpp"
#include "boost/math/special_functions/log1p.hpp"

double IndepTestGauss::test(int u, int v, std::vector<int>& S) const {
    // Return NaN if any of the correlation coefficients needed for calculation is NaN
    arma::mat C_sub;
    arma::uvec ind(S.size() + 2);
    ind(0) = u;
    ind(1) = v;
    uint i, j;
    for (i = 0; i < S.size(); ++i) ind(i + 2) = S[i];
    C_sub = _correlation->submat(ind, ind);
    for (i = 0; i < C_sub.n_rows; ++i)
        for (j = 0; j < C_sub.n_cols; ++j)
            if ((boost::math::isnan)(C_sub(i, j))) return std::numeric_limits<double>::quiet_NaN();

// Calculate (absolute value of) z statistic
#define CUT_THR 0.9999999
    double r, absz;
    if (S.empty()) {
        r = _correlation->at(u, v);
    } else if (S.size() == 1) {
        r = (C_sub(0, 1) - C_sub(0, 2) * C_sub(1, 2)) /
            sqrt((1 - C_sub(1, 2) * C_sub(1, 2)) * (1 - C_sub(0, 2) * C_sub(0, 2)));
    } else {
        arma::mat PM;
        pinv(PM, C_sub);
        r = -PM(0, 1) / sqrt(PM(0, 0) * PM(1, 1));
    }
    // Absolute value of r, respect cut threshold
    r = std::min(CUT_THR, std::abs(r));

    // Absolute value of z statistic
    // Note: log1p for more numerical stability, see "Aaux.R"; log1p is also available in
    // header <cmath>, but probably only on quite up to date headers (C++11)?
    absz = sqrt(_sampleSize - S.size() - 3.0) * 0.5 * boost::math::log1p(2 * r / (1 - r));

    // Calculate p-value to z statistic (based on standard normal distribution)
    boost::math::normal distN;
    return (2 * boost::math::cdf(boost::math::complement(distN, absz)));
}
