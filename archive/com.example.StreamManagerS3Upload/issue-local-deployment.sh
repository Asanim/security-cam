#!/bin/bash -e

# Usage:
# Run this script from the root of the repo with the desired COMPONENT_VERSION parameter.
# Example: ./scripts/deploy.sh 0.2.0.11

# Validate the COMPONENT_VERSION parameter
if [ $# -ne 1 ]; then
    echo "Usage: $0 COMPONENT_VERSION"
    exit 1
fi

# Define environment variables
export COMPONENT_VERSION="$1"
export AWS_REGION=ap-southeast-2
export THING_GROUP=TestStream
export AWS_ACCOUNT_ID=$(aws sts get-caller-identity | jq -r '.Account')
export BUCKET_NAME="greengrass-component-artifacts"
export COMPONENT_NAME="com.example.StreamManagerS3Upload"
export COMPONENT_AUTHOR="Asanim"

# Create 'recipe.json' using environment variables
# envsubst < "./aws-deployment-templates/recipe.json.template" > "recipe.json"
envsubst < "./aws-deployment-templates/recipe.json.template" > "./greengrass-build/recipes/recipe.json"

# Create 'gdk-config.json' using environment variables
envsubst < "./aws-deployment-templates/gdk-config.json.template" > "gdk-config.json"

# Build and publish the component
gdk component build
envsubst < "./aws-deployment-templates/recipe.json.template" > "./greengrass-build/recipes/recipe.json"

# Run the deployment create command
deployment_output=$(sudo /greengrass/v2/bin/greengrass-cli deployment create \
  --recipeDir ./greengrass-build/recipes \
  --artifactDir ./greengrass-build/artifacts \
  --merge "$COMPONENT_NAME=$COMPONENT_VERSION")

# Check if the deployment submission was successful
if [[ $deployment_output =~ Deployment\ Id:\ ([0-9a-fA-F-]+) ]]; then
    deployment_id="${BASH_REMATCH[1]}"
    echo "Deployment submitted! Deployment Id: $deployment_id"

    # Run the deployment status command with the obtained deployment ID
    while true; do
        sudo /greengrass/v2/bin/greengrass-cli deployment status -i "$deployment_id"
        sleep 1
    done
else
    echo "Error submitting deployment."
fi


