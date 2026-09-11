// Translated from solution.cpp.

var MAXN: dynamic = 100005;

var N: dynamic = cpp_uninitialized();

var L: dynamic = cpp_uninitialized();

var Q: dynamic = cpp_uninitialized();

var x: dynamic = cpp_array(MAXN);

var nxt: dynamic = cpp_array(25, MAXN);

var ans: dynamic = cpp_array(MAXN);

func main() -> dynamic
{
  scanf("%d", (&N));
  {
    var i: dynamic = 1;
    while ((i <= N))
    {
      scanf("%d", (&x[i]));
      i += 1;
    }
  }
  scanf("%d%d", (&L), (&Q));
  memset(nxt, 0x3F, cpp_sizeof(nxt));
  {
    var i: dynamic = 1;
    while ((i <= N))
    {
      nxt[i][0] = (lower_bound((x + 1), ((x + N) + 1), (x[i] + L)) - x);
      if ((x[nxt[i][0]] > (x[i] + L)))
      {
        nxt[i][0] -= 1;
      }
      i += 1;
    }
  }
  {
    var j: dynamic = 1;
    while ((((1 << j)) < N))
    {
      {
        var i: dynamic = 1;
        while ((i <= N))
        {
          if (((nxt[i][(j - 1)] != 0x3F3F3F3F) && (nxt[nxt[i][(j - 1)]][(j - 1)] != 0x3F3F3F3F)))
          {
            nxt[i][j] = nxt[nxt[i][(j - 1)]][(j - 1)];
          }
          i += 1;
        }
      }
      j += 1;
    }
  }
  {
    var i: dynamic = 1;
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    while ((i <= Q))
    {
      scanf("%d%d", (&a), (&b));
      if ((a > b))
      {
        swap(a, b);
      }
      var ans: dynamic = 0;
      {
        var j: dynamic = 20;
        while ((j >= 0))
        {
          if ((nxt[a][j] <= b))
          {
            a = nxt[a][j];
            ans += ((1 << j));
          }
          j -= 1;
        }
      }
      if ((a < b))
      {
        ans += 1;
      }
      printf("%d\n", ans);
      i += 1;
    }
  }
  return 0;
}
