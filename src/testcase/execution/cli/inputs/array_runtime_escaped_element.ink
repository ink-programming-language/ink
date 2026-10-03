// Returning an element pointer does not extend the callee-local array storage lifetime.
func escapedElement(): *i32
{
  var Values = [10, 20];
  var Index: i32 = 1;
  return &Values[Index];
}

func main(): i32
{
  var Address = escapedElement();
  return *Address;
}
