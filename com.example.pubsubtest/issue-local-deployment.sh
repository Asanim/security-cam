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
export THING_GROUP=Robot
export AWS_ACCOUNT_ID=$(aws sts get-caller-identity | jq -r '.Account')
export BUCKET_NAME="greengrass-component-artifacts"
export COMPONENT_NAME="com.example.pubsubtest"
export COMPONENT_AUTHOR="Synapse Automota"

envsubst < "./aws-deployment-templates/recipe.json.template" > "recipe.json"

envsubst < "./aws-deployment-templates/gdk-config.json.template" > "gdk-config.json"

gdk component build

envsubst < "./aws-deployment-templates/recipe.json.template" > "./greengrass-build/recipes/recipe.json"

sudo /greengrass/v2/bin/greengrass-cli deployment create \
  --recipeDir ./greengrass-build/recipes \
  --artifactDir ./greengrass-build/artifacts \
  --merge "com.example.pubsubtest=$COMPONENT_VERSION"

