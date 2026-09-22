pipeline {
    agent {
        label 'Broker-agent'
    }

    options {
        buildDiscarder(
            logRotator(
                numToKeepStr: '20',
                artifactNumToKeepStr: '10'
            )
        )
    }

    environment {
        GITHUB_TOKEN = credentials('automation')
        REPO_OWNER   = 'GarudaKurt'
        REPO_NAME    = 'scripting'
    }

    stages {

        stage('Show PR Info') {
            steps {
                echo "===================================================="
                echo "                 Pull Request Info                  "
                echo "===================================================="
                echo "PR Number:    #${env.CHANGE_ID}"
                echo "Title:        ${env.CHANGE_TITLE}"
                echo "Source:       ${env.CHANGE_BRANCH}"
                echo "Target:       ${env.CHANGE_TARGET}"
                echo "===================================================="
            }
        }

        stage('C++ Build') {
            steps {
                dir('cplusplus') {
                    sh 'bazel build //...'
                }
            }
        }

        stage('C++ Unit Tests') {
            steps {
                dir('cplusplus') {
                    sh 'bazel test //... --test_output=all'
                }
            }
        }

        stage('Ruby Checks') {
            steps {
                dir('ruby_scripts') {
                    sh 'ruby -c observer.rb'
                }
            }
        }

        stage('Ruby Integration Tests') {
            when {
                expression {
                    fileExists('ruby_scripts/tests')
                }
            }
            steps {
                dir('ruby_scripts') {
                    sh 'ruby tests/run_all.rb'
                }
            }
        }

        stage('Test GitHub Authentication') {
            steps {
                withCredentials([
                    usernamePassword(
                        credentialsId: 'automation',
                        usernameVariable: 'GITHUB_USER',
                        passwordVariable: 'GITHUB_TOKEN'
                    )
                ]) {
                    sh '''
                        set -e

                        echo "Testing GitHub authentication..."

                        curl -sS \
                            -H "Authorization: Bearer ${GITHUB_TOKEN}" \
                            -H "Accept: application/vnd.github+json" \
                            -H "X-GitHub-Api-Version: 2022-11-28" \
                            https://api.github.com/user | jq '{login, id}'
                    '''
                }
            }
        }
    }

    post {
        success {
            echo "Pipeline completed successfully."
        }

        failure {
            echo "Pipeline failed — PR will not be merged."
        }

        always {
            cleanWs()
        }
    }
}
