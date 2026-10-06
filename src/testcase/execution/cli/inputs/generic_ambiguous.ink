func choose[T: type](Value: T): i32 { return 1; }
func choose[T: type](Value: i32): i32 { return 2; }
func main(): i32 { return choose::[i32](42); }
