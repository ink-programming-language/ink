// Translated from solution.cpp.

var dr: dynamic = [-1, -1, 0, 1, 1, 1, 0, -1];

var dc: dynamic = [0, 1, 1, 1, 0, -1, -1, -1];

var PI: dynamic = acos(-1);

var EPS: dynamic = 10e-9;

var e4: dynamic = (1e4 + 5);

var e5: dynamic = (1e5 + 5);

var e6: dynamic = (1e6 + 5);

var m: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var ans: dynamic = 0;
  scanf("%d", (&n));
  var str: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(str);
      m[str] += 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(str);
      if ((m[str] != 0))
      {
        m[str] -= 1;
      } else
      {
        ans += 1;
      }
      i += 1;
    }
  }
  printf("%d\n", ans);
  return 0;
}
