// Translated from solution.cpp.

var dr = [-1, -1, 0, 1, 1, 1, 0, -1];

var dc = [0, 1, 1, 1, 0, -1, -1, -1];

var PI = acos(-1);

var EPS = 10e-9;

var e4 = (1e4 + 5);

var e5 = (1e5 + 5);

var e6 = (1e6 + 5);

var m: dynamic;

func main()
{
  var n: dynamic;
  var ans = 0;
  scanf("%d", (&n));
  var str: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      read(str);
      m[str] += 1;
      i += 1;
    }
  }
  {
    var i = 0;
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
