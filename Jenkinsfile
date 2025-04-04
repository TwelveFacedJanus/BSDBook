pipeline {
    agent any

    environment {
        // Define any environment variables here
        DOCKER_IMAGE = 'Dockerfile' // Replace with your Docker image name
    }

    stages {
        stage('Checkout') {
            steps {
                // Checkout the code from the GitHub repository
                git url: 'https://github.com/TwelveFacedJanus/BSDBook.git', branch: 'release'
            }
        }

        stage('Build Docker Image') {
            steps {
                script {
                    // Build the Docker image
                    docker.build("${DOCKER_IMAGE}")
                }
            }
        }

        stage('Build Project') {
            steps {
                script {
                    // Run the build inside the Docker container
                    docker.image("${DOCKER_IMAGE}").inside {
                        sh 'make'
                    }
                }
            }
        }

        stage('Test Project') {
            steps {
                script {
                    // Run the tests inside the Docker container
                    docker.image("${DOCKER_IMAGE}").inside {
                        sh 'make test'
                    }
                }
            }
        }

        stage('Archive') {
            steps {
                // Archive the build artifacts
                archiveArtifacts artifacts: '**/build/*', allowEmptyArchive: true
            }
        }
    }

    post {
        always {
            // Clean up the workspace
            cleanWs()
        }
    }
}
