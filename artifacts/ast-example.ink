func identity[T: type](value: T): T
{
    return value;
}

var numbers = [1, 2, 3];
var result = identity::[i32](numbers[0]);
var text = "hello\0Ink";
