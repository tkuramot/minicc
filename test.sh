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
assert 4 'foo=4;bar=4;'
assert 8 'foo=4;bar=foo+4;'
assert 12 'f=4;foo=4;bar=f+foo+4;'

# multiple statements
assert 4 '3+3; a=4;'
assert 5 'a=4; a=a+1;'

# return statement
assert 4 'return 4;'
assert 4 'a=4; return a;'
assert 5 'a=4; return a+1;'

# if statements
assert 4 'a=3; if (1) a=a+1;return a;'
assert 3 'a=3; if (0) a=a+1;return a;'
assert 4 'a=4; if (1) return 4; return 2;'
assert 2 'a=4; if (0) return 4; return 2;'
assert 2 'a=4; if (0) return 4; if (1) return 2;'

# if else statements
assert 2 'if (0) return 4; else return 2;'
assert 7 'a=3; if (a==9) return 9; else if (a==8) return 8; else return 7;'

echo OK
