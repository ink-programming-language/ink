// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var h: dynamic;
  read(n, h);
  var a = cpp_array(n);
  var b = cpp_array(n);
  var normal = 0;
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i], b[i]);
      normal = max(normal, a[i]);
      i += 1;
    }
  }
  sort(b, (b + n), greater());
  var x = 0;
  while ((((x < n) && (b[x] > normal)) && (h > 0)))
  {
    h -= b[x];
    x += 1;
  }
  if ((h <= 0))
  {
    write(x, "\n");
  } else
  {
    write(((x + (((h - 1)) / normal)) + 1), "\n");
  }
  return 0;
}
