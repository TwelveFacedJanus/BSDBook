pipeline {
	agent any
	stages {
		stage('Build') {
			steps {
				sh 'make'
			}
		}
		stage('Packing') {
			steps {
				sh 'make package'
			}
		}
	}
}
