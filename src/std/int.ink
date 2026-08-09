// Returns the smaller of two signed 32-bit integers.
public func min(Left: i32, Right: i32) -> i32 {
    return if (Left < Right) Left else Right;
}

// Returns the larger of two signed 32-bit integers.
public func max(Left: i32, Right: i32) -> i32 {
    return if (Left > Right) Left else Right;
}

// Restricts a signed 32-bit integer to an inclusive range.
public func clamp(Value: i32, Lower: i32, Upper: i32) -> i32 {
    return min(max(Value, Lower), Upper);
}
