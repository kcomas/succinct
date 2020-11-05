#!/bin/bash

echo `echo -n '[' && ./sc -t $1 | sed 's/,$//' && echo -n ']'` | python -m json.tool
