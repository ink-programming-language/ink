// Translated from solution.cpp.

func main(argument_0: dynamic) -> dynamic
{
  var rectangle: dynamic = 0;
  var lozenge: dynamic = 0;
  while (true)
  {
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    var c: dynamic = cpp_uninitialized();
    var e: dynamic = cpp_uninitialized();
    read(a, e, b, e, c);
    if (cin.eof())
    {
      break;
    }
    if ((a == b))
    {
      lozenge += 1;
    }
    if ((((a * a) + (b * b)) == (c * c)))
    {
      rectangle += 1;
    }
  }
  write(rectangle, "\n");
  write(lozenge, "\n");
  return 0;
}
