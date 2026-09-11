// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  var res: dynamic = cpp_uninitialized();
  for (var v: dynamic in vec)
  {
    read(v);
  }
  if ((k == 1))
  {
    return cpp_comma((cout << vec[0]), 0);
  }
  var l: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
        while ((j <= i))
        {
          l += 1;
          if ((l == k))
          {
            return cpp_comma((cout << vec[(j - 1)]), 0);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return 0;
}
