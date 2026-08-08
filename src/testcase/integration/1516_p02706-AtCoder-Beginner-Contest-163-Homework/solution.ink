// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var m: dynamic;
  var x: dynamic;
  read(n, m);
  while (cpp_update(m, "--"))
  {
    read(x);
    n -= x;
  }
  if ((n < 0))
  {
    write("-1", "\n");
  } else
  {
    write(n, "\n");
  }
}
