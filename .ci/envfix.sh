#!/bin/sh

FILE=/usr/bin/python3

while [ -h "$FILE" ]; do
  echo -n "Following symlink $FILE..."
  FILE=$(readlink "$FILE")
  echo " Found $FILE"
done

if [ "$FILE" != "/usr/bin/python3" ]; then
  echo "Replacing /usr/bin/python3"
  ln -sfn "$FILE" /usr/bin/python3
fi
