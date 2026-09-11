// Translated from solution.cpp.

var N: dynamic = 510;

var data: dynamic = cpp_array(N, N);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N, N);

var s: dynamic = cpp_array(N, N);

var go: dynamic = [0, 0, -1, 0, 0, 1, 1, 0, 0, -1];

func judge(x: dynamic, y: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < 5))
    {
      if ((data[(x + go[i][0])][(y + go[i][1])] != cpp_char("1")))
      {
        return 0;
      }
      i += 1;
    }
  }
  return 1;
}

func cal(x1: dynamic, y1: dynamic, x2: dynamic, y2: dynamic) -> dynamic
{
  return (((s[x2][y2] - s[(x1 - 1)][y2]) - s[x2][(y1 - 1)]) + s[(x1 - 1)][(y1 - 1)]);
}

func main() -> dynamic
{
  while ((scanf("%d%d%d", (&n), (&m), (&k)) != EOF))
  {
    memset(a, 0, cpp_sizeof((a)));
    memset(s, 0, cpp_sizeof((s)));
    var i: dynamic = cpp_uninitialized();
    var j: dynamic = cpp_uninitialized();
    var p: dynamic = cpp_uninitialized();
    {
      i = 1;
      while ((i <= n))
      {
        scanf("%s", (data[i] + 1));
        i += 1;
      }
    }
    {
      i = 1;
      while ((i <= n))
      {
        {
          j = 1;
          while ((j <= m))
          {
            a[i][j] = judge(i, j);
            s[i][j] = (((s[(i - 1)][j] + s[i][(j - 1)]) - s[(i - 1)][(j - 1)]) + a[i][j]);
            j += 1;
          }
        }
        i += 1;
      }
    }
    var ans: dynamic = 0;
    {
      i = 1;
      while ((i <= n))
      {
        {
          j = (i + 2);
          while ((j <= n))
          {
            if ((cal((i + 1), 2, (j - 1), (m - 1)) < k))
            {
              j += 1;
              continue;
            }
            var l: dynamic = 1;
            {
              p = 3;
              while ((p <= m))
              {
                while (((l <= p) && (cal((i + 1), (l + 2), (j - 1), (p - 1)) >= k)))
                {
                  l += 1;
                }
                if ((cal((i + 1), (l + 1), (j - 1), (p - 1)) >= k))
                {
                  ans += l;
                }
                p += 1;
              }
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    printf("%I64d\n", ans);
  }
}
