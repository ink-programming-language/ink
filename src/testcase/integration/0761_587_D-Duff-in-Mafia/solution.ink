// Translated from solution.cpp.

var N = (1e6 + 5);

var mod = (1e9 + 7);

class edge
{
  var u: dynamic;
  var v: dynamic;
  var t: dynamic;
  var c: dynamic;
  var id: dynamic;
}

var e = cpp_array(N);

var n: dynamic;

var m: dynamic;

var s = cpp_array(N);

func cmp(a: dynamic, b: dynamic)
{
  return (a.c < b.c);
}

class TwoSat
{
  var N: dynamic;
  var dfn: dynamic = cpp_array(N);
  var low: dynamic = cpp_array(N);
  var id: dynamic = cpp_array(N);
  var st: dynamic = cpp_array(N);
  var st: dynamic;
  var cpp_name: dynamic;
  var cc: dynamic;
  var g: dynamic = cpp_array(N);
  var mark: dynamic = cpp_array(N);
  var n: dynamic;
  func init(n: dynamic)
  {
      {
        var i = (((cpp_assign(n, "=", (n << 1)))) - 1);
        while ((i >= (0)))
        {
          g[i].clear();
          i -= 1;
        }
      }
    }
  func new_node()
  {
      {
        var i = (0);
        while ((i < (2)))
        {
          g[cpp_update(n, "++")].clear();
          i += 1;
        }
      }
      return ((n / 2) - 1);
    }
  func addedge(a: dynamic, va: dynamic, b: dynamic, vb: dynamic)
  {
      a = ((a << 1) | va);
      b = ((b << 1) | vb);
      g[a].push_back(b);
      g[(b ^ 1)].push_back((a ^ 1));
    }
  func add_set(a: dynamic, va: dynamic)
  {
      a = ((a << 1) | va);
      g[(a ^ 1)].push_back(a);
    }
  func add_then(a: dynamic, va: dynamic, b: dynamic, vb: dynamic)
  {
      addedge(a, va, b, (vb ^ 1));
    }
  func add_or(a: dynamic, va: dynamic, b: dynamic, vb: dynamic)
  {
      addedge(a, (va ^ 1), b, vb);
    }
  func add_xor(a: dynamic, va: dynamic, b: dynamic, vb: dynamic)
  {
      addedge(a, va, b, vb);
      addedge(b, vb, a, va);
    }
  func dfs(c: dynamic, g: dynamic)
  {
      dfn[c] = cpp_assign(low[c], "=", cpp_update(cc, "++"));
      st[cpp_update(st, "++")] = c;
      for (var t in g[c])
      {
        if ((!dfn[t]))
        {
          dfs(t, g);
          low[c] = min(low[c], low[t]);
        } else if ((!id[t]))
        {
          low[c] = min(low[c], dfn[t]);
        }
      }
      if ((low[c] == dfn[c]))
      {
        cpp_name += 1;
        while (true)
        {
          id[st[cpp_update(st, "--")]] = cpp_name;
          if (!(((st[st] != c))))
          {
            break;
          }
        }
      }
    }
  func find()
  {
      fill_n(dfn, n, cpp_assign(cc, "=", 0));
      fill_n(low, n, cpp_assign(st, "=", 0));
      fill_n(id, n, cpp_assign(cpp_name, "=", 0));
      {
        var i = (0);
        while ((i < (n)))
        {
          if ((!dfn[i]))
          {
            dfs(i, g);
          }
          i += 1;
        }
      }
      {
        var i = (0);
        while ((i < (n)))
        {
          id[i] -= 1;
          i += 1;
        }
      }
      return;
    }
  func solve()
  {
      find();
      {
        var i = 0;
        while ((i < n))
        {
          if ((id[i] == id[(i + 1)]))
          {
            return 0;
          }
          mark[(i >> 1)] = ((id[i] > id[(i + 1)]));
          i += 2;
        }
      }
      return 1;
    }
}

var ts: dynamic;

var p = 0;

func solve(u: dynamic)
{
  sort(s[u].begin(), s[u].end(), cmp);
  {
    var i = 0;
    var j: dynamic;
    while ((i < cpp_cast(s[u].size())))
    {
      j = i;
      while (((j < cpp_cast(s[u].size())) && (s[u][j].c == s[u][i].c)))
      {
        j += 1;
      }
      if (((j - i) >= 3))
      {
        write("No\n");
        exit(0);
      }
      if (((j - i) == 2))
      {
        var x = s[u][i].id;
        var y = s[u][(i + 1)].id;
        ts.add_or(x, 0, y, 0);
      }
      i = j;
    }
  }
  {
    var i = (0);
    while ((i < ((cpp_cast(s[u].size()) - 1))))
    {
      ts.addedge(((p + i) + 1), 0, (p + i), 0);
      i += 1;
    }
  }
  {
    var i = (0);
    while ((i < (cpp_cast(s[u].size()))))
    {
      var x = s[u][i].id;
      ts.addedge(x, 0, (p + i), 0);
      if ((i != (cpp_cast(s[u].size()) - 1)))
      {
        ts.addedge(x, 0, ((p + i) + 1), 1);
      }
      i += 1;
    }
  }
}

func link()
{
  p = m;
  {
    var i = (1);
    while ((i < ((n + 1))))
    {
      solve(i);
      p += cpp_cast(s[i].size());
      i += 1;
    }
  }
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  read(n, m);
  var L = 0;
  var R = 0;
  {
    var i = (1);
    while ((i < ((m + 1))))
    {
      read(e[i].u, e[i].v, e[i].c, e[i].t);
      e[i].id = (i - 1);
      R = max(R, e[i].t);
      s[e[i].u].push_back(e[i]);
      s[e[i].v].push_back(e[i]);
      i += 1;
    }
  }
  var ans = -1;
  while ((L <= R))
  {
    var mid = (((L + R)) / 2);
    ts.init((3 * m));
    link();
    {
      var i = (1);
      while ((i < ((m + 1))))
      {
        if ((e[i].t > mid))
        {
          ts.add_set(e[i].id, 1);
        }
        i += 1;
      }
    }
    if (ts.solve())
    {
      ans = mid;
      R = (mid - 1);
    } else
    {
      L = (mid + 1);
    }
  }
  if ((ans == -1))
  {
    write("No");
  } else
  {
    ts.init((3 * m));
    link();
    {
      var i = (1);
      while ((i < ((m + 1))))
      {
        if ((e[i].t > ans))
        {
          ts.add_set(e[i].id, 1);
        }
        i += 1;
      }
    }
    ts.solve();
    write("Yes\n");
    var task: dynamic;
    {
      var i = 0;
      while ((i < m))
      {
        if ((!ts.mark[i]))
        {
          task.push_back((i + 1));
        }
        i += 1;
      }
    }
    write(ans, " ", cpp_cast(task.size()), "\n");
    for (var v in task)
    {
      write(v, " ");
    }
  }
  return 0;
}
