
properties([
    parameters([
        choice(
            name: 'BUILD_TYPE',
            choices: ['Release', 'Debug', 'RelWithDebInfo'],
            description: 'CMake and Conan build type'
        ),

        string(
            name: 'DOCKER_IMAGE',
            defaultValue: 'yap7kor02438/myappconan',
            description: 'Docker Hub repository name'
        ),

        booleanParam(
            name: 'PUSH_DOCKER_IMAGE',
            defaultValue: true,
            description: 'Push image to Docker Hub'
        ),

        booleanParam(
            name: 'GENERATE_SBOM',
            defaultValue: true,
            description: 'Generate CycloneDX SBOM'
        ),

        booleanParam(
            name: 'FORCE_CONAN_UPDATE',
            defaultValue: false,
            description: 'Check Conan remotes for newer package revisions'
        )
    ]),

    pipelineTriggers([
        // Run once every weekday.
        cron('H H * * 1-5'),

        // Uncomment when GitHub webhook integration is configured.
        // githubPush()
    ]),

    buildDiscarder(
        logRotator(
            numToKeepStr: '5',
            artifactNumToKeepStr: '5'
        )
    ),

    disableConcurrentBuilds()
])
node('built-in') {

    def imageName = "yap7kor/myappconan:${BUILD_NUMBER}"

    stage('Checkout') {
        checkout scm
    }

    stage('Show Environment') {
        sh '''
            gcc --version
            conan --version
            cmake --version
            docker --version
        '''
    }

    stage('Conan Install') {
        lock(resource: "conan-cache-${env.NODE_NAME}") {
            sh """
                conan install . \
                --build=missing \
                -s build_type=${params.BUILD_TYPE}
          """
        }
   }

    stage('Configure') {
        sh """
            cmake -S . -B build \
            -DCMAKE_TOOLCHAIN_FILE=build/${params.BUILD_TYPE}/generators/conan_toolchain.cmake \
            -DCMAKE_BUILD_TYPE=${params.BUILD_TYPE}
       """
    }

    stage('Build') {
        sh '''
            cmake --build build
        '''
    }

    stage('Test') {
        sh '''
            ctest --test-dir build
        '''
    }

    stage('Generate SBOM') {
        sh '''
            conan sbom:cyclonedx --format 1.4_json . > sbom.json
        '''
    }

    stage('Cppcheck') {
        sh '''
            cppcheck src
        '''
    }

    stage('Docker Build') {

        script {
            dockerImage = docker.build(imageName)
        }
    }

    stage('Push Image') {

        docker.withRegistry(
            'https://index.docker.io/v1/',
            'dockerhub-creds'
        ) {
            dockerImage.push()
            dockerImage.push('latest')
        }
    }
}
