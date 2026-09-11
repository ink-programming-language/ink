// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var a: dynamic = cpp_array(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  sort(a, (a + n));
  if ((n == 1))
  {
    write(a[0], "\n");
    return 0;
  }
  var c: dynamic = (((a[0] + a[1])) / 2);
  {
    var i: dynamic = 2;
    while ((i < n))
    {
      c = (((c + a[i])) / 2);
      i += 1;
    }
  }
  write(c, "\n");
}
