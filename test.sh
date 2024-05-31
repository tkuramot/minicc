#!/bin/env bash

assert() {
  expected="$1"
  input="$2"

  ASAN_OPTIONS=detect_leaks=0 ./9cc "$input" > tmp.s
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

# function definition
assert 21 'fibo(n) { one_arg(n); if (n==0) return 0; if (n==1) return 1; return fibo(n-1) + fibo(n-2); } main() { return fibo(8); }'
assert 1 'one() { return 1; } main() { return one(); }'
assert 8 'number(n) { return n; } main() { return number(8); }'

# function call with return value
assert 1 'main() { a=one(); return a; }'
assert 1 'main() { return one(); }'
assert 2 'main() { return one() + 1; }'
assert 3 'main() { return 1 + one() + 1; }'
assert 3 'main() { return 1 + 1 + one(); }'
assert 3 'main() { return one() + 1 + 1; }'
assert 2 'main() { return one() + one(); }'
assert 3 'main() { a=one(); return a + one() + one(); }'
assert 1 'main() { return internal_call(); }'
assert 21 'main() { return fibonacci(8); }'
assert 41 'main() { a=42; return return_n(a-1); }'

# function call without return value
assert 42 'main() { no_arg(); return 42; }'
assert 42 'main() { one_arg(2); return 42; }'
assert 42 'main() { two_arg(3, 4); return 42; }'
assert 42 'main() { six_arg(1, 2, 3, 4, 5, 6); return 42; }'

# compound statements
assert 42 'main() { a=0; b=0; if (1) {a=40; b=a+2;} return b; }'
assert 84 'main() { a=0; b=0; for (; a<42; a=a+1) {c=a+1; b=b+2;} return b; }'

# for statements
assert 5 'main() { b=0; for (a=0; a<5; a=a+1) b=b+1; return b; }'
assert 10 'main() { a=0; for (; a<10; a=a+2) b=1; return a; }'
assert 5 'main() { a=0; for (; a<5; a=a+1) b=1; return a; }'
assert 10 'main() { a=10; for (; a<5;) b=1; return a; }'
assert 10 'main() { a=10; for (; ; a=a+2) if (a >= 10) return a; }'

# while statements
assert 5 'main() { a=0; while (a<5) a=a+1; }'

# if else statements
assert 2 'main() { if (0) return 4; else return 2; }'
assert 7 'main() { a=3; if (a==9) return 9; else if (a==8) return 8; else return 7; }'
assert 8 'main() { a=3; if (a==9) return 9; else if (a==3) return 8; else return 7; }'
assert 2 'main() { if (1) {} return 2; }'

# if statements
assert 3 'main() { a=3; if(1) return a; }'
assert 4 'main() { a=3; if (1) a=a+1;return a; }'
assert 3 'main() { a=3; if (0) a=a+1;return a; }'
assert 4 'main() { a=4; if (1) return 4; return 2; }'
assert 2 'main() { a=4; if (0) return 4; return 2; }'
assert 2 'main() { a=4; if (0) return 4; if (1) return 2; }'
assert 1 'main() { a=0; b=3; if (b==3) a=a+1; else if (b==3) a=a+1; return a; }'

# return statement
assert 4 'main() { return 4; }'
assert 4 'main() { a=4; return a; }'
assert 5 'main() { a=4; return a+1; }'

# multiple statements
assert 4 'main() { 3+3; a=4; }'
assert 5 'main() { a=4; a=a+1; }'

# local variables
assert 1 'main() { a=1; }'
assert 1 'main() { a=1==1; }'
assert 0 'main() { a=1==0; }'
assert 2 'main() { a=b=c=2; }'
assert 4 'main() { foo=4;bar=4; }'
assert 8 'main() { foo=4;bar=foo+4; }'
assert 12 'main() { f=4;foo=4;bar=f+foo+4; }'

# relational operators
assert 1 'main() { 1==1; }'
assert 0 'main() { 1!=1; }'
assert 1 'main() { 1!=0; }'
assert 0 'main() { 1==0; }'
assert 1 'main() { 0<1; }'
assert 0 'main() { 0>1; }'
assert 1 'main() { 1>0; }'
assert 0 'main() { 1<0; }'
assert 1 'main() { 0<=1; }'
assert 0 'main() { 0>=1; }'
assert 1 'main() { 1>=0; }'
assert 0 'main() { 1<=0; }'
assert 1 'main() { 1==1>=0; }'

# unary operators
assert 7 'main() { -3+10; }'
assert 5 'main() { -3*5+20; }'

# operator precedence
assert 47 'main() { 5+6*7; }'
assert 15 'main() { 5*(9-6); }'
assert 4 'main() { (3+5)/2; }'

# arithmetic operations
assert 21 'main() { 5+20-4; }'
assert 41 'main() {  12 + 34 - 5; }'

# exit status
assert 0 'main() { 0; }'
assert 42 'main() { 42; }'

echo OK
