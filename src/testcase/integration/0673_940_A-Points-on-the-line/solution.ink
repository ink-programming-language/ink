// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  read(n, d);
  var a: dynamic = cpp_new();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  sort(a, (a + n));
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = i;
        while ((j < n))
        {
          if ((((a[j] - a[i]) <= d) && (((j - i) + 1) > ans)))
          {
            ans = ((j - i) + 1);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(((n - ans)), "\n");
}
