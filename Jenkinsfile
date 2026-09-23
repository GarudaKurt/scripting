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
        REPO_OWNER = 'GarudaKurt'
        REPO_NAME  = 'scripting'
    }

    stages {

        stage('Hello World') {
            steps {
                echo "Hello World!"
            }
        }

        stage('Show PR Info') {
            steps {
                echo "===================================================="
                echo "                 Pull Request Info"
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
            when {
                expression {
                    return env.CHANGE_ID != null
                }
            }

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

                        echo "===================================================="
                        echo "             GitHub Authentication Test"
                        echo "===================================================="

                        curl -sS \
                            -H "Authorization: Bearer ${GITHUB_TOKEN}" \
                            -H "Accept: application/vnd.github+json" \
                            -H "X-GitHub-Api-Version: 2022-11-28" \
                            "https://api.github.com/user" \
                            | jq '{login, id}'

                        echo "GitHub authentication successful."
                    '''
                }
            }
        }

        stage('Check PR') {
            when {
                expression {
                    return env.CHANGE_ID != null
                }
            }

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

                        echo "===================================================="
                        echo "                 Checking Pull Request"
                        echo "===================================================="

                        curl -sS \
                            -H "Authorization: Bearer ${GITHUB_TOKEN}" \
                            -H "Accept: application/vnd.github+json" \
                            -H "X-GitHub-Api-Version: 2022-11-28" \
                            "https://api.github.com/repos/${REPO_OWNER}/${REPO_NAME}/pulls/${CHANGE_ID}" \
                            > github_pr.json

                        echo "PR information:"

                        jq '{
                            number,
                            title,
                            state,
                            draft,
                            mergeable,
                            mergeable_state,
                            head: .head.ref,
                            base: .base.ref
                        }' github_pr.json

                        STATE=$(jq -r '.state' github_pr.json)
                        DRAFT=$(jq -r '.draft' github_pr.json)
                        MERGEABLE=$(jq -r '.mergeable' github_pr.json)

                        if [ "$STATE" != "open" ]; then
                            echo "ERROR: PR is not open."
                            exit 1
                        fi

                        if [ "$DRAFT" = "true" ]; then
                            echo "ERROR: PR is still a draft."
                            exit 1
                        fi

                        if [ "$MERGEABLE" = "false" ]; then
                            echo "ERROR: PR is not mergeable."
                            exit 1
                        fi

                        echo "PR is open and mergeable."
                    '''
                }
            }
        }

        stage('Auto-Merge') {
            when {
                expression {
                    return env.CHANGE_ID != null
                }
            }

            steps {
                script {
                    echo "===================================================="
                    echo "                  Auto-Merge PR"
                    echo "===================================================="
                    echo "PR #${env.CHANGE_ID}"
                    echo "Target: ${env.CHANGE_TARGET}"
                    echo "All required checks passed."
                    echo "Attempting to merge..."
                    echo "===================================================="

                    withCredentials([
                        usernamePassword(
                            credentialsId: 'automation',
                            usernameVariable: 'GITHUB_USER',
                            passwordVariable: 'GITHUB_TOKEN'
                        )
                    ]) {
                        sh '''
                            set -e

                            echo "Sending merge request to GitHub..."

                            HTTP_CODE=$(curl -sS \
                                -o merge_response.json \
                                -w "%{http_code}" \
                                -X PUT \
                                -H "Authorization: Bearer ${GITHUB_TOKEN}" \
                                -H "Accept: application/vnd.github+json" \
                                -H "X-GitHub-Api-Version: 2022-11-28" \
                                -H "Content-Type: application/json" \
                                --data '{"merge_method":"squash"}' \
                                "https://api.github.com/repos/${REPO_OWNER}/${REPO_NAME}/pulls/${CHANGE_ID}/merge")

                            echo "GitHub HTTP status: ${HTTP_CODE}"
                            echo "GitHub merge response:"
                            jq . merge_response.json

                            if [ "${HTTP_CODE}" != "200" ]; then
                                echo "ERROR: GitHub merge request failed."
                                exit 1
                            fi

                            MERGED=$(jq -r '.merged' merge_response.json)

                            if [ "${MERGED}" = "true" ]; then
                                echo "PR #${CHANGE_ID} successfully merged."
                            else
                                echo "ERROR: GitHub did not merge the PR."
                                exit 1
                            fi
                        '''
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
            script {
                if (env.NODE_NAME) {
                    cleanWs()
                }
            }
        }
    }
}
