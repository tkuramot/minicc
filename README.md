# minicc

## features

- arithmetic operators
- operator precedence
- relational and logical operators
- multiple statements
- variable declaration and assignments
- conditional statements
- iteration structure (while, for)
- compound statements
- function call with and without arguments
- function definition
- address and dereference

## syntax

```bash
program    = func*
func       = ident "(" params? ")" "{" stmt* "}"
stmt       = expr ";"
            | "{" stmt* "}"
            | "if" "(" expr ")" stmt ("else" stmt)?
            | "while" "(" expr ")" stmt
            | "for" "(" expr? ";" expr? ";" expr? ")" stmt
            | "return" expr ";"
expr       = assign
assign     = equality ("=" assign)?
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary)*
unary      = "+"? primary | "-"? primary | "*" unary | "&" unary
primary    = num | ident ("(" args? ")")? | "(" expr ")"
args       = expr ("," expr)*
params     = ident ("," ident)*
```

## references

- [低レイヤを知りたい人のためのCコンパイラ作成入門](https://www.sigbus.info/compilerbook)
- [System V ABI](https://wiki.osdev.org/System_V_ABI)
- [System V ABI - AMD64 Architecture Processor Supplement](https://gitlab.com/x86-psABIs/x86-64-ABI)
