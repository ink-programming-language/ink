// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var a = cpp_array(100000);
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  var btn = 1;
  var count = 0;
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
