import "C" func puts(Text: *u8): i32;

class Watch
{
    field Count: *i32;
    field Tag: i32;

    func __init__(Count: *i32, Tag: i32): void
    {
        this.Count = Count;
        this.Tag = Tag;
    }

    func __del__(): void
    {
        (*this.Count)++;
        if (this.Tag == 1) puts("body");
        else if (this.Tag == 2) puts("inner");
        else puts("header");
    }

    func yes(): bool
    {
        return true;
    }
};

func run(Count: *i32): void
{
    for (var Header = Watch(Count, 3); true;)
    {
        var Body = Watch(Count, 1);
        return;
    }
}

func main(): i32
{
    var Count = 0;
    var I = 0;
    for (var Header = Watch(&Count, 3); I < 4; I++)
    {
        var Body = Watch(&Count, 1);
        if (I == 0) continue;
        var Inner = Watch(&Count, 2);
        if (I == 2) break;
    }
    if (Count != 6) return 1;
    run(&Count);
    if (Count != 8) return 2;
    for (Item in [Watch(&Count, 1), Watch(&Count, 2)])
    {
        break;
    }
    if (Count != 11) return 3;
    I = 0;
    while (I < 2 && Watch(&Count, 1).yes())
    {
        I++;
        continue;
    }
    if (Count != 13) return 4;
    for (I = 0; I < 3; I++, Watch(&Count, 2))
    {
        if (I == 2) break;
    }
    if (Count != 15) return 5;
    return 42;
}
