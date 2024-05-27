#!/bin/env bash

assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s mock.c -Wa,--noexecstack
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
assert 3 'a=3; if(1) return a;'
assert 4 'a=3; if (1) a=a+1;return a;'
assert 3 'a=3; if (0) a=a+1;return a;'
assert 4 'a=4; if (1) return 4; return 2;'
assert 2 'a=4; if (0) return 4; return 2;'
assert 2 'a=4; if (0) return 4; if (1) return 2;'
assert 1 'a=0; b=3; if (b==3) a=a+1; else if (b==3) a=a+1; return a;'

# if else statements
assert 2 'if (0) return 4; else return 2;'
assert 7 'a=3; if (a==9) return 9; else if (a==8) return 8; else return 7;'
assert 8 'a=3; if (a==9) return 9; else if (a==3) return 8; else return 7;'

# while statements
assert 5 'a=0; while (a<5) a=a+1;'

# for statements
assert 5 'b=0; for (a=0; a<5; a=a+1) b=b+1; return b;'
assert 10 'a=0; for (; a<10; a=a+2) b=1; return a;'
assert 5 'a=0; for (; a<5; a=a+1) b=1; return a;'
assert 10 'a=10; for (; a<5;) b=1; return a;'
assert 10 'a=10; for (; ; a=a+2) if (a >= 10) return a;'

# compound statements
assert 42 'a=0; b=0; if (1) {a=40; b=a+2;} return b;'
assert 84 'a=0; b=0; for (; a<42; a=a+1) {c=a+1; b=b+2;} return b;'

# function call
assert 42 'foo(); return 42;'

echo OK
