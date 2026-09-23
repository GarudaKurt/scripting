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

                        echo "Testing GitHub authentication..."

                        curl -sS \
                            -H "Authorization: Bearer ${GITHUB_TOKEN}" \
                            -H "Accept: application/vnd.github+json" \
                            -H "X-GitHub-Api-Version: 2022-11-28" \
                            "https://api.github.com/user" |
                            jq '{login, id}'
                    '''
                }
            }
        }

        stage('Check PR Status') {
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

                        echo "Checking PR #${CHANGE_ID}..."

                        RESPONSE=$(curl -sS \
                            -w '\\n%{http_code}' \
                            -H "Authorization: Bearer ${GITHUB_TOKEN}" \
                            -H "Accept: application/vnd.github+json" \
                            -H "X-GitHub-Api-Version: 2022-11-28" \
                            "https://api.github.com/repos/${REPO_OWNER}/${REPO_NAME}/pulls/${CHANGE_ID}")

                        HTTP_CODE=$(echo "$RESPONSE" | tail -n 1)
                        BODY=$(echo "$RESPONSE" | sed '$d')

                        echo "GitHub HTTP status: ${HTTP_CODE}"

                        if [ "$HTTP_CODE" != "200" ]; then
                            echo "Failed to retrieve PR information."
                            echo "$BODY"
                            exit 1
                        fi

                        echo "$BODY" | jq '{
                            number,
                            title,
                            state,
                            draft,
                            mergeable,
                            mergeable_state,
                            head: .head.ref,
                            base: .base.ref
                        }'

                        STATE=$(echo "$BODY" | jq -r '.state')
                        DRAFT=$(echo "$BODY" | jq -r '.draft')
                        MERGEABLE=$(echo "$BODY" | jq -r '.mergeable')

                        if [ "$STATE" != "open" ]; then
                            echo "PR #${CHANGE_ID} is not open."
                            exit 1
                        fi

                        if [ "$DRAFT" = "true" ]; then
                            echo "PR #${CHANGE_ID} is still a draft."
                            exit 1
                        fi

                        if [ "$MERGEABLE" = "false" ]; then
                            echo "PR #${CHANGE_ID} is not mergeable."
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
                    echo "                  Auto-Merge PR                     "
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

                            RESPONSE=$(curl -sS \
                                -w '\\n%{http_code}' \
                                -X PUT \
                                -H "Authorization: Bearer ${GITHUB_TOKEN}" \
                                -H "Accept: application/vnd.github+json" \
                                -H "X-GitHub-Api-Version: 2022-11-28" \
                                -H "Content-Type: application/json" \
                                -d "{\"commit_title\":\"Auto-merge PR #${CHANGE_ID}\",\"merge_method\":\"squash\"}" \
                                "https://api.github.com/repos/${REPO_OWNER}/${REPO_NAME}/pulls/${CHANGE_ID}/merge")

                            HTTP_CODE=$(echo "$RESPONSE" | tail -n 1)
                            BODY=$(echo "$RESPONSE" | sed '$d')

                            echo "GitHub HTTP status: ${HTTP_CODE}"
                            echo "GitHub merge response:"
                            echo "$BODY" | jq .

                            if [ "$HTTP_CODE" != "200" ]; then
                                echo "GitHub merge request failed."
                                exit 1
                            fi

                            MERGED=$(echo "$BODY" | jq -r '.merged')

                            if [ "$MERGED" = "true" ]; then
                                echo "===================================================="
                                echo "PR #${CHANGE_ID} successfully merged."
                                echo "===================================================="
                            else
                                echo "GitHub did not merge the PR."
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
            cleanWs()
        }
    }
}