/*
 * daquiri Jenkinsfile
 */

node ("boost && fedora") {

    dir("code") {
        stage("Checkout projects") {
            checkout scm
            sh "git submodule update --init"
        }
    }

    dir("build") {
        stage("Run CMake") {
            sh 'rm -rf ./*'
            sh "HDF5_ROOT=$HDF5_ROOT \
                CMAKE_PREFIX_PATH=$HDF5_ROOT \
                /opt/cmake/cmake-3.7.1-Linux-x86_64/bin/cmake ../code/src"
        }

        stage("Build project") {
            sh "make VERBOSE=1"
        }

    }
}
