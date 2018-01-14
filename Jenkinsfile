/*
 * daquiri Jenkinsfile
 */

project = "daquiri"

images = [
    'centos-gcc6': [
        'name': 'essdmscdm/centos-gcc6-build-node:0.3.4',
        'sh': '/usr/bin/scl enable rh-python35 devtoolset-6 -- /bin/bash'
    ],
    'fedora': [
        'name': 'essdmscdm/fedora-build-node:0.4.2',
        'sh': 'sh'
    ],
    'ubuntu1604': [
        'name': 'essdmscdm/ubuntu16.04-build-node:0.0.2',
        'sh': 'sh'
    ],
    'ubuntu1710': [
        'name': 'essdmscdm/ubuntu17.10-build-node:0.0.3',
        'sh': 'sh'
    ]
]

base_container_name = "${project}-${env.BRANCH_NAME}-${env.BUILD_NUMBER}"

def failure_function(exception_obj, failureMessage) {
    def toEmails = [[$class: 'DevelopersRecipientProvider']]
    emailext body: '${DEFAULT_CONTENT}\n\"' + failureMessage + '\"\n\nCheck console output at $BUILD_URL to view the results.', recipientProviders: toEmails, subject: '${DEFAULT_SUBJECT}'
    throw exception_obj
}

def Object container_name(image_key) {
    return "${base_container_name}-${image_key}"
}

def docker_dependencies(image_key) {
    def conan_remote = "ess-dmsc-local"
    def custom_sh = images[image_key]['sh']
    sh """docker exec ${container_name(image_key)} ${custom_sh} -c \"
        mkdir build
        cd build
        conan remote add \
            --insert 0 \
            ${conan_remote} ${local_conan_server}
        conan install --file=../${project}/conanfile.txt --build=missing
    \""""
}

def docker_cmake(image_key) {
    cmake_exec = "/home/jenkins/build/bin/cmake"
    def custom_sh = images[image_key]['sh']
    sh """docker exec ${container_name(image_key)} ${custom_sh} -c \"
        cd build
        ${cmake_exec} --version
        ${cmake_exec} -DCOV=on -DDAQuiri_config=1 -DDAQuiri_cmd=1 -DDAQuiri_gui=0 \
                    -DDAQuiri_enabled_producers=DummyDevice\\;MockProducer\\;DetectorIndex \
                    ../${project}
    \""""
}

def docker_build(image_key) {
    def custom_sh = images[image_key]['sh']
    sh """docker exec ${container_name(image_key)} ${custom_sh} -c \"
        cd build
        make --version
        make
    \""""
}

def docker_tests(image_key) {
    def custom_sh = images[image_key]['sh']
    dir("${project}/tests") {
        try {
            sh """docker exec ${container_name(image_key)} ${custom_sh} -c \"
                cd build
                make run_tests
            \""""
        } catch(e) {
            sh "docker cp ${container_name(image_key)}:/home/jenkins/build/test/unit_tests_run.xml unit_tests_run.xml"
            junit 'unit_tests_run.xml'
            failure_function(e, 'Run tests (${container_name(image_key)}) failed')
        }
    }
}

def Object get_container(image_key) {
    def image = docker.image(images[image_key]['name'])
    def container = image.run("\
        --name ${container_name(image_key)} \
        --tty \
        --network=host \
        --env http_proxy=${env.http_proxy} \
        --env https_proxy=${env.https_proxy} \
        --env local_conan_server=${env.local_conan_server} \
        ")
    return container
}

def get_pipeline(image_key)
{
    return {
        stage("${image_key}") {
            try {
                def container = get_container(image_key)
                def custom_sh = images[image_key]['sh']

                // Copy sources to container and change owner and group.
                dir("${project}") {
                    sh "docker cp code ${container_name(image_key)}:/home/jenkins/${project}"
                    sh """docker exec --user root ${container_name(image_key)} ${custom_sh} -c \"
                        chown -R jenkins.jenkins /home/jenkins/${project}
                        \""""
                }

                try {
                    docker_dependencies(image_key)
                } catch (e) {
                    failure_function(e, "Get dependencies for ${image_key} failed")
                }

                try {
                    docker_cmake(image_key)
                } catch (e) {
                    failure_function(e, "CMake for ${image_key} failed")
                }

                try {
                    docker_build(image_key)
                } catch (e) {
                    failure_function(e, "Build for ${image_key} failed")
                }

                //docker_tests(image_key)
            } catch(e) {
                failure_function(e, "Unknown build failure for ${image_key}")
            } finally {
                sh "docker stop ${container_name(image_key)}"
                sh "docker rm -f ${container_name(image_key)}"
            }
        }
    }
}

node('docker') {
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
    //builders['MocOSX'] = get_osx_pipeline()
    
    parallel builders

    // Delete workspace when build is done
    cleanWs()
}
