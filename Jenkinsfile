/*
 * daquiri Jenkinsfile
 */
def failure_function(exception_obj, failureMessage) {
    def toEmails = [[$class: 'DevelopersRecipientProvider']]
    emailext body: '${DEFAULT_CONTENT}\n\"' + failureMessage + '\"\n\nCheck console output at $BUILD_URL to view the results.', recipientProviders: toEmails, subject: '${DEFAULT_SUBJECT}'
    throw exception_obj
}

node ("boost && fedora") {
    dir("code") {
        try {
            stage("Checkout projects") {
                checkout scm
                sh "git submodule update --init"
            }
        } catch (e) {
            failure_function(e, 'Checkout failed')
        }
    }

    dir("build") {
        try {
            stage("Run CMake") {
                sh 'rm -rf ./*'
                sh "HDF5_ROOT=$HDF5_ROOT \
                    CMAKE_PREFIX_PATH=$HDF5_ROOT \
                    /opt/cmake/cmake-3.7.1-Linux-x86_64/bin/cmake ../code/src"
            }
        } catch (e) {
            failure_function(e, 'CMake failed')
        }
        
        try {
            stage("Build project") {
                sh "make VERBOSE=1"
            }
        } catch (e) {
            failure_function(e, 'Build failed')
        }

    }
}
