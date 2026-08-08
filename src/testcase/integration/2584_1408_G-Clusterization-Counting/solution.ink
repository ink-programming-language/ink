// Translated from solution.cpp.

var c = 1502;

var mod = 998244353;

var dp = cpp_array(c, c);

var el = cpp_array(c, c);

var inv = cpp_array(c, c);

var f = cpp_array(c);

var db = cpp_array(c);

var ki = cpp_array(c);

var n: dynamic;

var cnt = 0;

var v = cpp_array(c);

var h = cpp_array(c);

var kesz: dynamic;

var sz = cpp_array(c);

var jo = cpp_array(c);

var q: dynamic;

func dfs(a: dynamic, b: dynamic)
{
  if ((!v[a]))
  {
    v[a] = 1;
    cnt += 1;
  }
  if ((a < b))
  {
    kesz = 1;
    {
      var i = 0;
      while ((i < jo[a].size()))
      {
        if ((jo[a][i] >= inv[a][b]))
        {
          jo[b].push_back(jo[a][i]);
        }
        i += 1;
      }
    }
    return;
  }
  while (((f[a] + 1) <= cnt))
  {
    f[a] += 1;
    var x = sz[a][f[a]].second;
    if ((!v[x]))
    {
      dfs(x, b);
      if (kesz)
      {
        return;
      }
    }
    if (((f[a] == cnt) && (a == b)))
    {
      {
        var i = 1;
        while ((i <= cnt))
        {
          var x = sz[a][i].second;
          if ((f[x] != cnt))
          {
            dfs(x, b);
          }
          i += 1;
        }
      }
    }
    if (((f[a] == cnt) && (a == b)))
    {
      jo[b].push_back(cnt);
      if ((cnt < n))
      {
        dfs(sz[a][(cnt + 1)].second, b);
        if (kesz)
        {
          return;
        }
      }
    }
  }
}

func unio(a: dynamic, b: dynamic)
{
  h[b] = 1;
  var sb = 0;
  var sa = 0;
  var st = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      if (dp[b][i])
      {
        sb = i;
      }
      if (dp[a][i])
      {
        sa = i;
      }
      if ((dp[a][i] && (!st)))
      {
        st = i;
      }
      i += 1;
    }
  }
  {
    var i = (sa + sb);
    while ((i >= 1))
    {
      dp[a][i] = 0;
      {
        var j = 1;
        while ((j <= min(i, sb)))
        {
          dp[a][i] += (dp[a][(i - j)] * dp[b][j]);
          dp[a][i] %= mod;
          j += 1;
        }
      }
      i -= 1;
    }
  }
  {
    var i = 1;
    while ((i < st))
    {
      dp[a][i] = 0;
      i += 1;
    }
  }
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      sz[i].push_back([-1, 0]);
      {
        var j = 1;
        while ((j <= n))
        {
          var x: dynamic;
          read(x);
          sz[i].push_back([x, j]);
          j += 1;
        }
      }
      sort(sz[i].begin(), sz[i].end());
      {
        var j = 1;
        while ((j <= n))
        {
          inv[i][sz[i][j].second] = j;
          j += 1;
        }
      }
      i += 1;
    }
  }
  if ((n == 1))
  {
    write(1, "\n");
    return 0;
  }
  {
    var i = 1;
    while ((i <= n))
    {
      cnt = 0;
      kesz = 0;
      {
        var j = 1;
        while ((j <= n))
        {
          v[j] = 0;
          f[j] = 0;
          j += 1;
        }
      }
      dfs(i, i);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      dp[i][1] = 1;
      q.push([(1 - jo[i][1]), [i, 0]]);
      i += 1;
    }
  }
  while ((q.size() > 0))
  {
    var tav = (-q.top().first);
    var id = q.top().second.first;
    var pos = q.top().second.second;
    var db = jo[id][pos];
    q.pop();
    if ((!h[id]))
    {
      {
        var i = (db + 1);
        while ((i <= (db + tav)))
        {
          var x = sz[id][i].second;
          if ((!h[x]))
          {
            unio(id, x);
          }
          i += 1;
        }
      }
      dp[id][1] = 1;
      if (((tav + db) < n))
      {
        q.push([((tav + db) - jo[id][(pos + 2)]), [id, (pos + 1)]]);
      } else
      {
        {
          var i = 1;
          while ((i <= n))
          {
            write(dp[id][i], " ");
            i += 1;
          }
        }
        write("\n");
      }
    }
  }
  return 0;
}
