// Translated from solution.cpp.

var inf = 0x3f3f3f3f;

var INF = 0x3f3f3f3f3f3f3f3f;

var N = (5e5 + 7);

var M = 22;

var w = cpp_array(N);

var last: dynamic;

var sum = cpp_array(M, N);

var nx = cpp_array(M, N);

var cnt = 1;

func add(u: dynamic, v: dynamic)
{
  if ((w[u] <= w[v]))
  {
    nx[u][0] = v;
  } else
  {
    {
      var i = 20;
      while ((i >= 0))
      {
        if (((nx[v][i] != -1) && (w[nx[v][i]] < w[u])))
        {
          v = nx[v][i];
        }
        i -= 1;
      }
    }
    nx[u][0] = nx[v][0];
  }
  if ((nx[u][0] != -1))
  {
    sum[u][0] = w[nx[u][0]];
  }
  {
    var i = 1;
    while ((i <= 20))
    {
      if ((nx[u][(i - 1)] == -1))
      {
        nx[u][i] = -1;
      } else
      {
        nx[u][i] = nx[nx[u][(i - 1)]][(i - 1)];
        sum[u][i] = (sum[u][(i - 1)] + sum[nx[u][(i - 1)]][(i - 1)]);
      }
      i += 1;
    }
  }
}

func cal(u: dynamic, all: dynamic)
{
  if ((w[u] > all))
  {
    return 0;
  }
  all -= w[u];
  var ans = 1;
  {
    var i = 20;
    while ((i >= 0))
    {
      if (((u != -1) && (all >= sum[u][i])))
      {
        all -= sum[u][i];
        ans += ((1 << i));
        u = nx[u][i];
      }
      i -= 1;
    }
  }
  return ans;
}

func init()
{
  memset(sum, INF, cpp_sizeof((sum)));
  memset(nx, -1, cpp_sizeof((nx)));
  w[0] = INF;
}

func main()
{
  init();
  var q: dynamic;
  scanf("%d", (&q));
  while (cpp_update(q, "--"))
  {
    var op: dynamic;
    scanf("%d", (&op));
    if ((op == 1))
    {
      var p: dynamic;
      var q: dynamic;
      scanf("%lld", (&p));
      scanf("%lld", (&q));
      p ^= last;
      q ^= last;
      w[cpp_update(cnt, "++")] = q;
      add(cnt, p);
    } else
    {
      var p: dynamic;
      var q: dynamic;
      scanf("%lld", (&p));
      scanf("%lld", (&q));
      p ^= last;
      q ^= last;
      last = cal(p, q);
      printf("%lld\n", last);
    }
  }
  return 0;
}
