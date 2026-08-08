// Translated from solution.cpp.

func main()
{
  var i: dynamic;
  var m: dynamic;
  var n: dynamic;
  var y: dynamic;
  var x: dynamic;
  var a = cpp_array(106);
  read(n);
  {
    i = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  read(m);
  {
    i = 1;
    while ((i <= m))
    {
      read(x, y);
      a[(x - 1)] += (y - 1);
      a[(x + 1)] += (a[x] - y);
      a[x] = 0;
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      write(a[i], "\n");
      i += 1;
    }
  }
  return 0;
}
