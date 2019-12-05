FROM quay.io/pypa/manylinux1_x86_64

# Install cmake3
RUN /opt/python/cp36-cp36m/bin/pip install cmake==3.13.3
RUN ln -sfn /opt/python/cp36-cp36m/bin/cmake /usr/bin/cmake

WORKDIR /tmp
# Install armadillo
RUN curl -L http://sourceforge.net/projects/arma/files/armadillo-9.800.2.tar.xz -O
RUN yum -y install xz
RUN unxz armadillo-9.800.2.tar.xz
RUN tar -xf armadillo-9.800.2.tar
WORKDIR armadillo-9.800.2
RUN cmake .
RUN make
RUN make install

WORKDIR /tmp
# Install boost
RUN curl -L https://dl.bintray.com/boostorg/release/1.66.0/source/boost_1_66_0.tar.gz -O
RUN tar -xf boost_1_66_0.tar.gz
WORKDIR boost_1_66_0
RUN ./bootstrap.sh
RUN ./b2 link=static cxxflags=-fPIC install ; exit 0
