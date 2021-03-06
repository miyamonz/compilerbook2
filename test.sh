#!/bin/bash

assert() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    cc -g -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected, but got $actual"
        exit 1
    fi
}

assert 0 '{ return 0; }'
assert 42 '{ return 42; }'
assert 21 '{ return 5+20-4; }'
assert 0 '{ return 10-1-2-3-4; }'
assert 21 '{ return 5 +      20          -4 ; }'
assert 47 '{ return 5+6*7; }'
assert 23 '{ return 5*6-7; }'
assert 3 '{ return 6/2; }'
assert 2 '{ return 5 - 6/2; }'

assert 15 '{ return 5*(9-6); }'
assert 4 '{ return (3+5)/2; }'

assert 2 '{ return -3+5; }'
assert 3 '{ return -(-6)/2; }'
assert 6 '{ return -3*-2; }'
assert 15 '{ return 3*+5; }'

assert 0 '{ return 0==1; }'
assert 1 '{ return 42==42; }'
assert 1 '{ return 0!=1; }'
assert 0 '{ return 42!=42; }'

assert 1 '{ return 0<1; }'
assert 0 '{ return 1<1; }'
assert 0 '{ return 2<1; }'
assert 1 '{ return 0<=1; }'
assert 1 '{ return 1<=1; }'
assert 0 '{ return 2<=1; }'

assert 1 '{ return 1>0; }'
assert 0 '{ return 1>1; }'
assert 0 '{ return 1>2; }'
assert 1 '{ return 1>=0; }'
assert 1 '{ return 1>=1; }'
assert 0 '{ return 1>=2; }'

assert 3 '{ 1; 2; 3; }'
assert 1 '{ return 1; 2; 3; }'
assert 2 '{ 1;return 2; 3; }'
assert 3 '{ 1;2;return 3; }'

assert 3 '{ a=3; return a; }'
assert 8 '{ a=3; z=5; return a+z; }'
assert 6 '{ a=b=3; return a+b; }'

assert 3 '{ foo=3; return foo; }'
assert 8 '{ foo123=3; bar=5; return foo123+bar; }'

assert 3 '{ if(0) return 2; return 3; }'
assert 3 '{ if(1-1) return 2; return 3; }'
assert 2 '{ if(1) return 2; return 3; }'
assert 2 '{ if(2-1) return 2; return 3; }'

assert 5 '{ for(;;) return 5; }'
assert 20 '{ a=0; for(i=0; i<2 ;i=i+1) a= a+10; return a; }'
assert 10 '{ b=0; for(i=0; i<5 ;i=i+1) b= b+i; return b; }'

assert 5 '{ i=0; while(i<5) i = i+1; return i; }'

assert 3 '{ 1; {2;} return 3; }'
assert 10 '{ i=0; while(i<10){i=i+1;} return i; }'
assert 55 '{ i=0; j=0; while(i<=10){j = j+i; i=i+1;} return j; }'

echo OK
