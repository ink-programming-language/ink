// Translated from solution.cpp.

var n: dynamic;

var k: dynamic;

var root: dynamic;

var d = cpp_array(200005);

var h = cpp_array(200005);

var res = cpp_array(200005);

var a = cpp_array(200005);

func dfs(u: dynamic, p: dynamic)
{
  var tmp = 0;
  {
    var i = 0;
    while ((i < cpp_cast(a[u].size())))
    {
      var v = a[u][i];
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

func color(u: dynamic, p: dynamic, cur: dynamic, step: dynamic, diameter: dynamic = true)
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
  var U = 0;
  {
    var i = 0;
    while ((i < cpp_cast(a[u].size())))
    {
      var v = a[u][i];
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
        var len = ((h[v] - d[v]) + 1);
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

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  read(n, k);
  {
    var i = 1;
    while ((i < n))
    {
      var u: dynamic;
      var v: dynamic;
      read(u, v);
      a[u].push_back(v);
      a[v].push_back(u);
      i += 1;
    }
  }
  dfs(1, 0);
  {
    var i = 2;
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
    var i = 1;
    while ((i <= n))
    {
      write(res[i], " ");
      i += 1;
    }
  }
  return 0;
}
