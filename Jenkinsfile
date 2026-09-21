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
        stage('Check agent') {
            steps {
                sh '''
                    echo "==== Agent Information ===="
                    echo "User: $(whoami)"
                    echo "Hostname: $(hostname)"
                    echo "OS:"
                    cat /etc/os-release
                    echo "Java: "
                    java --version
                    echo "Bazel: "
                    bazel --version
                    echo "Ruby: "
                    ruby --version
                '''
            }
        }
    }
}
