#!/bin/bash

assert() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected, but got $actual"
        exit 1
    fi
}

assert 0 0 
assert 42 42
assert 21 '5+20-4'
assert 0 '10-1-2-3-4'
assert 21 '5 +      20          -4'
assert 47 '5+6*7'
assert 23 '5*6-7'
assert 3 '6/2'
assert 2 '5 - 6/2'