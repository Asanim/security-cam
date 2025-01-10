from ultralytics import YOLO

# Load a model
model = YOLO("yolo11n_saved_model/yolo11n_full_integer_quant_edgetpu.tflite")  # Load an official model or custom model

# Run Prediction
model.predict("es6dshf.png", task="detect")  # Replace 'detect' with the correct task
