// Translated from solution.cpp.

var maxn: dynamic = 105;

var inf: dynamic = 1e9;

var eps: dynamic = 1e-12;

var n: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var flag: dynamic = cpp_array(maxn);

var mp: dynamic = cpp_array(maxn);

func add_edge(u: dynamic, v: dynamic, d: dynamic) -> dynamic
{
  mp[u].push_back(make_pair(v, d));
  mp[v].push_back(make_pair(u, d));
}

var eq: dynamic = cpp_array(maxn, maxn);

func gauss() -> dynamic
{
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var tmp: dynamic = i;
      {
        var j: dynamic = i;
        while ((j <= n))
        {
          if ((fabs(eq[j][i]) > fabs(eq[tmp][i])))
          {
            tmp = j;
          }
          j += 1;
        }
      }
      {
        var j: dynamic = i;
        while ((j <= n))
        {
          swap(eq[i][j], eq[tmp][j]);
          j += 1;
        }
      }
      swap(eq[i][0], eq[tmp][0]);
      {
        var j: dynamic = (i + 1);
        while ((j <= n))
        {
          var tt: dynamic = (eq[j][i] / eq[i][i]);
          {
            var k: dynamic = i;
            while ((k <= n))
            {
              eq[j][k] -= (eq[i][k] * tt);
              k += 1;
            }
          }
          eq[j][0] -= (eq[i][0] * tt);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = n;
    while ((i >= 1))
    {
      {
        var j: dynamic = (i + 1);
        while ((j <= n))
        {
          eq[i][0] -= (eq[i][j] * eq[j][0]);
          j += 1;
        }
      }
      eq[i][0] /= eq[i][i];
      i -= 1;
    }
  }
  return eq[s][0];
}

var que: dynamic = cpp_uninitialized();

var dist: dynamic = cpp_array(maxn);

var inq: dynamic = cpp_array(maxn);

func spfa() -> dynamic
{
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      dist[i] = inf;
      i += 1;
    }
  }
  dist[t] = 0;
  que.push(t);
  while ((!que.empty()))
  {
    var u: dynamic = que.front();
    inq[u] = false;
    que.pop();
    {
      var l: dynamic = 0;
      while ((l < mp[u].size()))
      {
        var v: dynamic = mp[u][l].first;
        if ((dist[v] <= (dist[u] + mp[u][l].second)))
        {
          l += 1;
          continue;
        }
        dist[v] = (dist[u] + mp[u][l].second);
        if ((!inq[v]))
        {
          inq[v] = true;
          que.push(v);
        }
        l += 1;
      }
    }
  }
}

func work() -> dynamic
{
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      mp[i].clear();
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 0;
        while ((j <= n))
        {
          eq[i][j] = 0.00;
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&flag[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
        while ((j <= n))
        {
          var d: dynamic = cpp_uninitialized();
          scanf("%d", (&d));
          if (((i < j) || (d == 0)))
          {
            j += 1;
            continue;
          }
          add_edge(i, j, d);
          j += 1;
        }
      }
      i += 1;
    }
  }
  spfa();
  if ((dist[s] == inf))
  {
    puts("impossible");
    return;
  }
  {
    var u: dynamic = 1;
    while ((u <= n))
    {
      if ((u == t))
      {
        eq[u][u] = 1.00;
        u += 1;
        continue;
      }
      {
        var l: dynamic = 0;
        while ((l < mp[u].size()))
        {
          var v: dynamic = mp[u][l].first;
          if ((flag[u] && (dist[u] != (dist[v] + mp[u][l].second))))
          {
            l += 1;
            continue;
          }
          eq[u][v] += 1.00;
          eq[u][0] -= (1.00 * mp[u][l].second);
          eq[u][u] -= 1.00;
          l += 1;
        }
      }
      u += 1;
    }
  }
  printf("%.10f\n", (gauss() + eps));
}

func main() -> dynamic
{
  while (true)
  {
    scanf("%d%d%d", (&n), (&s), (&t));
    if ((((n + s) + t) == 0))
    {
      break;
    }
    work();
  }
  return 0;
}
