project = "daquiri"
coverage_on = "ubuntu18"

images = [
        'ubuntu18' : [
                'name'  : 'essdmscdm/ubuntu18.04-build-node:1.2.0'
        ]
]

base_container_name = "${project}-${env.BRANCH_NAME}-${env.BUILD_NUMBER}"

def failure_function(exception_obj, failureMessage) {
    def toEmails = [[$class: 'DevelopersRecipientProvider']]
    emailext body: '${DEFAULT_CONTENT}\n\"' + failureMessage + '\"\n\nCheck console output at $BUILD_URL to view the results.',
            recipientProviders: toEmails,
            subject: '${DEFAULT_SUBJECT}'
    slackSend color: 'danger',
            message: "${base_container_name}: " + failureMessage
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
                        sh "cmake ../code"
                    } catch (e) {
                        failure_function(e, 'MacOSX / CMake failed')
                    }

                    try {
                        sh "make everything -j4"
                    } catch (e) {
                        failure_function(e, 'MacOSX / build failed')
                    }

                    try {
                        sh "source ./activate_run.sh && \
                            tests/unit_tests && \
                            tests/system_test"
                    } catch (e) {
                        failure_function(e, 'MacOSX / tests failed')
                    }
                }

            }
        }
    }
}

def Object container_name(image_key) {
    return "${base_container_name}-${image_key}"
}

def create_container(image_key) {
    def image = docker.image(images[image_key]['name'])
    def container = image.run("\
            --name ${container_name(image_key)} \
        --tty \
        --network=host \
        --env http_proxy=${env.http_proxy} \
        --env https_proxy=${env.https_proxy} \
        --env local_conan_server=${env.local_conan_server} \
          ")
}

def docker_copy_code(image_key) {
    sh "docker cp ${project}_code ${container_name(image_key)}:/home/jenkins/${project}"
    sh """docker exec --user root ${container_name(image_key)} sh -c \"
                        chown -R jenkins.jenkins /home/jenkins/${project}
                        \""""
}

def docker_clone(image_key) {
    def clone_script = """
        git clone \
            --branch ${env.BRANCH_NAME} \
            https://github.com/ess-dmsc/${project}.git /home/jenkins/${project}
        cd ${project}
        """
    sh "docker exec ${container_name(image_key)} bash -e -c \"${clone_script}\""
}

def docker_dependencies(image_key) {
    def conan_remote = "ess-dmsc-local"
    def dependencies_script = """
        mkdir ${project}/build
        cd ${project}/build
        conan remote add \\
            --insert 0 \\
            ${conan_remote} ${local_conan_server}
                    """
    sh "docker exec ${container_name(image_key)} bash -e -c \"${dependencies_script}\""
}

def docker_cmake(image_key, xtra_flags) {
    def configure_script = """
        cd ${project}/build
        cmake ${xtra_flags} ..
        """

    sh "docker exec ${container_name(image_key)} bash -e -c \"${configure_script}\""
}

def docker_build(image_key) {
    def build_script = """
        cd ${project}/build
        make everything -j4
        """
    sh "docker exec ${container_name(image_key)} bash -e -c \"${build_script}\""
}

def docker_tests(image_key) {
    def test_script = """
                cd ${project}/build
                . ./activate_run.sh
                make run_tests
                """
    sh "docker exec ${container_name(image_key)} bash -e -c \"${test_script}\""
}

def docker_tests_coverage(image_key) {
    abs_dir = pwd()

    try {
        sh """docker exec ${container_name(image_key)} bash -e -c \"
                cd ${project}/build
                . ./activate_run.sh
                make run_tests && make coverage
            \""""
        sh "docker cp ${container_name(image_key)}:/home/jenkins/${project} ./"
    } catch(e) {
        sh "docker cp ${container_name(image_key)}:/home/jenkins/${project}/build/test ./"
        junit 'tests/test_results.xml'
        failure_function(e, 'Run tests (${container_name(image_key)}) failed')
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
}


def get_pipeline(image_key) {
    return {
        stage("${image_key}") {
                try {
                    create_container(image_key)

                    docker_copy_code(image_key)
                    //docker_clone(image_key)

                    docker_dependencies(image_key)

                    if (image_key == coverage_on) {
                        docker_cmake(image_key, "-DCOV=1")
                    } else {
                        docker_cmake(image_key, "")
                    }

                    docker_build(image_key)

                    if (image_key == coverage_on) {
                        docker_tests_coverage(image_key)
                    } else {
                        docker_tests(image_key)
                    }

                } finally {
                    sh "docker stop ${container_name(image_key)}"
                    sh "docker rm -f ${container_name(image_key)}"
                }
        }
    }
}

node('docker') {
    cleanWs()

    dir("${project}_code") {
        stage('Checkout') {
            try {
                scm_vars = checkout scm
            } catch (e) {
                failure_function(e, 'Checkout failed')
            }
        }

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

    def builders = [:]
    for (x in images.keySet()) {
        def image_key = x
        builders[image_key] = get_pipeline(image_key)
    }
    builders['macOS'] = get_macos_pipeline()

    try {
        parallel builders
    } catch (e) {
        failure_function(e, 'Job failed')
    }

    // Delete workspace if build was successful
    cleanWs()
}
