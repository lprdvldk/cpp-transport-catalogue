#!/usr/bin/env bash
# Builds the project and runs it against the fixtures in this folder.
set -e
cd "$(dirname "$0")/.."

g++ -std=c++20 -Wall -Wextra -O2 *.cpp -o transport_catalogue

echo "== example1 (from the assignment spec) =="
./transport_catalogue < tests/example1_input.json | python3 -m json.tool

echo
echo "== example2 (round trip / not-found / empty-buses) =="
./transport_catalogue < tests/example2_input.json | python3 -m json.tool
