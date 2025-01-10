#!/bin/bash

wget https://github.com/ultralytics/assets/releases/download/v8.3.0/yolo11n.pt

conda create -n ultralytics python=3.11
conda install -c conda-forge ultralytics
