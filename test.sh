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

# int and ptr type
assert 3 'int main() { int x; int *y=&x; *y=3; return x; }'

# unary operators; * and & operators
assert 3 'int main() { int a=3; int *b=&a; return *b; }'
assert 42 'int main() { int a=42; return *&a; }'
assert 3 'int main() { int x=3; int y=4; int *z=&y+8; return *z; }'

# function definition
assert 21 'int fibo(int n) { if (n==0) return 0; if (n==1) return 1; return fibo(n-1) + fibo(n-2); } int main() { return fibo(8); }'
assert 1 'int one() { return 1; } int main() { return one(); }'
assert 8 'int number(int n) { return n; } int main() { return number(8); }'

# function call with return value
assert 1 'int main() { int a=one(); return a; }'
assert 1 'int main() { return one(); }'
assert 2 'int main() { return one() + 1; }'
assert 3 'int main() { return 1 + one() + 1; }'
assert 3 'int main() { return 1 + 1 + one(); }'
assert 3 'int main() { return one() + 1 + 1; }'
assert 2 'int main() { return one() + one(); }'
assert 3 'int main() { int a=one(); return a + one() + one(); }'
assert 1 'int main() { return internal_call(); }'
assert 21 'int main() { return fibonacci(8); }'
assert 41 'int main() { int a=42; return return_n(a-1); }'

# function call without return value
assert 42 'int main() { no_arg(); return 42; }'
assert 42 'int main() { one_arg(2); return 42; }'
assert 42 'int main() { two_arg(3, 4); return 42; }'
assert 42 'int main() { six_arg(1, 2, 3, 4, 5, 6); return 42; }'

# compound statements
assert 42 'int main() { int a=0; int b=0; if (1) {a=40; b=a+2;} return b; }'
assert 84 'int main() { int a=0; int b=0; for (; a<42; a=a+1) {int c=a+1; b=b+2;} return b; }'

# for statements
assert 5 'int main() { int b=0; for (int a=0; a<5; a=a+1) b=b+1; return b; }'
assert 10 'int main() { int a=0; for (; a<10; a=a+2) int b=1; return a; }'
assert 5 'int main() { int a=0; for (; a<5; a=a+1) int b=1; return a; }'
assert 10 'int main() { int a=10; for (; a<5;) int b=1; return a; }'
assert 10 'int main() { int a=10; for (; ; a=a+2) if (a >= 10) return a; }'

# while statements
assert 5 'int main() { int a=0; while (a<5) a=a+1; }'

# if else statements
assert 2 'int main() { if (0) return 4; else return 2; }'
assert 7 'int main() { int a=3; if (a==9) return 9; else if (a==8) return 8; else return 7; }'
assert 8 'int main() { int a=3; if (a==9) return 9; else if (a==3) return 8; else return 7; }'
assert 2 'int main() { if (1) {} return 2; }'

# if statements
assert 3 'int main() { int a=3; if(1) return a; }'
assert 4 'int main() { int a=3; if (1) a=a+1;return a; }'
assert 3 'int main() { int a=3; if (0) a=a+1;return a; }'
assert 4 'int main() { int a=4; if (1) return 4; return 2; }'
assert 2 'int main() { int a=4; if (0) return 4; return 2; }'
assert 2 'int main() { int a=4; if (0) return 4; if (1) return 2; }'
assert 1 'int main() { int a=0; int b=3; if (b==3) a=a+1; else if (b==3) a=a+1; return a; }'

# return statement
assert 4 'int main() { return 4; }'
assert 4 'int main() { int a=4; return a; }'
assert 5 'int main() { int a=4; return a+1; }'

# multiple statements
assert 4 'int main() { 3+3; int a=4; }'
assert 5 'int main() { int a=4; a=a+1; }'

# local variables
assert 1 'int main() { int a=1; }'
assert 1 'int main() { int a=1==1; }'
assert 0 'int main() { int a=1==0; }'
assert 4 'int main() { int foo=4; int bar=4; }'
assert 8 'int main() { int foo=4; int bar=foo+4; }'
assert 12 'int main() { int f=4; int foo=4; int bar=f+foo+4; }'

# relational operators
assert 1 'int main() { 1==1; }'
assert 0 'int main() { 1!=1; }'
assert 1 'int main() { 1!=0; }'
assert 0 'int main() { 1==0; }'
assert 1 'int main() { 0<1; }'
assert 0 'int main() { 0>1; }'
assert 1 'int main() { 1>0; }'
assert 0 'int main() { 1<0; }'
assert 1 'int main() { 0<=1; }'
assert 0 'int main() { 0>=1; }'
assert 1 'int main() { 1>=0; }'
assert 0 'int main() { 1<=0; }'
assert 1 'int main() { 1==1>=0; }'

# unary operators; + and - operators
assert 7 'int main() { -3++10; }'
assert 7 'int main() { -3+10; }'
assert 5 'int main() { -3*5+20; }'

# operator precedence
assert 47 'int main() { 5+6*7; }'
assert 15 'int main() { 5*(9-6); }'
assert 4 'int main() { (3+5)/2; }'

# arithmetic operations
assert 21 'int main() { 5+20-4; }'
assert 41 'int main() { 12 + 34 - 5; }'

# exit status
assert 0 'int main() { 0; }'
assert 42 'int main() { 42; }'

echo OK
