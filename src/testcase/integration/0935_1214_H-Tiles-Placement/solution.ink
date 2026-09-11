// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var root: dynamic = cpp_uninitialized();

var d: dynamic = cpp_array(200005);

var h: dynamic = cpp_array(200005);

var res: dynamic = cpp_array(200005);

var a: dynamic = cpp_array(200005);

func dfs(u: dynamic, p: dynamic) -> dynamic
{
  var tmp: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(a[u].size())))
    {
      var v: dynamic = a[u][i];
      if ((v == p))
      {
        i += 1;
        continue;
      }
      d[v] = (d[u] + 1);
      dfs(v, u);
      tmp = max(tmp, h[v]);
      i += 1;
    }
  }
  h[u] = max(d[u], tmp);
}

func color(u: dynamic, p: dynamic, cur: dynamic, step: dynamic, diameter: dynamic = true) -> dynamic
{
  cur += step;
  if ((cur > k))
  {
    cur -= k;
  } else if ((cur <= 0))
  {
    cur += k;
  }
  res[u] = cur;
  var U: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(a[u].size())))
    {
      var v: dynamic = a[u][i];
      if ((v == p))
      {
        i += 1;
        continue;
      }
      if (diameter)
      {
        if (((U == 0) && (h[v] == h[root])))
        {
          U = v;
          i += 1;
          continue;
        }
        var len: dynamic = ((h[v] - d[v]) + 1);
        if ((((k > 2) && ((len + d[u]) >= k)) && ((((len + h[root]) - d[u]) + 1) >= k)))
        {
          write("No");
          exit(0);
        } else if (((len + d[u]) >= k))
        {
          color(v, u, cur, step, false);
        } else
        {
          color(v, u, cur, (-step), false);
        }
      } else
      {
        color(v, u, cur, step, false);
      }
      i += 1;
    }
  }
  if (U)
  {
    color(U, u, cur, step);
  }
}

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(n, k);
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      read(u, v);
      a[u].push_back(v);
      a[v].push_back(u);
      i += 1;
    }
  }
  dfs(1, 0);
  {
    var i: dynamic = 2;
    while ((i <= n))
    {
      if ((d[i] == h[1]))
      {
        root = i;
      }
      i += 1;
    }
  }
  d[root] = 1;
  dfs(root, 0);
  color(root, 0, 0, 1);
  write("Yes\n");
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      write(res[i], " ");
      i += 1;
    }
  }
  return 0;
}
