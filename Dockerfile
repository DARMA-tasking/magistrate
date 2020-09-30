FROM lifflander1/vt:alpine-final
LABEL maintainer="Jonathan Lifflander <jliffla@sandia.gov>"

COPY . /usr/src/checkpoint

WORKDIR /usr/src

RUN /bin/bash -c 'source $HOME/.bashrc && \
 source /usr/share/spack/share/spack/setup-env.sh && \
 spack env activate clang-mpich && \
 export CC=clang && \
 export CXX=clang++ && \
 echo $HTTP_PROXY && \
 echo $HTTPS_PROXY && \
 echo $ALL_PROXY && \
 echo $http_proxy && \
 echo $https_proxy && \
 echo $all_proxy && \
 unset https_proxy &&  \
 unset http_proxy && \
 unset all_proxy && \
 unset HTTPS_PROXY && \
 unset HTTP_PROXY && \
 unset ALL_PROXY && \
 if [ -d "detector" ]; then rm -Rf detector; fi && \
 export CHECKPOINT=$PWD/checkpoint && \
 export CHECKPOINT_BUILD=/usr/build/checkpoint && \
 git clone -b master --depth 1 https://github.com/DARMA-tasking/detector.git && \
 export DETECTOR=$PWD/detector && \
 export DETECTOR_BUILD=/usr/build/detector && \
 echo $SOURCE_COMMIT && \
 cd $DETECTOR_BUILD && \
 mkdir build && \
 cd build && \
 cmake -DCMAKE_INSTALL_PREFIX=$DETECTOR_BUILD/install -DCMAKE_CXX_COMPILER=$CXX -DCMAKE_C_COMPILER=$CC $DETECTOR && \
 make && \
 make install && \
 cd $CHECKPOINT_BUILD && \
 mkdir build && \
 cd build && \
 cmake -DCHECKPOINT_BUILD_TESTS:BOOL=1 -DCHECKPOINT_BUILD_EXAMPLES:BOOL=1 -DCMAKE_INSTALL_PREFIX=$CHECKPOINT_BUILD/install -DCMAKE_CXX_COMPILER=$CXX -DCMAKE_C_COMPILER=$CC -Ddetector_DIR=$DETECTOR_BUILD/install $CHECKPOINT && \
 make && \
 make test && \
 make install'

COPY $DETECTOR_BUILD/ $DETECTOR_BUILD
COPY $CHECKPOINT_BUILD/ $CHECKPOINT_BUILD
