#!/bin/bash
python3 setup.py bdist_wheel
python3 patcher.py dist/ 
pip3 uninstall -qy pyterrier_pisa
pip3 install dist/*.whl
