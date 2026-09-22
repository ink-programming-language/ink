## 1 概念

### 1.1 %ID 值
### 1.2 $ID 名字
### 1.3 ID: block

## 2 操作符

| 指令     | IR                                    |
|--------|---------------------------------------|
| add    | %DST = add %type %Operand1, %Operand2 |
| sub    | %DST = sub %type %Operand1, %Operand2 |
| alloca | %result = alloca <type> [, <count_type> <count>] [, align <alignment>] |
| store  | store [volatile] <type> <value>, ptr <address> [, align <alignment>]  |

## 3.函数

### 3.1 格式

```text
generic func @add<%arg: %type, ...>(%arg: %type, ...)->%type:
    entry_block(%x, %y):
        %r = dependent_binary "+" %x, %y;
        return %r;

generic class @Vector<%arg: %type, ...>(%arg: %type, ...):
    base_class:
        VecParent<>

    entry_block(%x, %y):
        %r = dependent_binary "+" %x, %y;
        return %r;
```

## 4.class
