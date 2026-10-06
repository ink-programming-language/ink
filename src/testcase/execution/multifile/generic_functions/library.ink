private func helper(Value: i32): i32 { return Value + 1; }
func identity[T: type](Value: T): T { return Value; }
func offset[N: i32](Value: i32): i32 { return helper(Value) + N; }
private func hidden[T: type](Value: T): T { return Value; }
