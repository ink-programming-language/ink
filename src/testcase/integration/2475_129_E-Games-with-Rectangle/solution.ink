// Translated from solution.cpp.

var c = cpp_array(1005, 1005);

var n: dynamic;

var m: dynamic;

var k: dynamic;

func init()
{
  memset(c, 0, cpp_sizeof((c)));
  var i: dynamic;
  var j: dynamic;
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

func main()
{
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  init();
  while ((scanf("%d%d%d", (&n), (&m), (&k)) != EOF))
  {
    var ans = 0;
    if ((((2 * k) <= (n - 1)) && ((2 * k) <= (m - 1))))
    {
      ans = (((c[(n - 1)][(2 * k)] * c[(m - 1)][(2 * k)])) % 1000000007);
    }
    printf("%I64d\n", ans);
  }
  return 0;
}
