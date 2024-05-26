#!/bin/env bash

assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s -Wa,--noexecstack
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

# exit status
assert 0 '0;'
assert 42 '42;'

# arithmetic operations
assert 21 '5+20-4;'
assert 41 ' 12 + 34 - 5;'

# operator precedence
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'

# unary operators
assert 7 '-3+10;'
assert 5 '-3*5+20;'

# relational operators
assert 1 '1==1;'
assert 0 '1!=1;'
assert 1 '1!=0;'
assert 0 '1==0;'
assert 1 '0<1;'
assert 0 '0>1;'
assert 1 '1>0;'
assert 0 '1<0;'
assert 1 '0<=1;'
assert 0 '0>=1;'
assert 1 '1>=0;'
assert 0 '1<=0;'
assert 1 '1==1>=0;'

# local variables
assert 1 'a=1;'
assert 1 'a=1==1;'
assert 0 'a=1==0;'
assert 2 'a=b=c=2;'

echo OK
