#!/usr/bin/env python3

import json
import os
import sys
import time
import traceback

import database as SQLLib

from pathlib import Path

from awsiot.greengrasscoreipc.clientv2 import GreengrassCoreIPCClientV2
import awsiot.greengrasscoreipc
import awsiot.greengrasscoreipc.model as model

HELLO_WORLD_PRINT_INTERVAL = 15  # Seconds
DEFER_COMPONENT_UPDATE_INTERVAL = 30 * 1000  # Milliseconds

import time
import sys
import threading
import random  # Import the random module

# Import any necessary libraries for database or AWS IoT
gc_iPartitionSize = 65535

class C_DetectionEventUpload:
    def __init__(self, sPartitionKey, sDeviceName, idMachine, iNumberOfCameras):
        # Initialize your variables here
        self.sPartitionKey = sPartitionKey
        self.sDeviceName = sDeviceName
        self.iNumberOfCameras = iNumberOfCameras
        tStartupTime = time.time()

        self.sTopicName = "DetectionEvent/dev"

        self.stDetectionEvent = {
            "idCompanyPartition": sPartitionKey,
            "iTimestamp": int(round(tStartupTime * 1000)),
            "idMachine": idMachine,
            "stDetections": [],
            "stPosition": [{
                "fLatitude": random.uniform(-13.2744, -31.2532),
                "fLongitude": random.uniform(153.0260, 133.7751),
            }],
            "iDuration": random.randint(1, 60),
            "tUpdatedAt" :    SQLLib.convertTimeToAWSDateTime(tStartupTime),
            "tCreatedAt" :    SQLLib.convertTimeToAWSDateTime(tStartupTime),
        }
        self.ipc_client = awsiot.greengrasscoreipc.connect()


    def logicAlarmTriggerCallback(self):
        # Generate random data for demonstration purposes
        # Replace this section with your actual logic
        xAlarmInput = bool(random.getrandbits(1))
        ts = time.time()
        self.tDetectionStart = ts
        self.stDetectionEvent["iTimestamp"] = int(round(ts * 1000))
        self.stDetectionEvent["tUpdatedAt"] = SQLLib.convertTimeToAWSDateTime(ts)
        self.stDetectionEvent["tCreatedAt"] = SQLLib.convertTimeToAWSDateTime(ts)
        self.stDetectionEvent["stDetections"] = []  # Reset detections
        for i in range(random.randint(1, 4)):
                # Generate random data for bounding boxes
                random_detection = {
                    "iCameraZoneID": random.randint(1, 4),
                    "iConfidence": random.randint(0, 100),
                    "stBoundingBox": [{
                        "iXMin": random.randint(0, 1000),
                        "iYMin": random.randint(0, 1000),
                        "iXMax": random.randint(0, 1000),
                        "iYMax": random.randint(0, 1000)
                    }],
                    "sType": "RandomType"
                }
                self.stDetectionEvent["stDetections"].append(random_detection)

        self.xInAlarm = True
        self.UploadDetectionEvent()

    def UploadDetectionEvent(self):     
        stTelemetryMessage = self.stDetectionEvent

        oSQLService = SQLLib.SQLService("detectionEvents", gc_iPartitionSize)
        oSQLServiceTimeout = SQLLib.SQLService("detectionEventsTimeout", gc_iPartitionSize)
        
        print(stTelemetryMessage)
        
        # save to db (for webapp)
        oSQLService.insertDetectionEvent(stTelemetryMessage)            

        # upload to cloud
        op = self.ipc_client.new_publish_to_iot_core()
        
        op.activate(model.PublishToIoTCoreRequest(
            topic_name=self.sPartitionKey+"/"+self.sDeviceName+"/"+self.sTopicName,
            qos=model.QOS.AT_LEAST_ONCE,
            payload=json.dumps(stTelemetryMessage).encode(),
        ))
        
        # check if data has been saved or not.. if not save to offline db
        try:
            result = op.get_response().result(timeout=5.0)
            print("Upload result")
            print(result)
            
        except Exception as e:
            oSQLServiceTimeout.insertDetectionEvent(stTelemetryMessage)            

