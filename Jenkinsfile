pipeline { 
    agent { 
        label 'Broker-agent'
    }

    options { 
        buildDiscarder (
            logRotator(
                numToKeepStr: '20',
                artifactNumToKeepStr: '10'
            )
        )
    }

    stages {
        stage('Checkout') {
            steps {
                checkout scm
            }
        }
        stage('Build & Test') {
            parallel {
                stage('C++ Pipeline') {
                    dir('cplusplus') {
                        echo 'Start build and test c++'
                        sh 'bazel build //...'
                        sh 'bazel test //... --test_output=all'
                    }
                }
                stage('Ruby Pipeline') {
                    steps {
                        dir('ruby_scripts') {
                            echo 'Ruby scripts start observing...'
                            sh 'ruby -c observer.rb'
                        }
                    }
                }
            }
        }
    }
}


