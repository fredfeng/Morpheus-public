#!/bin/bash

fname=$1

sed -n '/^R program:/,$p' $fname | sed -e '/^R program:/d' | sed '/morpheus$/q'

