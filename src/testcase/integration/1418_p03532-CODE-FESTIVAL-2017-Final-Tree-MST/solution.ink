// Translated from solution.cpp.

class edge
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  func operator_less(z: dynamic) -> dynamic
  {
      return ((x > z.x) || (((x == z.x) && (y < z.y))));
    }
}

var h: dynamic = cpp_uninitialized();

var v: dynamic = cpp_uninitialized();

class edge2
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var z: dynamic = cpp_uninitialized();
}

var e: dynamic = cpp_array(400005);

var n: dynamic = cpp_uninitialized();

var i: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var z: dynamic = cpp_uninitialized();

var tot: dynamic = 0;

var ev: dynamic = cpp_array(400005, 3);

var dsu: dynamic = cpp_array(200005);

var dis: dynamic = cpp_array(400005);

var pre: dynamic = cpp_array(400005);

var ans: dynamic = 0;

var fx: dynamic = cpp_uninitialized();

var fy: dynamic = cpp_uninitialized();

var inf: dynamic = 1e18;

var q: dynamic = cpp_uninitialized();

var s: dynamic = cpp_array(400005);

func getf(p: dynamic) -> dynamic
{
  return  ((dsu[p] == p)) ? p : cpp_assign(dsu[p], "=", getf(dsu[p]));
}

func cmp(a: dynamic, b: dynamic) -> dynamic
{
  if ((a.z != b.z))
  {
    return (a.z < b.z);
  }
  if ((a.x != b.x))
  {
    return (a.x < b.x);
  }
  return (a.y < b.y);
}

func main() -> dynamic
{
  scanf("%lld", (&n));
  {
    i = 1;
    while ((i <= n))
    {
      dsu[i] = i;
      q.push([0, (n + i)]);
      pre[i] = i;
      pre[(n + i)] = (n + i);
      dis[i] = inf;
      dis[(n + i)] = 0;
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%lld", (&x));
      s[i].push_back([(n + i), x]);
      s[(n + i)].push_back([i, x]);
      ev[0][i] = i;
      ev[1][i] = (n + i);
      ev[2][i] = x;
      i += 1;
    }
  }
  {
    i = 1;
    while ((i < n))
    {
      scanf("%lld%lld%lld", (&x), (&y), (&z));
      s[x].push_back([y, z]);
      s[y].push_back([x, z]);
      ev[0][(n + i)] = x;
      ev[1][(n + i)] = y;
      ev[2][(n + i)] = z;
      i += 1;
    }
  }
  while ((!q.empty()))
  {
    h = q.top();
    q.pop();
    if ((h.x > dis[h.y]))
    {
      continue;
    }
    {
      i = 0;
      while ((i < s[h.y].size()))
      {
        v = s[h.y][i];
        if ((dis[v.x] > (dis[h.y] + v.y)))
        {
          dis[v.x] = (dis[h.y] + v.y);
          pre[v.x] = pre[h.y];
          q.push([dis[v.x], v.x]);
        }
        i += 1;
      }
    }
  }
  {
    i = 1;
    while ((i <= ((2 * n) - 1)))
    {
      e[i] = [(pre[ev[0][i]] - n), (pre[ev[1][i]] - n), ((ev[2][i] + dis[ev[0][i]]) + dis[ev[1][i]])];
      i += 1;
    }
  }
  sort((e + 1), (e + (2 * n)), cmp);
  {
    i = 1;
    while ((i <= ((2 * n) - 1)))
    {
      fx = getf(e[i].x);
      fy = getf(e[i].y);
      if ((fx == fy))
      {
        i += 1;
        continue;
      }
      ans += e[i].z;
      dsu[fx] = fy;
      i += 1;
    }
  }
  printf("%lld\n", ans);
  return 0;
}
