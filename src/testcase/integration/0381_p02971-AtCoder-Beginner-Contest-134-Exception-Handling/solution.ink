// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var max: dynamic = 0;
  var b: dynamic = 0;
  read(n);
  var a: dynamic = cpp_array(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      if ((max < a[i]))
      {
        max = a[i];
        b = i;
      }
      i += 1;
    }
  }
  sort(a, (a + n));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((i != b))
      {
        write(max, "\n");
      } else
      {
        write(a[(n - 2)], "\n");
      }
      i += 1;
    }
  }
}
