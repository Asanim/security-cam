# greengrass-components

## Use
This repo includes a set of convenience scripts

To create a new deployment, navigate to the component's directory and run:
./issue-manual-deployment.sh 0.0.0
Please review the 


## Useful Commands

* discover the accound an aws profile is associated with
aws sts get-caller-identity
* Change the aws profile used to a non-default user
export AWS_PROFILE=iamadmin-general