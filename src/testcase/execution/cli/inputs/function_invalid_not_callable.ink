// An integer local cannot be called using function-call syntax.
func main(): i32
{
  var Value: i32 = 7;
  return Value();
}
