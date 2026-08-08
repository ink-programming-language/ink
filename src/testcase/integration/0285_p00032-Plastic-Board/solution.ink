// Translated from solution.cpp.

func main(argument_0: dynamic)
{
  var rectangle = 0;
  var lozenge = 0;
  while (true)
  {
    var a: dynamic;
    var b: dynamic;
    var c: dynamic;
    var e: dynamic;
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
