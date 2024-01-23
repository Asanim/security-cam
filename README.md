# AWS Greengrass Component Repository

Welcome to the AWS Greengrass Component Repository! This repository houses a collection of Greengrass components and convenience scripts designed to streamline the deployment and management of AWS Greengrass applications. Whether you are exploring AWS IoT capabilities, experimenting with device shadows, testing publish-subscribe functionalities, or implementing a security camera solution, this repository provides a diverse set of components to meet your needs.

## Overview

The repository is organized into several component directories, each serving a specific purpose:

- **com.example.pubsubtest:** Ideal for testing and demonstrating the publish-subscribe capabilities of AWS Greengrass.

- **com.example.StreamManagerS3Upload:** Focuses on uploading data to Amazon S3 using AWS IoT Greengrass Stream Manager, facilitating seamless data transfer and storage in the AWS Cloud.

- **com.example.BatteryAwareHelloWorld:** Demonstrates the creation of a battery-aware Greengrass Lambda function, addressing power-efficient application development.

- **com.example.shadowtest:** Designed for testing and interacting with AWS IoT Thing Shadows, enabling experimentation with device state synchronization and management.

- **com.synapseautomota.SecurityCamera:** Tailored for security camera applications, observing image streams, detecting people, and uploading video recordings and snapshots to the AWS Cloud.

- **scripts:** Houses convenience scripts for managing AWS Greengrass components and AWS profiles, simplifying common deployment and AWS profile management tasks.

# Component Directories Overview

## com.example.pubsubtest

The `com.example.pubsubtest` directory contains a Greengrass component designed for testing and demonstrating the publish-subscribe capabilities of AWS Greengrass. It is particularly useful for experimenting with message passing between devices within a Greengrass group.

### Usage

To deploy this component, navigate to the directory and run the manual deployment script:

```bash
cd com.example.pubsubtest
./issue-manual-deployment.sh 0.0.0
```

Replace `0.0.0` with the desired version number.

## com.example.StreamManagerS3Upload

The `com.example.StreamManagerS3Upload` directory houses a Greengrass component focused on uploading data to Amazon S3 using AWS IoT Greengrass Stream Manager. This component is beneficial for scenarios requiring seamless data transfer and storage in the AWS Cloud.

### Usage

Deploy the component by navigating to its directory and running the manual deployment script:

```bash
cd com.example.StreamManagerS3Upload
./issue-manual-deployment.sh 0.0.0
```

Replace `0.0.0` with the version number you intend to use.

## com.example.BatteryAwareHelloWorld

In the `com.example.BatteryAwareHelloWorld` directory, you'll find a Greengrass component that demonstrates how to create a battery-aware Greengrass Lambda function. This component is helpful for understanding and implementing power-efficient applications that consider the device's battery status.

### Usage

To deploy this battery-aware component, navigate to its directory and run the manual deployment script:

```bash
cd com.example.BatteryAwareHelloWorld
./issue-manual-deployment.sh 0.0.0
```

Replace `0.0.0` with your chosen version number.

## com.example.shadowtest

The `com.example.shadowtest` directory contains a Greengrass component for testing and interacting with AWS IoT Thing Shadows. This component is ideal for experimenting with device state synchronization and management using AWS IoT Thing Shadows.

### Usage

Deploy the `com.example.shadowtest` component by navigating to its directory and executing the manual deployment script:

```bash
cd com.example.shadowtest
./issue-manual-deployment.sh 0.0.0
```

Specify the desired version number instead of `0.0.0`.

## com.synapseautomota.SecurityCamera

The `com.synapseautomota.SecurityCamera` directory houses a Greengrass component designed for security camera applications. This component is configured to observe an incoming image stream, detect people, and upload video recordings and snapshots to the AWS Cloud for further analysis.

### Usage

Deploy the security camera component by navigating to its directory and running the manual deployment script:

```bash
cd com.synapseautomota.SecurityCamera
./issue-manual-deployment.sh 0.0.0
```

Replace `0.0.0` with the version number you wish to use.

## scripts

The `scripts` directory contains convenience scripts for managing AWS Greengrass components and AWS profiles. These scripts simplify common tasks related to deployment and AWS profile management.

### Usage

Explore and utilize the scripts in the `scripts` directory based on your specific Greengrass component deployment and management needs. Refer to the script documentation for detailed instructions.

Feel free to customize and extend these components according to your project requirements.


## Usage

### Manual Deployment

To create a new deployment, follow these steps:

1. Navigate to the component's directory.

   ```bash
   cd your-component-directory
   ```

2. Run the manual deployment script, specifying the version number.

   ```bash
   ./issue-manual-deployment.sh 0.0.0
   ```

   Replace `0.0.0` with the desired version number. Please review the component-specific documentation for any additional deployment instructions.

## Useful Commands

Here are some useful commands to enhance your Greengrass component management experience:

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

This command will prompt you to set up a new AWS profile, allowing you to easily switch between different AWS configurations.

Feel free to explore and customize these commands to suit your specific Greengrass component deployment and management needs.



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

## License

This security camera application is released under the [MIT License](LICENSE). Feel free to customize and extend it according to your needs.

## Support and Contributions

For questions, issues, or contributions, please create a GitHub issue or reach out to our community at [community.example.com](https://community.example.com).

Happy monitoring! 📷🔒
