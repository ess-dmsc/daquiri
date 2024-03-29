@Library('ecdc-pipeline')
import ecdcpipeline.ContainerBuildNode
import ecdcpipeline.PipelineBuilder

project = "daquiri"
coverage_on = "centos"

// Set number of old builds to keep.
properties([[
  $class: 'BuildDiscarderProperty',
  strategy: [
    $class: 'LogRotator',
    artifactDaysToKeepStr: '',
    artifactNumToKeepStr: '10',
    daysToKeepStr: '',
    numToKeepStr: ''
  ]
]]);

container_build_nodes = [
  'centos': ContainerBuildNode.getDefaultContainerBuildNode('centos7-gcc11'),
  'ubuntu2204': ContainerBuildNode.getDefaultContainerBuildNode('ubuntu2204')
]

pipeline_builder = new PipelineBuilder(this, container_build_nodes)

def failure_function(exception_obj, failureMessage) {
    def toEmails = [[$class: 'DevelopersRecipientProvider']]
    emailext body: '${DEFAULT_CONTENT}\n\"' + failureMessage + '\"\n\nCheck console output at $BUILD_URL to view the results.',
            recipientProviders: toEmails,
            subject: '${DEFAULT_SUBJECT}'
    throw exception_obj
}

def get_macos_pipeline() {
    return {
        stage("macOS") {
            node("macos") {
                // Delete workspace when build is done
                cleanWs()

                dir("${project}/code") {
                    try {
                        checkout scm
                    } catch (e) {
                        failure_function(e, 'MacOSX / Checkout failed')
                    }
                }

                dir("${project}/build") {
                    try {
                        // Remove existing CLI11 because of case insensitive filesystem issue
                        sh "conan remove -f 'CLI11*' && \
                            CFLAGS='-Wno-error=implicit-function-declaration' cmake ../code"
                    } catch (e) {
                        failure_function(e, 'MacOSX / CMake failed')
                    }

                    try {
                        sh "make everything -j8"
                    } catch (e) {
                        failure_function(e, 'MacOSX / build failed')
                    }

                    try {
                        sh ". ./activate_run.sh && \
                            tests/unit_tests && \
                            tests/system_test && \
                            bin/gui_tests"
                    } catch (e) {
                        failure_function(e, 'MacOSX / tests failed')
                    }
                }

            }
        }
    }
}

builders = pipeline_builder.createBuilders { container ->

    pipeline_builder.stage("${container.key}: checkout") {
        dir(pipeline_builder.project) {
            scm_vars = checkout scm
        }
        // Copy source code to container
        container.copyTo(pipeline_builder.project, pipeline_builder.project)
    }  // stage

    pipeline_builder.stage("${container.key}: configure conan") {
        container.sh """
            cd ${project}
            mkdir build
            cd build
        """
    }  // stage

    pipeline_builder.stage("${container.key}: CMake") {
        def coverage_flag
        if (container.key == coverage_on) {
            coverage_flag = "-DCOV=1"
        } else {
            coverage_flag = ""
        }

        container.sh """
            cd ${project}/build
            CFLAGS=-Wno-error=implicit-function-declaration cmake .. ${coverage_flag}
        """
    }  // stage

    pipeline_builder.stage("${container.key}: build") {
        container.sh """
            cd ${project}/build
            . ./activate_run.sh
            make everything -j4
        """
    }  // stage

    pipeline_builder.stage("${container.key}: test") {
        if (container.key == coverage_on) {
            abs_dir = pwd()

            try {
                container.sh """
                        cd ${project}/build
                        . ./activate_run.sh
                        make run_tests && make coverage
                    """
                container.copyFrom("${project}", '.')
            } catch(e) {
                container.copyFrom("${project}/build/test", '.')
                junit 'tests/test_results.xml'
                failure_function(e, "Run tests (${container_name(image_key)}) failed")
            }
            dir("${project}/build") {
                junit 'tests/test_results.xml'
                sh "../jenkins/redirect_coverage.sh ./tests/coverage/coverage.xml ${abs_dir}/${project}/source/"

                step([
                        $class: 'CoberturaPublisher',
                        autoUpdateHealth: true,
                        autoUpdateStability: true,
                        coberturaReportFile: 'tests/coverage/coverage.xml',
                        failUnhealthy: false,
                        failUnstable: false,
                        maxNumberOfBuilds: 0,
                        onlyStable: false,
                        sourceEncoding: 'ASCII',
                        zoomCoverageChart: true
                ])
            }
        } else {
            // Run tests.
            container.sh """
                cd ${project}/build
                . ./activate_run.sh
                make run_tests
            """
        }  // if/else
    }  // stage
}  // createBuilders

// Script actions start here
node('docker') {
    // Delete workspace when build is done.
    cleanWs()

    dir("${project}") {
        stage('Checkout') {
            try {
                scm_vars = checkout scm
            } catch (e) {
                failure_function(e, 'Checkout failed')
            }
        }

        // skip build process if message contains '[ci skip]'
        pipeline_builder.abortBuildOnMagicCommitMessage()

        stage("Static analysis") {
            try {
                sh "cloc --by-file --xml --out=cloc.xml ."
                sh "xsltproc jenkins/cloc2sloccount.xsl cloc.xml > sloccount.sc"
                sloccountPublish encoding: '', pattern: ''
            } catch (e) {
                failure_function(e, 'Static analysis failed')
            }
        }
    }

    if (env.ENABLE_MACOS_BUILDS.toUpperCase() == 'TRUE') {
        // Add macOS pipeline to builders
        builders['macOS'] = get_macos_pipeline()
    }

    try {
        timeout(time: 2, unit: 'HOURS') {
            // run all builders in parallel
            parallel builders
        }
    } catch (e) {
        pipeline_builder.handleFailureMessages()
        throw e
    }
}
