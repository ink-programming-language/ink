// Translated from solution.cpp.

var N: dynamic = 100005;

var v: dynamic = cpp_array(N);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var f: dynamic = cpp_array(N);

var deg: dynamic = cpp_array(N);

var fa: dynamic = cpp_array(N);

func getf(v: dynamic) -> dynamic
{
  return  ((f[v] == v)) ? v : cpp_assign(f[v], "=", getf(f[v]));
}

func merge(x: dynamic, y: dynamic) -> dynamic
{
  x = getf(x);
  y = getf(y);
  if ((x != y))
  {
    f[x] = y;
  }
}

var vis: dynamic = cpp_array(N);

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  read(n, m);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      f[i] = i;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      var t1: dynamic = cpp_uninitialized();
      var t2: dynamic = cpp_uninitialized();
      read(t1, t2);
      v[t1].push_back(t2);
      merge(t1, t2);
      deg[t2] += 1;
      i += 1;
    }
  }
  var ans: dynamic = n;
  {
    var i: dynamic = 1;
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
  var q: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
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
    var nd: dynamic = q.front();
    q.pop();
    for (var i: dynamic in v[nd])
    {
      if ((!cpp_update(deg[i], "--")))
      {
        q.push(i);
      }
    }
  }
  {
    var i: dynamic = 1;
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
