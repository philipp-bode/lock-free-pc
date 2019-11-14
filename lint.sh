#!/bin/bash

cpplint \
    --verbose=0 \
    --extensions=hpp,cpp \
    --counting=detailed \
    --filter=-legal/copyright,-whitespace/newline,-runtime/references,-build/c++11 \
    --linelength=120 \
    src/*pp
