#!/bin/sh

FILE=/usr/bin/python3

while [ -h "$FILE" ]; do
  echo -n "Following symlink $FILE..."
  FILE=$(readlink "$FILE")
  echo " Found $FILE"
done

echo "Replacing /usr/bin/python3"
#ln -sfn "$FILE" /usr/bin/python3
BIN=/home/travis/virtualenv/python3.5.*
BIN=$BIN/bin/python3

ln -sfn "$BIN" /usr/bin/python3
