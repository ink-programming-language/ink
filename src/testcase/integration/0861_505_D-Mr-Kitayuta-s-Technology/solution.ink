// Translated from solution.cpp.

var N = 100005;

var v = cpp_array(N);

var n: dynamic;

var m: dynamic;

var f = cpp_array(N);

var deg = cpp_array(N);

var fa = cpp_array(N);

func getf(v: dynamic)
{
  return if ((f[v] == v)) v else cpp_assign(f[v], "=", getf(f[v]));
}

func merge(x: dynamic, y: dynamic)
{
  x = getf(x);
  y = getf(y);
  if ((x != y))
  {
    f[x] = y;
  }
}

var vis = cpp_array(N);

func main()
{
  ios.sync_with_stdio(false);
  read(n, m);
  {
    var i = 1;
    while ((i <= n))
    {
      f[i] = i;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      var t1: dynamic;
      var t2: dynamic;
      read(t1, t2);
      v[t1].push_back(t2);
      merge(t1, t2);
      deg[t2] += 1;
      i += 1;
    }
  }
  var ans = n;
  {
    var i = 1;
    while ((i <= n))
    {
      fa[i] = getf(i);
      if ((!vis[fa[i]]))
      {
        ans -= 1;
        vis[fa[i]] = 1;
      }
      i += 1;
    }
  }
  var q: dynamic;
  {
    var i = 1;
    while ((i <= n))
    {
      if ((!deg[i]))
      {
        q.push(i);
      }
      i += 1;
    }
  }
  while ((!q.empty()))
  {
    var nd = q.front();
    q.pop();
    for (var i in v[nd])
    {
      if ((!cpp_update(deg[i], "--")))
      {
        q.push(i);
      }
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if (deg[i])
      {
        ans += vis[fa[i]];
        vis[fa[i]] = 0;
      }
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
