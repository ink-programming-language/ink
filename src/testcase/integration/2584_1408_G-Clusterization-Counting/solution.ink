// Translated from solution.cpp.

var c: dynamic = 1502;

var mod: dynamic = 998244353;

var dp: dynamic = cpp_array(c, c);

var el: dynamic = cpp_array(c, c);

var inv: dynamic = cpp_array(c, c);

var f: dynamic = cpp_array(c);

var db: dynamic = cpp_array(c);

var ki: dynamic = cpp_array(c);

var n: dynamic = cpp_uninitialized();

var cnt: dynamic = 0;

var v: dynamic = cpp_array(c);

var h: dynamic = cpp_array(c);

var kesz: dynamic = cpp_uninitialized();

var sz: dynamic = cpp_array(c);

var jo: dynamic = cpp_array(c);

var q: dynamic = cpp_uninitialized();

func dfs(a: dynamic, b: dynamic) -> dynamic
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
      var i: dynamic = 0;
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
    var x: dynamic = sz[a][f[a]].second;
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
        var i: dynamic = 1;
        while ((i <= cnt))
        {
          var x: dynamic = sz[a][i].second;
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

func unio(a: dynamic, b: dynamic) -> dynamic
{
  h[b] = 1;
  var sb: dynamic = 0;
  var sa: dynamic = 0;
  var st: dynamic = 0;
  {
    var i: dynamic = 1;
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
    var i: dynamic = (sa + sb);
    while ((i >= 1))
    {
      dp[a][i] = 0;
      {
        var j: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i < st))
    {
      dp[a][i] = 0;
      i += 1;
    }
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      sz[i].push_back([-1, 0]);
      {
        var j: dynamic = 1;
        while ((j <= n))
        {
          var x: dynamic = cpp_uninitialized();
          read(x);
          sz[i].push_back([x, j]);
          j += 1;
        }
      }
      sort(sz[i].begin(), sz[i].end());
      {
        var j: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= n))
    {
      cnt = 0;
      kesz = 0;
      {
        var j: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= n))
    {
      dp[i][1] = 1;
      q.push([(1 - jo[i][1]), [i, 0]]);
      i += 1;
    }
  }
  while ((q.size() > 0))
  {
    var tav: dynamic = (-q.top().first);
    var id: dynamic = q.top().second.first;
    var pos: dynamic = q.top().second.second;
    var db: dynamic = jo[id][pos];
    q.pop();
    if ((!h[id]))
    {
      {
        var i: dynamic = (db + 1);
        while ((i <= (db + tav)))
        {
          var x: dynamic = sz[id][i].second;
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
          var i: dynamic = 1;
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
