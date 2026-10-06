func expand[N: i32](): i32 { return expand::[N + 1](); }
func main(): i32 { return expand::[0](); }
