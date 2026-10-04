class Box
{
  field Value: i32;
};

func escape(): *i32
{
  var Object = Box(42);
  return &Object.Value;
}

func main(): i32
{
  var Address = escape();
  // The address does not keep Object alive. Dereferencing it would be undefined.
  return 42;
}
