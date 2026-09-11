// Translated from solution.cpp.

var mod: dynamic = 1000000007;

var sx: dynamic = mod;

var sy: dynamic = mod;

var bx: dynamic = cpp_uninitialized();

var by: dynamic = cpp_uninitialized();

var cnt: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var vis: dynamic = cpp_array(5001);

var cost: dynamic = cpp_array(5001, 5001);

var x: dynamic = cpp_array(5001);

var y: dynamic = cpp_array(5001);

func dfs(v: dynamic, c: dynamic, mid: dynamic) -> dynamic
{
  if ((vis[v] && (c != vis[v])))
  {
    return 0;
  }
  if (vis[v])
  {
    return 1;
  }
  vis[v] = c;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((cost[v][i] > mid))
      {
        if ((!dfs(i, (3 - c), mid)))
        {
          return 0;
        }
      }
      i += 1;
    }
  }
  return 1;
}

func check(mid: dynamic) -> dynamic
{
  cnt = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((!vis[i]))
      {
        if ((!dfs(i, 1, mid)))
        {
          return 0;
        }
        cnt += 1;
      }
      i += 1;
    }
  }
  return 1;
}

func out() -> dynamic
{
  var ans: dynamic = 1;
  {
    var i: dynamic = 1;
    while ((i <= cnt))
    {
      ans = ((ans * 2) % mod);
      i += 1;
    }
  }
  write(ans, "\n");
}

func main() -> dynamic
{
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d%d", (&x[i]), (&y[i]));
      sx = min(x[i], sx);
      sy = min(y[i], sy);
      bx = max(x[i], bx);
      by = max(y[i], by);
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
          cost[i][j] = (abs((x[j] - x[i])) + abs((y[j] - y[i])));
          j += 1;
        }
      }
      i += 1;
    }
  }
  var lb: dynamic = 0;
  var ub: dynamic = (((bx + by) - sx) - sy);
  while ((lb <= ub))
  {
    memset(vis, 0, cpp_sizeof((vis)));
    var mid: dynamic = (((lb + ub)) / 2);
    if ((!check(mid)))
    {
      lb = (mid + 1);
    } else
    {
      ub = (mid - 1);
    }
  }
  write(lb, "\n");
  memset(vis, 0, cpp_sizeof((vis)));
  check(lb);
  out();
  return 0;
}