class BatteryAwareHelloWorldPrinter():
    def __init__(self, ipc_client: GreengrassCoreIPCClientV2, battery_file_path: Path, battery_threshold: float):
        self.battery_file_path = battery_file_path
        self.battery_threshold = battery_threshold
        self.ipc_client = ipc_client
        self.subscription_operation = None

    def on_component_update_event(self, event):
        try:
            if event.pre_update_event is not None:
                if self.is_battery_below_threshold():
                    self.defer_update(event.pre_update_event.deployment_id)
                    print('Deferred update for deployment %s' %
                          event.pre_update_event.deployment_id)
                else:
                    self.acknowledge_update(
                        event.pre_update_event.deployment_id)
                    print('Acknowledged update for deployment %s' %
                          event.pre_update_event.deployment_id)
            elif event.post_update_event is not None:
                print('Applied update for deployment')
        except:
            traceback.print_exc()

    def subscribe_to_component_updates(self):
        if self.subscription_operation == None:
            # SubscribeToComponentUpdates returns a tuple with the response and the operation.
            _, self.subscription_operation = self.ipc_client.subscribe_to_component_updates(
                on_stream_event=self.on_component_update_event)

    def close_subscription(self):
        if self.subscription_operation is not None:
            self.subscription_operation.close()
            self.subscription_operation = None

    def defer_update(self, deployment_id):
        self.ipc_client.defer_component_update(
            deployment_id=deployment_id, recheck_after_ms=DEFER_COMPONENT_UPDATE_INTERVAL)

    def acknowledge_update(self, deployment_id):
        # Specify recheck_after_ms=0 to acknowledge a component update.
        self.ipc_client.defer_component_update(
            deployment_id=deployment_id, recheck_after_ms=0)

    def is_battery_below_threshold(self):
        return self.get_battery_level() < self.battery_threshold

    def get_battery_level(self):
        # Read the battery level from the virtual battery level file.
        with self.battery_file_path.open('r') as f:
            data = json.load(f)
            return float(data['battery_level'])

    def print_message(self):
        message = 'Hello, World!'
        if self.is_battery_below_threshold():
            message += ' Battery level (%d) is below threshold (%d), so the component will defer updates' % (
                self.get_battery_level(), self.battery_threshold)
        else:
            message += ' Battery level (%d) is above threshold (%d), so the component will acknowledge updates' % (
                self.get_battery_level(), self.battery_threshold)
        print(message)


def main():
    # Read the battery threshold and virtual battery file path from command-line args.
    args = sys.argv[1:]
    battery_threshold = float(args[0])
    battery_file_path = Path(args[1])
    print('Reading battery level from %s and deferring updates when below %d' % (
        str(battery_file_path), battery_threshold))


    sThingName = os.getenv("AWS_IOT_THING_NAME")
    sPartitionKey = ""
    idMachine = 0
    iNumberOfCameras = 0

    # #Read eventCloudPublisher.ini file
    # oEventCloudPublisherConfig = ConfigParser()
    # oEventCloudPublisherConfig.read("/home/visionai/eventCloudPublisher.ini")

    # #Get the password
    # stCloudPublisherConfig = oEventCloudPublisherConfig["CLOUDPUBLISHER"]

    # sPartitionKey = stCloudPublisherConfig["sCompanyPartitionKey"]
    # idMachine = stCloudPublisherConfig["idmachine"]
    # iNumberOfCameras =  int(stCloudPublisherConfig["inumberofcameras"])
    # global gc_iPartitionSize 
    # gc_iPartitionSize =  int(stCloudPublisherConfig["iPartitionSize"])
    sPartitionKey = "PRMEngineering"
    idMachine = "zephyrus"
    iNumberOfCameras = 4
    gc_iPartitionSize = 100


    detection_event = C_DetectionEventUpload(sPartitionKey, sThingName, idMachine, iNumberOfCameras)

    try:
        # Create an IPC client and a Hello World printer that defers component updates.
        ipc_client = GreengrassCoreIPCClientV2()
        hello_world_printer = BatteryAwareHelloWorldPrinter(
            ipc_client, battery_file_path, battery_threshold)
        hello_world_printer.subscribe_to_component_updates()
        try:
            # Keep the main thread alive, or the process will exit.
            while True:
                hello_world_printer.print_message()
                detection_event.logicAlarmTriggerCallback()

                time.sleep(HELLO_WORLD_PRINT_INTERVAL)
        except InterruptedError:
            print('Subscription interrupted')
        hello_world_printer.close_subscription()
    except Exception:
        print('Exception occurred', file=sys.stderr)
        traceback.print_exc()
        exit(1)


if __name__ == '__main__':
    main()
