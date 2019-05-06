# Lock-Free-PC
Lock-Free-PC is a load-balanced, lock-free implementation of the PC-algorithm for causal inference from high-dimensional data.

It can be used to discover the skeleton graphs of Causal Bayesian Networks.
## Setup

### Dependencies

To build from this source you need to have the
[GSL](https://www.gnu.org/software/gsl/),
[Boost](https://www.boost.org/) and
[Armadillo](http://arma.sourceforge.net/) installed.

Please feel free to add instructions for your system, especially when it's more
complicated than `brew install ...`.

#### On MacOS (using [homebrew](https://brew.sh/))

To install these dependencies on MacOS using homebrew run:

```sh
brew install gsl
brew install armadillo
brew install boost
```
#### On Ubuntu/Debian

To install these dependencies on Ubuntu/Debian using apt run:

```sh
sudo apt install libgsl-dev
sudo apt install libarmadillo-dev
sudo apt install libboost-all-dev
```

#### Submodules

This project uses the
[concurrentqueue](https://github.com/cameron314/concurrentqueue/tree/8f7e861dd9411a0bf77a6b9de83a47b3424fafba) project
internally. After cloning the repository for the first time, run

```sh
git submodule update --init --recursive
```

to pull into your working directory as well.

### Build

To build the project from source, follow the standard cmake build flow:

```sh
mkdir build
cd build
cmake ..
make
```
