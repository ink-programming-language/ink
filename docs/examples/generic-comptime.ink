class Box[T: type, WithTag: bool = false]
{
    class_field Value: T;

    comptime if (WithTag)
    {
        class_field Tag: int32;
    }

    class_method tagCount() -> int32
    {
        return comptime (WithTag ? 1 : 0);
    }
}

let TaggedIntBox: type = Box::[int32, true];
