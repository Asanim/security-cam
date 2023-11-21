#!/bin/bash

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

envsubst < "gdk-config.json.template" > "gdk-config.json"

gdk component build

sudo /greengrass/v2/bin/greengrass-cli deployment create \
  --recipeDir ./greengrass-build/recipes \
  --artifactDir ./greengrass-build/artifacts \
  --merge "com.example.BatteryAwareHelloWorld=$COMPONENT_VERSION"

