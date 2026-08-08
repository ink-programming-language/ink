// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var i = 1;
  read(n);
  read(a);
  while (true)
  {
    read(b);
    if ((a < b))
    {
      c = b;
      b = a;
      a = c;
    }
    while ((a % b))
    {
      c = b;
      b = (a % b);
      a = c;
    }
    a = b;
    i += 1;
    if (!(((i < n))))
    {
      break;
    }
  }
  write(a);
  return 0;
}
