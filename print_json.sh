#!/bin/bash

set -e

echo `echo -n '[' && ./sc $1 $2 | sed 's/,$//' && echo -n ']'` | python -m json.tool
