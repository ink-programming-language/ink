func value[N: i32](): i32 { return N; }
func main(): i32 { var Runtime: i32 = 42; return value::[Runtime](); }
