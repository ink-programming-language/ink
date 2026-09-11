// Translated from solution.cpp.

var N: dynamic = 100010;

var inf: dynamic = (1 << 60);

var n: dynamic = cpp_uninitialized();

var tot: dynamic = cpp_uninitialized();

var to: dynamic = cpp_array((N << 1));

var nex: dynamic = cpp_array((N << 1));

var head: dynamic = cpp_array(N);

var ans: dynamic = 0;

var p: dynamic = cpp_array(N);

var sum: dynamic = cpp_array(N);

var flag: dynamic = cpp_uninitialized();

func SE(u: dynamic, v: dynamic) -> dynamic
{
  to[cpp_update(tot, "++")] = v;
  nex[tot] = head[u];
  head[u] = tot;
  return;
}

func GCD(x: dynamic, y: dynamic) -> dynamic
{
  var r: dynamic = (x % y);
  while (r)
  {
    x = y;
    y = r;
    r = (x % y);
  }
  return y;
}

func DFS(x: dynamic, fa: dynamic) -> dynamic
{
  var mi: dynamic = inf;
  var cnt: dynamic = 0;
  {
    var i: dynamic = head[x];
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
  var lcm: dynamic = 1;
  {
    var i: dynamic = head[x];
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

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%I64d", (&p[i]));
      ans += p[i];
      i += 1;
    }
  }
  {
    var u: dynamic = cpp_uninitialized();
    var v: dynamic = cpp_uninitialized();
    var i: dynamic = 1;
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
