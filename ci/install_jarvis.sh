#!/bin/bash
git clone https://github.com/grc-iit/jarvis-cd
cd jarvis-cd
pip install -e . -r requirements.txt
# Initialize the Jarvis testing Hermes environment
jarvis init \
"${HOME}/jarvis-config" \
"${HOME}/jarvis-priv" \
"${HOME}/jarvis-shared"
cp ../ci/resource_graph.yaml ./config/resource_graph.yaml
jarvis env build hermes
cd ..

