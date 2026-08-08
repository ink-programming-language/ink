// Translated from solution.cpp.

var N = 100010;

var inf = (1 << 60);

var n: dynamic;

var tot: dynamic;

var to = cpp_array((N << 1));

var nex = cpp_array((N << 1));

var head = cpp_array(N);

var ans = 0;

var p = cpp_array(N);

var sum = cpp_array(N);

var flag: dynamic;

func SE(u: dynamic, v: dynamic)
{
  to[cpp_update(tot, "++")] = v;
  nex[tot] = head[u];
  head[u] = tot;
  return;
}

func GCD(x: dynamic, y: dynamic)
{
  var r = (x % y);
  while (r)
  {
    x = y;
    y = r;
    r = (x % y);
  }
  return y;
}

func DFS(x: dynamic, fa: dynamic)
{
  var mi = inf;
  var cnt = 0;
  {
    var i = head[x];
    while (i)
    {
      if ((to[i] == fa))
      {
        i = nex[i];
        continue;
      }
      DFS(to[i], x);
      if (flag)
      {
        return;
      }
      if ((p[to[i]] < mi))
      {
        mi = p[to[i]];
      }
      cnt += 1;
      i = nex[i];
    }
  }
  if ((cnt == 0))
  {
    sum[x] = 1;
    return;
  }
  var lcm = 1;
  {
    var i = head[x];
    while (i)
    {
      if ((to[i] == fa))
      {
        i = nex[i];
        continue;
      }
      if ((((cpp_cast(lcm) / GCD(lcm, sum[to[i]])) * sum[to[i]]) > cpp_cast(mi)))
      {
        flag = 1;
        return;
      }
      lcm = ((lcm / GCD(lcm, sum[to[i]])) * sum[to[i]]);
      i = nex[i];
    }
  }
  p[x] = (((mi / lcm) * lcm) * cnt);
  sum[x] = (lcm * cnt);
  return;
}

func main()
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%I64d", (&p[i]));
      ans += p[i];
      i += 1;
    }
  }
  {
    var u: dynamic;
    var v: dynamic;
    var i = 1;
    while ((i < n))
    {
      scanf("%d%d", (&u), (&v));
      SE(u, v);
      SE(v, u);
      i += 1;
    }
  }
  DFS(1, 0);
  if ((!flag))
  {
    ans -= p[1];
  }
  printf("%I64d\n", ans);
  return 0;
}
