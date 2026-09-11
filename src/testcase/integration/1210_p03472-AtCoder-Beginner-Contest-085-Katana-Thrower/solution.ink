// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var h: dynamic = cpp_uninitialized();
  read(n, h);
  var a: dynamic = cpp_array(n);
  var b: dynamic = cpp_array(n);
  var normal: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i], b[i]);
      normal = max(normal, a[i]);
      i += 1;
    }
  }
  sort(b, (b + n), greater());
  var x: dynamic = 0;
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
