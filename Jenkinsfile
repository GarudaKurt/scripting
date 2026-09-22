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

        stage('Auto-Merge') {
            when {
                allOf {
                    changeRequest()
                    expression {
                        env.CHANGE_TARGET == 'main'
                    }
                }
            }

            steps {
                script {
                    echo "===================================================="
                    echo "                  Auto-Merge PR                     "
                    echo "===================================================="
                    echo "PR #${env.CHANGE_ID}"
                    echo "Target: ${env.CHANGE_TARGET}"
                    echo "All required checks passed."
                    echo "Attempting to merge..."
                    echo "===================================================="

                    def mergeResponse = sh(
                        script: """
                            curl -s \
                                -X PUT \
                                -H "Authorization: Bearer ${GITHUB_TOKEN}" \
                                -H "Accept: application/vnd.github+json" \
                                -H "X-GitHub-Api-Version: 2022-11-28" \
                                -H "Content-Type: application/json" \
                                -d '{"commit_title":"Auto-merge PR #${env.CHANGE_ID}","merge_method":"squash"}' \
                                "https://api.github.com/repos/${REPO_OWNER}/${REPO_NAME}/pulls/${env.CHANGE_ID}/merge"
                        """,
                        returnStdout: true
                    ).trim()

                    echo "GitHub merge response:"
                    echo mergeResponse

                    def mergeResult = readJSON text: mergeResponse

                    if (mergeResult.merged == true) {
                        echo "===================================================="
                        echo "PR #${env.CHANGE_ID} successfully merged."
                        echo "Target branch: ${env.CHANGE_TARGET}"
                        echo "===================================================="
                    } else {
                        def message = mergeResult.message ?: 'Unknown merge error'
                        error "PR #${env.CHANGE_ID} was not merged: ${message}"
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
