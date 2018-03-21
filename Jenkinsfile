project = "daquiri"
coverage_on = "ubuntu1710"

images = [
        'centos7-gcc6': [
                'name': 'essdmscdm/centos7-gcc6-build-node:2.1.0',
                'sh'  : '/usr/bin/scl enable rh-python35 devtoolset-6 -- /bin/bash'
        ],
        'fedora25'    : [
                'name': 'essdmscdm/fedora25-build-node:1.0.0',
                'sh'  : 'sh'
        ],
        'ubuntu1604'  : [
                'name': 'essdmscdm/ubuntu16.04-build-node:2.1.0',
                'sh'  : 'sh'
        ],
        'ubuntu1710'  : [
                'name': 'essdmscdm/ubuntu17.10-build-node:2.0.0',
                'sh'  : 'sh'
        ]
]

base_container_name = "${project}-${env.BRANCH_NAME}-${env.BUILD_NUMBER}"

def failure_function(exception_obj, failureMessage) {
    def toEmails = [[$class: 'DevelopersRecipientProvider']]
    emailext body: '${DEFAULT_CONTENT}\n\"' + failureMessage + '\"\n\nCheck console output at $BUILD_URL to view the results.',
            recipientProviders: toEmails,
            subject: '${DEFAULT_SUBJECT}'
    slackSend color: 'danger',
            message: "${project}-${env.BRANCH_NAME}: " + failureMessage
    throw exception_obj
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

def docker_clone(image_key) {
    def custom_sh = images[image_key]['sh']
    sh """docker exec ${container_name(image_key)} ${custom_sh} -c \"
        git clone \
            --branch ${env.BRANCH_NAME} \
            https://github.com/ess-dmsc/event-formation-unit.git /home/jenkins/${project}
        cd ${project}
        git submodule update --init
    \""""
}

def docker_dependencies(image_key) {
    def custom_sh = images[image_key]['sh']
    def conan_remote = "ess-dmsc-local"
    def dependencies_script = """
        mkdir ${project}/build
        cd ${project}/build
        conan remote add \\
            --insert 0 \\
            ${conan_remote} ${local_conan_server}
        conan install --build=outdated ../${project}/conanfile.txt
                    """
    sh "docker exec ${container_name(image_key)} ${custom_sh} -c \"${dependencies_script}\""
}

def docker_cmake(image_key, xtra_flags) {
    def cmake_exec = "/home/jenkins/build/bin/cmake"
    def custom_sh = images[image_key]['sh']
    def configure_script = """
        cd ${project}/build
        ${cmake_exec} --version
        ${cmake_exec} -DCONAN=MANUAL -DDAQuiri_config=1 -DDAQuiri_cmd=1 -DDAQuiri_gui=0 \
              -DDAQuiri_enabled_producers=DummyDevice\\;MockProducer\\;DetectorIndex\\;ESSStream \
              ${xtra_flags} \
              ..
        """

    sh "docker exec ${container_name(image_key)} ${custom_sh} -c \"${configure_script}\""
}

def docker_build(image_key) {
    def custom_sh = images[image_key]['sh']
    def build_script = """
        cd ${project}/build
        make --version
        make VERBOSE=1
                  """
    sh "docker exec ${container_name(image_key)} ${custom_sh} -c \"${build_script}\""
}

def docker_tests(image_key) {
    def custom_sh = images[image_key]['sh']
    def test_script = """
                cd ${project}/build
                . ./activate_run.sh
                make run_tests
                ./bin/daquiri_cmd
                    """
    sh "docker exec ${container_name(image_key)} ${custom_sh} -c \"${test_script}\""
}

def docker_tests_coverage(image_key) {
    def custom_sh = images[image_key]['sh']
    abs_dir = pwd()

    try {
        sh """docker exec ${container_name(image_key)} ${custom_sh} -c \"
                cd ${project}/build
                . ./activate_run.sh
                make VERBOSE=ON
                make coverage VERBOSE=ON
                ./bin/daquiri_cmd
            \""""
        sh "docker cp ${container_name(image_key)}:/home/jenkins/${project} ./"
    } catch(e) {
        sh "docker cp ${container_name(image_key)}:/home/jenkins/${project}/build/test ./"
        junit 'test/unit_tests_run.xml'
        failure_function(e, 'Run tests (${container_name(image_key)}) failed')
    }

    dir("${project}/build") {
        junit 'test/unit_tests_run.xml'
        sh "../jenkins/redirect_coverage.sh ./test/coverage/coverage.xml ${abs_dir}/${project}"

        step([
                $class: 'CoberturaPublisher',
                autoUpdateHealth: true,
                autoUpdateStability: true,
                coberturaReportFile: 'test/coverage/coverage.xml',
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
            node("docker") {
                try {
                    create_container(image_key)
                    docker_clone(image_key)

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
                        sh "git submodule update --init"
                    } catch (e) {
                        failure_function(e, 'MacOSX / Checkout failed')
                    }
                }

                dir("${project}/build") {
                    try {
                        sh "conan install --build=outdated ../code/conanfile.txt"
                    } catch (e) {
                        failure_function(e, 'MacOSX / getting dependencies failed')
                    }

                    try {
                        sh "cmake -DCONAN=MANUAL -DDAQuiri_config=1 -DDAQuiri_cmd=1 -DDAQuiri_gui=0 \
                            -DDAQuiri_enabled_producers=DummyDevice\\;MockProducer\\;DetectorIndex\\;ESSStream ../code"
                    } catch (e) {
                        failure_function(e, 'MacOSX / CMake failed')
                    }

                    try {
                        sh "make VERBOSE=1"
                        sh "make run_tests"
                        sh "./bin/daquiri_cmd"
                    } catch (e) {
                        junit 'test/unit_tests_run.xml'
                        failure_function(e, 'MacOSX / build+test failed')
                    }
                }

            }
        }
    }
}

node('docker') {
    cleanWs()

    dir("${project}/code") {
        stage('Checkout') {
            try {
                scm_vars = checkout scm
                sh "git submodule update --init"
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
