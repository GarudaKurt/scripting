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
        GITHUB_TOKEN = credentials('github-token')
        REPO_OWNER   = 'GarudaKurt'
        REPO_NAME    = 'scripting'
    }

    stages {

        stage('Checkout') {
            steps {
                checkout scm
            }
        }

        stage('Show PR Info') {
            when {
                expression { env.CHANGE_ID != null }   // CHANGE_ID only exists for PR builds
            }
            steps {
                echo "Building PR #${env.CHANGE_ID}: ${env.CHANGE_TITLE}"
                echo "Source branch: ${env.CHANGE_BRANCH}"
                echo "Target branch: ${env.CHANGE_TARGET}"
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
                expression { fileExists('ruby_scripts/tests') }
            }
            steps {
                dir('ruby_scripts') {
                    sh 'ruby tests/run_all.rb'
                }
            }
        }

        stage('Auto-Merge') {
            when {
                expression { env.CHANGE_ID != null }
            }
            steps {
                script {
                    echo "All required checks passed — merging PR #${env.CHANGE_ID} into ${env.CHANGE_TARGET}..."

                    def mergeResponse = sh(
                        script: """
                            curl -s -X PUT \
                            -H "Authorization: token ${GITHUB_TOKEN}" \
                            -H "Content-Type: application/json" \
                            -d '{"commit_title": "Auto-merge PR #${env.CHANGE_ID}", "merge_method": "squash"}' \
                            https://api.github.com/repos/${REPO_OWNER}/${REPO_NAME}/pulls/${env.CHANGE_ID}/merge
                        """,
                        returnStdout: true
                    )

                    echo "Merge response: ${mergeResponse}"

                    if (mergeResponse.contains('"merged":true')) {
                        echo "PR #${env.CHANGE_ID} successfully merged to ${env.CHANGE_TARGET}."
                    } else {
                        error "Merge failed. Response: ${mergeResponse}"
                    }
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