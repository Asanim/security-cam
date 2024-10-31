import os
import asyncio
import logging
import time
import sys

from stream_manager import (
    ExportDefinition,
    MessageStreamDefinition,
    ReadMessagesOptions,
    ResourceNotFoundException,
    S3ExportTaskDefinition,
    S3ExportTaskExecutorConfig,
    Status,
    StatusConfig,
    StatusLevel,
    StatusMessage,
    StrategyOnFull,
    StreamManagerClient,
    StreamManagerException,
)
from stream_manager.util import Util

class S3Uploader:
    def __init__(self, directory_path, stream_name, status_stream_name, bucket_name):
        self.directory_path = directory_path
        self.stream_name = stream_name
        self.status_stream_name = status_stream_name
        self.bucket_name = bucket_name
        self.client = StreamManagerClient()
        self.logger = logging.getLogger()

    def initialize_streams(self):
        try:
            # Try deleting the status stream (if it exists) so that we have a fresh start
            try:
                self.client.delete_message_stream(stream_name=self.status_stream_name)
            except ResourceNotFoundException:
                pass

            # Try deleting the stream (if it exists) so that we have a fresh start
            try:
                self.client.delete_message_stream(stream_name=self.stream_name)
            except ResourceNotFoundException:
                pass

            exports = ExportDefinition(
                s3_task_executor=[
                    S3ExportTaskExecutorConfig(
                        identifier="S3TaskExecutor" + self.stream_name,
                        status_config=StatusConfig(
                            status_level=StatusLevel.INFO,
                            status_stream_name=self.status_stream_name,
                        ),
                    )
                ]
            )

            # Create the Status Stream.
            self.client.create_message_stream(
                MessageStreamDefinition(name=self.status_stream_name, strategy_on_full=StrategyOnFull.OverwriteOldestData)
            )

            # Create the message stream with the S3 Export definition.
            self.client.create_message_stream(
                MessageStreamDefinition(
                    name=self.stream_name, strategy_on_full=StrategyOnFull.OverwriteOldestData, export_definition=exports
                )
            )
        except Exception as e:
            self.logger.exception(f"Exception during stream initialization: {e}")

    def upload_files(self):
        try:
            # Iterate through files in the specified directory
            for filename in os.listdir(self.directory_path):
                file_path = os.path.join(self.directory_path, filename)

                # Append a S3 Task definition and print the sequence number
                s3_export_task_definition = S3ExportTaskDefinition(
                    input_url=f"file:{file_path}", bucket=self.bucket_name, key=filename
                )
                sequence_number = self.client.append_message(
                    self.stream_name, Util.validate_and_serialize_to_json_bytes(s3_export_task_definition)
                )
                self.logger.info(
                    f"Successfully appended S3 Task Definition to stream with sequence number {sequence_number}"
                )

            # Monitor the status of file uploads
            self.monitor_status()

        except Exception as e:
            self.logger.exception(f"Exception during file upload: {e}")

    def monitor_status(self):
        stop_checking = False
        next_seq = 0
        while not stop_checking:
            try:
                messages_list = self.client.read_messages(
                    self.status_stream_name,
                    ReadMessagesOptions(
                        desired_start_sequence_number=next_seq, min_message_count=1, read_timeout_millis=1000
                    ),
                )
                for message in messages_list:
                    status_message = Util.deserialize_json_bytes_to_obj(message.payload, StatusMessage)

                    if status_message.status == Status.Success:
                        self.logger.info(f"Successfully uploaded file with sequence number {message.sequence_number} to S3.")
                    elif status_message.status == Status.InProgress:
                        self.logger.info("File upload is in Progress.")
                        next_seq = message.sequence_number + 1
                    elif status_message.status in [Status.Failure, Status.Canceled]:
                        self.logger.info(
                            f"Unable to upload file with sequence number {message.sequence_number} to S3. Message: {status_message.message}"
                        )

                if not stop_checking:
                    time.sleep(5)
            except StreamManagerException as e:
                self.logger.exception(f"Exception while running: {e}")
                time.sleep(5)
            except Exception as e:
                self.logger.exception(f"Unhandled exception during status monitoring: {e}")

    def cleanup(self):
        try:
            if self.client:
                self.client.close()
        except Exception as e:
            self.logger.exception(f"Exception during cleanup: {e}")

if __name__ == "__main__":
    args = sys.argv[1:]
    directory_path = args[0]

    logging.basicConfig(level=logging.INFO)
    stream_name = "SomeStream"
    status_stream_name = "SomeStatusStreamName"
    bucket_name = "video-bucket-517597205684"

    uploader = S3Uploader(directory_path, stream_name, status_stream_name, bucket_name)
    uploader.initialize_streams()
    uploader.upload_files()
    uploader.cleanup()
