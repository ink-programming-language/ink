// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var u: dynamic;

var a = cpp_array(1000);

var d = cpp_array(1000);

var minn = -1e17;

func main()
{
  scanf("%d%d", (&n), (&m));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%lld", (&a[i]));
      d[i] = minn;
      i += 1;
    }
  }
  {
    var i = n;
    while ((i >= 0))
    {
      {
        var j = n;
        while ((j >= 0))
        {
          if ((j == 0))
          {
            u = minn;
          } else
          {
            u = d[(j - 1)];
          }
          d[j] = max(min((d[j] + a[i]), 0), u);
          j -= 1;
        }
      }
      i -= 1;
    }
  }
  while (cpp_update(m, "--"))
  {
    scanf("%lld", (&u));
    var k = (lower_bound(d, ((d + n) + 1), (-u)) - d);
    write(k, "\n");
  }
  return 0;
}
