#!/usr/bin/env bash
DIR=$(dirname "$0")
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$DIR LD_PRELOAD=$DIR/libcs2djit.so $DIR/cs2d_dedicated
