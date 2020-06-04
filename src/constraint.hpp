/**
 * Auxiliary methods for constraint-based algorithms
 *
 * @author Alain Hauser <alain.hauser@biology.unibe.ch>
 */
#pragma once

#include <memory>
#include <vector>

#include "armadillo"

/**
 * Virtual base class for conditional independence tests
 */
class IndepTest {
 public:
    /**
     * Virtual destructor
     *
     * Needed because of different storage of data.
     */
    virtual ~IndepTest() {}

    /**
     * Virtual test function: must be implemented in all derived classes
     *
     * @param   u   First variable index
     * @param   v   Second variable index
     * @param   S   Conditioning set
     * @return  p-value for independence test of X_u and X_v given X_S
     */
    virtual double test(int u, int v, std::vector<int>& S) const = 0;
};

/**
 * Conditional independence test for Gaussian data
 */
class IndepTestGauss : public IndepTest {
 protected:
    /**
     * Sufficient statistic for easier access in independence test
     */
    int _sampleSize;
    std::shared_ptr<arma::mat> _correlation;

 public:
    IndepTestGauss() = default;

    IndepTestGauss(int sampleSize, std::shared_ptr<arma::mat> cor) : _sampleSize(sampleSize), _correlation(cor) {}

    virtual double test(int u, int v, std::vector<int>& S) const;
};
