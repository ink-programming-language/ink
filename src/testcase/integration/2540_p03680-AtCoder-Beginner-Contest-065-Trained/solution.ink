// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_array(100000);
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var btn: dynamic = 1;
  var count: dynamic = 0;
  while (1)
  {
    if ((btn == 2))
    {
      write(count, "\n");
      return 0;
    }
    btn = a[btn];
    count += 1;
    if ((count > (n + 1)))
    {
      write(-1, "\n");
      return 0;
    }
  }
}
