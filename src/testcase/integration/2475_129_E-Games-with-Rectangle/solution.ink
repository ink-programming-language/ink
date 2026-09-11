// Translated from solution.cpp.

var c: dynamic = cpp_array(1005, 1005);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

func init() -> dynamic
{
  memset(c, 0, cpp_sizeof((c)));
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  {
    i = 0;
    while ((i <= 1000))
    {
      c[i][0] = 1;
      {
        j = 1;
        while ((j <= i))
        {
          c[i][j] = (((c[(i - 1)][(j - 1)] + c[(i - 1)][j])) % 1000000007);
          j += 1;
        }
      }
      i += 1;
    }
  }
}

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  init();
  while ((scanf("%d%d%d", (&n), (&m), (&k)) != EOF))
  {
    var ans: dynamic = 0;
    if ((((2 * k) <= (n - 1)) && ((2 * k) <= (m - 1))))
    {
      ans = (((c[(n - 1)][(2 * k)] * c[(m - 1)][(2 * k)])) % 1000000007);
    }
    printf("%I64d\n", ans);
  }
  return 0;
}
