#!/usr/bin/env bash

./tests/integration/echo_server.py &
./tests/integration/echo_test.exs
