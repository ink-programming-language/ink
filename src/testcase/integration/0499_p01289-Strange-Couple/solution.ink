// Translated from solution.cpp.

var maxn = 105;

var inf = 1e9;

var eps = 1e-12;

var n: dynamic;

var s: dynamic;

var t: dynamic;

var flag = cpp_array(maxn);

var mp = cpp_array(maxn);

func add_edge(u: dynamic, v: dynamic, d: dynamic)
{
  mp[u].push_back(make_pair(v, d));
  mp[v].push_back(make_pair(u, d));
}

var eq = cpp_array(maxn, maxn);

func gauss()
{
  {
    var i = 1;
    while ((i <= n))
    {
      var tmp = i;
      {
        var j = i;
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
        var j = i;
        while ((j <= n))
        {
          swap(eq[i][j], eq[tmp][j]);
          j += 1;
        }
      }
      swap(eq[i][0], eq[tmp][0]);
      {
        var j = (i + 1);
        while ((j <= n))
        {
          var tt = (eq[j][i] / eq[i][i]);
          {
            var k = i;
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
    var i = n;
    while ((i >= 1))
    {
      {
        var j = (i + 1);
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

var que: dynamic;

var dist = cpp_array(maxn);

var inq = cpp_array(maxn);

func spfa()
{
  {
    var i = 1;
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
    var u = que.front();
    inq[u] = false;
    que.pop();
    {
      var l = 0;
      while ((l < mp[u].size()))
      {
        var v = mp[u][l].first;
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

func work()
{
  {
    var i = 1;
    while ((i <= n))
    {
      mp[i].clear();
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 0;
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
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&flag[i]));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
        while ((j <= n))
        {
          var d: dynamic;
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
    var u = 1;
    while ((u <= n))
    {
      if ((u == t))
      {
        eq[u][u] = 1.00;
        u += 1;
        continue;
      }
      {
        var l = 0;
        while ((l < mp[u].size()))
        {
          var v = mp[u][l].first;
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

func main()
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
