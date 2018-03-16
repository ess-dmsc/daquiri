project = "daquiri"

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

def Object container_name(image_key) {
    return "${base_container_name}-${image_key}"
}

def failure_function(exception_obj, failureMessage) {
    def toEmails = [[$class: 'DevelopersRecipientProvider']]
    emailext body: '${DEFAULT_CONTENT}\n\"' + failureMessage + '\"\n\nCheck console output at $BUILD_URL to view the results.', recipientProviders: toEmails, subject: '${DEFAULT_SUBJECT}'
    slackSend color: 'danger', message: "${project}: " + failureMessage
    throw exception_obj
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
    def custom_sh = images[image_key]['sh']

    // Copy sources to container and change owner and group.
    dir("${project}") {
        sh "docker cp code ${container_name(image_key)}:/home/jenkins/${project}"
        sh """docker exec --user root ${container_name(image_key)} ${custom_sh} -c \"
                        chown -R jenkins.jenkins /home/jenkins/${project}
                        \""""
    }
}

def docker_dependencies(image_key) {
    def custom_sh = images[image_key]['sh']
    def conan_remote = "ess-dmsc-local"
    def dependencies_script = """
        mkdir build
        cd build
        conan remote add \\
            --insert 0 \\
            ${conan_remote} ${local_conan_server}
                    """
    try {
        sh "docker exec ${container_name(image_key)} ${custom_sh} -c \"${dependencies_script}\""
    } catch (e) {
        failure_function(e, "Get dependencies for (${container_name(image_key)}) failed")
    }
}

def docker_cmake(image_key, xtra_flags) {
    def cmake_exec = "/home/jenkins/build/bin/cmake"
    def custom_sh = images[image_key]['sh']
    def configure_script = """
        cd build
        ${cmake_exec} --version
        ${cmake_exec} -DDAQuiri_config=1 -DDAQuiri_cmd=1 -DDAQuiri_gui=0 \
              -DDAQuiri_enabled_producers=DummyDevice\\;MockProducer\\;DetectorIndex\\;ESSStream \
              ${xtra_flags} \
              ../${project}
        """

    try {
        sh "docker exec ${container_name(image_key)} ${custom_sh} -c \"${configure_script}\""
    } catch (e) {
        failure_function(e, "CMake step for (${container_name(image_key)}) failed")
    }
}

def docker_build(image_key) {
    def custom_sh = images[image_key]['sh']
    def build_script = """
        cd build
        make --version
        make VERBOSE=1
                  """
    try {
        sh "docker exec ${container_name(image_key)} ${custom_sh} -c \"${build_script}\""
    } catch (e) {
        failure_function(e, "Build step for (${container_name(image_key)}) failed")
    }
}

def docker_tests(image_key) {
    def custom_sh = images[image_key]['sh']
    def test_script = """
                cd build
                . ./activate_run.sh
                make run_tests
                ./bin/daquiri_cmd
                    """
    dir("${image_key}") {
        try {
            sh "docker exec ${container_name(image_key)} ${custom_sh} -c \"${test_script}\""
        } catch (e) {
            sh "docker cp ${container_name(image_key)}:/home/jenkins/build/test/unit_tests_run.xml unit_tests_run.xml"
            junit 'unit_tests_run.xml'
            failure_function(e, "Test step for (${container_name(image_key)}) failed")
        }
    }
}

def get_pipeline(image_key) {
    return {
        stage("${image_key}") {
            try {
                create_container(image_key)
                docker_copy_code(image_key)

                docker_dependencies(image_key)

                def xtra_flags = ""
                docker_cmake(image_key, xtra_flags)

                docker_build(image_key)
                docker_tests(image_key)

            } catch (e) {
                failure_function(e, "Unknown build failure for ${image_key}")
            } finally {
                sh "docker stop ${container_name(image_key)}"
                sh "docker rm -f ${container_name(image_key)}"
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
//                    try {
//                        sh "conan install --build=outdated ../code/conanfile.txt"
//                    } catch (e) {
//                        failure_function(e, 'MacOSX / getting dependencies failed')
//                    }

                    try {
                        sh "cmake -DDAQuiri_config=1 -DDAQuiri_cmd=1 -DDAQuiri_gui=0 \
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

    stage('Checkout') {
        dir("${project}/code") {
            try {
                scm_vars = checkout scm
                sh "git submodule update --init"
            } catch (e) {
                failure_function(e, 'Checkout failed')
            }
        }
    }

    def builders = [:]
    for (x in images.keySet()) {
        def image_key = x
        builders[image_key] = get_pipeline(image_key)
    }
    builders['macOS'] = get_macos_pipeline()

    parallel builders

    // Delete workspace if build was successful
    cleanWs()
}
