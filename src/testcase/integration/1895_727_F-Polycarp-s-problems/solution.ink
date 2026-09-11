// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var u: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(1000);

var d: dynamic = cpp_array(1000);

var minn: dynamic = -1e17;

func main() -> dynamic
{
  scanf("%d%d", (&n), (&m));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%lld", (&a[i]));
      d[i] = minn;
      i += 1;
    }
  }
  {
    var i: dynamic = n;
    while ((i >= 0))
    {
      {
        var j: dynamic = n;
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
    var k: dynamic = (lower_bound(d, ((d + n) + 1), (-u)) - d);
    write(k, "\n");
  }
  return 0;
}
