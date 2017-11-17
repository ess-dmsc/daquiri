/*
 * daquiri Jenkinsfile
 */
def failure_function(exception_obj, failureMessage) {
    def toEmails = [[$class: 'DevelopersRecipientProvider']]
    emailext body: '${DEFAULT_CONTENT}\n\"' + failureMessage + '\"\n\nCheck console output at $BUILD_URL to view the results.', recipientProviders: toEmails, subject: '${DEFAULT_SUBJECT}'
    throw exception_obj
}

node ("qt && boost && fedora") {
    cleanWs()

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
                    cmake -DCOV=on -DDAQuiri_cmd=1 -DDAQuiri_gui=1 \
                    -DDAQuiri_enabled_producers=DummyDevice\\;MockProducer\\;ESSStream \
                    ../code"
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

        try {
            stage("Run test") {
                sh "make run_tests"
                junit 'tests/test_results.xml'
                // Publish test coverage results.
                sh "make coverage"
                step([
                    $class: 'CoberturaPublisher',
                    autoUpdateHealth: false,
                    autoUpdateStability: false,
                    coberturaReportFile: 'tests/coverage/coverage.xml',
                    failUnhealthy: false,
                    failUnstable: false,
                    maxNumberOfBuilds: 0,
                    onlyStable: false,
                    sourceEncoding: 'ASCII',
                    zoomCoverageChart: false
                ])
          }
        } catch (e) {
            junit 'tests/test_results.xml'
            failure_function(e, 'Tests failed')
        }

    }
}
