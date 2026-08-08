// Translated from solution.cpp.

var MAXN = 100005;

var N: dynamic;

var L: dynamic;

var Q: dynamic;

var x = cpp_array(MAXN);

var nxt = cpp_array(25, MAXN);

var ans = cpp_array(MAXN);

func main()
{
  scanf("%d", (&N));
  {
    var i = 1;
    while ((i <= N))
    {
      scanf("%d", (&x[i]));
      i += 1;
    }
  }
  scanf("%d%d", (&L), (&Q));
  memset(nxt, 0x3F, cpp_sizeof(nxt));
  {
    var i = 1;
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
    var j = 1;
    while ((((1 << j)) < N))
    {
      {
        var i = 1;
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
    var i = 1;
    var a: dynamic;
    var b: dynamic;
    while ((i <= Q))
    {
      scanf("%d%d", (&a), (&b));
      if ((a > b))
      {
        swap(a, b);
      }
      var ans = 0;
      {
        var j = 20;
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
