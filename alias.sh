#!/bin/sh

# Convenience script to source when working

alias make="make -j8"
alias test="make run_tests" # Automatically -j8 due to previous alias