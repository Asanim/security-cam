# Security Camera Reference Application
Security Camera AI enabled using Amazon Web Services (AWS). This is an example project for demonstrating and exploring AWS IoT functionality

This repos has been cloned from my example repo for AWS. The original repo can be found at the following link:
https://github.com/Asanim/greengrass-components

A Greengrass component designed for security camera applications. This component is configured to observe an incoming image stream, detect people, and upload video recordings and snapshots to the AWS Cloud for further analysis.

## Requirements 

this project is designed to showcase a basic implementation of a security camera for vehicles. 

shall run off the car's battery whiel it is turned off

https://github.com/rockchip-linux/rknn-toolkit.git


AWS
- Remote update 
- remote upload of events 
   - publish MQTT message of raw 
   - 
- support rockchip and RK3399pro. Add further support for cam1126su
- 


cloud 
- monitor runtimes / operational times
- 

todo
- create document pdf generator
- 



- must integrate with the RK3399 Pro platform
* monitor the outside of the car 
* terraform

why 


## Useful Links
https://wiki.t-firefly.com/en/3399pro_npu/npu_rknn_api.html
https://wiki.t-firefly.com/en/Firefly-RK3399/index.html
https://github.com/airockchip/RK3399Pro_npu

### Usage

Deploy the security camera component by navigating to its directory and running the manual deployment script:

```bash
./scripts/issue-manual-deployment.sh 0.0.0
```

Replace `0.0.0` with the version number you wish to set.

## scripts

The `scripts` directory contains convenience scripts for managing AWS Greengrass components and AWS profiles. These scripts simplify common tasks related to deployment and AWS profile management.

### Usage

Explore and utilize the scripts in the `scripts` directory based on your specific Greengrass component deployment and management needs. Refer to the script documentation for detailed instructions.

## Useful Commands

### Discover AWS Profile Information

To discover the AWS account associated with an AWS profile, use the following command:

```bash
aws sts get-caller-identity
```

### Change AWS Profile

Switch the AWS profile to a non-default user using the following commands:

```bash
export AWS_PROFILE=iamadmin-general
```

Replace `iamadmin-general` with the desired profile name.

```bash
export AWS_PROFILE=admin-sandbox
```

Replace `admin-sandbox` with the desired profile name.

### Add a New AWS Profile

To add a new AWS profile, run the following command:

```bash
aws configure --profile
```

# ARM64 and ARM32 Security Camera Application

## Overview

This is a security camera application built in pure C++ for ARM64 and ARM32 architectures. The application is designed to run on the RK3399 Pro platform, leveraging the capabilities of the RV1126 camera module for efficient security monitoring. It detects people in the incoming image stream, records videos, captures snapshots, and uploads the data to the AWS Cloud for centralized storage and accessibility.

## Features

- **Person Detection:** Utilizes computer vision algorithms to detect the presence of people in the incoming image stream.

- **Video Recording:** Initiates video recording upon detecting a person, capturing the activities of the detected person.

- **Snapshot Capture:** Captures a snapshot of the detected person for quick reference and analysis.

- **Cloud Upload:** Securely uploads recorded videos and snapshots to the AWS Cloud.

## Hardware Requirements

- **RK3399 Pro:** The application is optimized for the RK3399 Pro platform. Ensure that the necessary hardware and drivers are properly configured.

- **RV1126 Camera Module:** Required for capturing the image stream. Please refer to the [RV1126 product page](https://en.t-firefly.com/product/eca3399proc) for details.

## Build Instructions

1. Clone this repository to your ARM64/ARM32 device.

   ```bash
   git clone https://github.com/yourusername/security-camera-arm.git
   ```

2. Navigate to the project directory.

   ```bash
   cd security-camera-arm
   ```

3. Build the application for ARM64.

   ```bash
   make ARCH=arm64
   ```

4. Build the application for ARM32.

   ```bash
   make ARCH=arm32
   ```

## Configuration

1. Open the `config.yml` file to configure AWS credentials, IoT Core endpoint, and other parameters.

2. Adjust detection sensitivity and recording settings in the configuration file based on your preferences and requirements.

## Usage

Run the compiled executable on your RK3399 Pro device.

For ARM64:

```bash
./security-camera-arm64
```

For ARM32:

```bash
./security-camera-arm32
```

The application will start monitoring the image stream, and detected people will trigger video recording and snapshot capture. The data will be uploaded to the specified AWS Cloud storage.