func sum(Limit: i32): i32
{
    var Result = 0;
    for (var I = 0; I < Limit; I++)
    {
        if (I == 2) continue;
        Result += I;
        if (I == 4) break;
    }
    return Result;
}

func make(Calls: *i32): [i32; 3]
{
    (*Calls)++;
    return [1, 2, 3];
}

func main(): i32
{
    if (sum(0) != 0 || sum(1) != 0 || sum(10) != 8) return 1;
    var I = 0;
    var Count = 0;
    while (I++ < 4)
    {
        for (var J = 3; J > 0; --J)
        {
            if (J == 2) continue;
            Count++;
        }
    }
    if (I != 5 || Count != 8) return 2;
    var Calls = 0;
    var Total = 0;
    for (X in make(&Calls)) Total += X;
    if (Calls != 1 || Total != 6) return 3;
    var Source = [1, 2, 3];
    Total = 0;
    for (X in Source)
    {
        Source[1] = 99;
        X++;
        Total += X;
    }
    if (Total != 9) return 4;
    Total = 0;
    for ([Head, Tail...] in [[1, 2, 3], [4, 5, 6]])
    {
        Total += Head;
        for (X in Tail) Total += X;
    }
    if (Total != 21) return 5;
    var Empty: [i32; 0] = [];
    for (_ in Empty) return 6;
    var Answer: i32;
    for (;;)
    {
        Answer = comptime sum(10);
        break;
    }
    if (Answer != 8) return 7;
    var Steps = 0;
    for (I = 0; I < 4; I++, Steps += I)
    {
        if (I == 1) continue;
        if (I == 3) break;
    }
    if (Steps != 6) return 8;
    return 42;
}
