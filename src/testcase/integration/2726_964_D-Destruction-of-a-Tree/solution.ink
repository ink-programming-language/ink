// Translated from solution.cpp.

func read_file(outToFile: dynamic = 1)
{
  freopen("in", "r", stdin);
  if (outToFile)
  {
    freopen("out", "w", stdout);
  }
}

var v = cpp_array(300005);

var dp = cpp_array(2, 300005);

var ed = cpp_array(300005);

var ze = cpp_array(300005);

var bo = cpp_array(300005);

var on = cpp_array(300005);

var P = cpp_array(300005);

func dfs(node: dynamic, p: dynamic = -1)
{
  P[node] = p;
  var isleaf = true;
  var zero = 0;
  var one = 0;
  var both = 0;
  {
    var i = 0;
    while ((i < v[node].size()))
    {
      var cur = v[node][i];
      if ((cur == p))
      {
        i += 1;
        continue;
      }
      isleaf = false;
      dfs(cur, node);
      if ((dp[cur][0] == dp[cur][1]))
      {
        if ((dp[node][0] == 0))
        {
          dp[node][0] = cpp_assign(dp[node][1], "=", 0);
          return;
        } else
        {
          both += 1;
        }
      } else if ((dp[cur][0] == 1))
      {
        zero += 1;
      } else
      {
        one += 1;
      }
      i += 1;
    }
  }
  if (isleaf)
  {
    dp[node][1] = 0;
    dp[node][0] = 1;
  } else
  {
    dp[node][0] = cpp_assign(dp[node][1], "=", 0);
    var edges = ((zero + one) + both);
    ed[node] = edges;
    ze[node] = zero;
    on[node] = one;
    bo[node] = both;
    {
      var i = 0;
      while ((i <= 1))
      {
        if (((((i + edges)) % 2) == 0))
        {
          dp[node][i] = ((((one % 2) == 0) || ((((one % 2) == 1) && (both > 0)))));
        } else
        {
          dp[node][i] = ((((one % 2) == 1) || ((((one % 2) == 0) && (both > 0)))));
        }
        i += 1;
      }
    }
  }
}

func out(node: dynamic, state: dynamic)
{
  var parity = (((state + ed[node])) % 2);
  if (((parity % 2) == 0))
  {
    if (((on[node] % 2) == 0))
    {
      {
        var i = 0;
        while ((i < v[node].size()))
        {
          var cur = v[node][i];
          if ((cur == P[node]))
          {
            i += 1;
            continue;
          }
          if (((dp[cur][1] == 1) && (dp[cur][0] == 0)))
          {
            out(cur, 1);
          }
          i += 1;
        }
      }
      write((node + 1), "\n");
      {
        var i = 0;
        while ((i < v[node].size()))
        {
          var cur = v[node][i];
          if ((cur == P[node]))
          {
            i += 1;
            continue;
          }
          if (((dp[cur][1] == 1) && (dp[cur][0] == 0)))
          {
            i += 1;
            continue;
          }
          out(cur, 0);
          i += 1;
        }
      }
    } else
    {
      var mark = -1;
      {
        var i = 0;
        while ((i < v[node].size()))
        {
          var cur = v[node][i];
          if ((cur == P[node]))
          {
            i += 1;
            continue;
          }
          if (((dp[cur][1] == 1) && (dp[cur][0] == 0)))
          {
            out(cur, 1);
          }
          if ((((dp[cur][1] == 1) && (dp[cur][0] == 1)) && (mark == -1)))
          {
            out(cur, 1);
            mark = cur;
          }
          i += 1;
        }
      }
      write((node + 1), "\n");
      {
        var i = 0;
        while ((i < v[node].size()))
        {
          var cur = v[node][i];
          if (((cur == P[node]) || (cur == mark)))
          {
            i += 1;
            continue;
          }
          if (((dp[cur][1] == 1) && (dp[cur][0] == 0)))
          {
            i += 1;
            continue;
          }
          out(cur, 0);
          i += 1;
        }
      }
    }
  } else
  {
    if (((on[node] % 2) == 1))
    {
      {
        var i = 0;
        while ((i < v[node].size()))
        {
          var cur = v[node][i];
          if ((cur == P[node]))
          {
            i += 1;
            continue;
          }
          if (((dp[cur][1] == 1) && (dp[cur][0] == 0)))
          {
            out(cur, 1);
          }
          i += 1;
        }
      }
      write((node + 1), "\n");
      {
        var i = 0;
        while ((i < v[node].size()))
        {
          var cur = v[node][i];
          if ((cur == P[node]))
          {
            i += 1;
            continue;
          }
          if (((dp[cur][1] == 1) && (dp[cur][0] == 0)))
          {
            i += 1;
            continue;
          }
          out(cur, 0);
          i += 1;
        }
      }
    } else
    {
      var mark = -1;
      {
        var i = 0;
        while ((i < v[node].size()))
        {
          var cur = v[node][i];
          if ((cur == P[node]))
          {
            i += 1;
            continue;
          }
          if (((dp[cur][1] == 1) && (dp[cur][0] == 0)))
          {
            out(cur, 1);
          }
          if ((((dp[cur][1] == 1) && (dp[cur][0] == 1)) && (mark == -1)))
          {
            out(cur, 1);
            mark = cur;
          }
          i += 1;
        }
      }
      write((node + 1), "\n");
      {
        var i = 0;
        while ((i < v[node].size()))
        {
          var cur = v[node][i];
          if (((cur == P[node]) || (cur == mark)))
          {
            i += 1;
            continue;
          }
          if (((dp[cur][1] == 1) && (dp[cur][0] == 0)))
          {
            i += 1;
            continue;
          }
          out(cur, 0);
          i += 1;
        }
      }
    }
  }
}

func main()
{
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      var x: dynamic;
      read(x);
      if ((x != 0))
      {
        x -= 1;
        v[x].push_back(i);
        v[i].push_back(x);
      }
      i += 1;
    }
  }
  dfs(0);
  if ((dp[0][0] == 1))
  {
    write("YES\n");
    out(0, 0);
  } else
  {
    write("NO\n");
    return 0;
  }
}
