#!/bin/sh

if [ ! -x /usr/bin/python3 ]; then
  echo "/usr/bin/python3 not found, replacing..."
  ln -sfn $(which python3) /usr/bin/python3
fi
