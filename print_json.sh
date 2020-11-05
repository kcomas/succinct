#!/bin/bash

echo `echo -n '[' && ./sc $1 | sed 's/,$//' && echo -n ']'` | python -m json.tool
