func identity[T: type](Value: T): T { return Value; }
func main(): i32 { return identity::[](42); }
