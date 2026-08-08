// Translated from solution.cpp.

var N = ((500 * 1000) + 10);

var t: dynamic;

var n: dynamic;

var m: dynamic;

var par = cpp_array(N);

var deg = cpp_array(N);

var st: dynamic;

var en: dynamic;

var h = cpp_array(N);

var mn: dynamic;

var comp = cpp_array(N);

var sv = cpp_array(N, 2);

var adj = cpp_array(N);

var vis = cpp_array(N);

var ans = cpp_array(N);

func gclear()
{
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < 2))
        {
          sv[j][i] = -1;
          j += 1;
        }
      }
      adj[i].clear();
      comp[i] = cpp_assign(ans[i], "=", cpp_assign(vis[i], "=", 0));
      par[i] = -1;
      i += 1;
    }
  }
  mn = (1 << 30);
  st = cpp_assign(en, "=", -1);
}

func print()
{
  var res = n;
  {
    var v = 0;
    while ((v < n))
    {
      res -= ans[v];
      v += 1;
    }
  }
  if ((!res))
  {
    return cpp_comma((cout << "No\n"), void_cpp());
  }
  write("Yes\n", res, cpp_char("\n"));
  {
    var v = 0;
    while ((v < n))
    {
      if ((!ans[v]))
      {
        write((v + 1), cpp_char(" "));
      }
      v += 1;
    }
  }
  write(cpp_char("\n"));
}

func dfs(v: dynamic)
{
  vis[v] = true;
  for (var u in adj[v])
  {
    if ((deg[u] ^ 2))
    {
      continue;
    }
    if ((!vis[u]))
    {
      h[u] = (h[v] + 1);
      par[u] = v;
      dfs(u);
    } else if (((par[v] ^ u) && (abs((h[v] - h[u])) < mn)))
    {
      st = u;
      en = v;
      if ((h[st] > h[en]))
      {
        swap(st, en);
      }
      mn = (h[en] - h[st]);
    }
  }
}

func bfs(x: dynamic)
{
  var q: dynamic;
  fill(vis, (vis + n), 0);
  q.push(x);
  vis[x] = true;
  while ((!q.empty()))
  {
    var v = q.front();
    if (((deg[v] == 1) && (x ^ v)))
    {
      en = v;
      break;
    }
    q.pop();
    for (var u in adj[v])
    {
      if ((!vis[u]))
      {
        vis[u] = true;
        par[u] = v;
        q.push(u);
      }
    }
  }
  while ((en ^ -1))
  {
    ans[en] = true;
    en = par[en];
  }
  print();
}

func bf(v: dynamic, x: dynamic)
{
  comp[v] = x;
  for (var u in adj[v])
  {
    if ((!comp[u]))
    {
      h[u] = (h[v] + 1);
      par[u] = v;
      bf(u, x);
    }
  }
}

func chck(s: dynamic, e: dynamic)
{
  while ((e ^ -1))
  {
    if ((s == e))
    {
      return 1;
    }
    e = par[e];
  }
  return 0;
}

func solve()
{
  var lst = -1;
  var cnt = 0;
  {
    var v = 0;
    while ((v < n))
    {
      deg[v] = (cpp_cast(adj[v].size()) % 3);
      if ((!deg[v]))
      {
        ans[v] = true;
        return print();
      } else if ((deg[v] == 1))
      {
        lst = v;
        cnt += 1;
      }
      v += 1;
    }
  }
  if ((cnt > 1))
  {
    return bfs(lst);
  }
  {
    var v = 0;
    while ((v < n))
    {
      if (((deg[v] == 2) && (!vis[v])))
      {
        dfs(v);
        if (((st ^ -1) && (en ^ -1)))
        {
          if (chck(st, en))
          {
            while ((en ^ st))
            {
              ans[en] = true;
              en = par[en];
            }
            ans[st] = true;
            return print();
          } else
          {
            st = -1;
            en = -1;
            mn = (1 << 30);
          }
        }
      }
      v += 1;
    }
  }
  if (cnt)
  {
    fill(par, (par + n), -1);
    cnt = 0;
    comp[lst] = -1;
    var num: dynamic;
    for (var v in adj[lst])
    {
      if ((!comp[v]))
      {
        sv[0][cpp_update(cnt, "++")] = v;
        bf(v, cnt);
      } else
      {
        num.insert(comp[v]);
        if (((((sv[1][comp[v]] ^ -1) && (h[sv[1][comp[v]]] > h[v]))) || (sv[1][comp[v]] == -1)))
        {
          sv[1][comp[v]] = v;
        }
      }
    }
    cnt = 0;
    if ((num.size() > 1))
    {
      ans[lst] = true;
      for (var x in num)
      {
        cnt += 1;
        if ((cnt > 2))
        {
          break;
        }
        while ((sv[0][x] ^ sv[1][x]))
        {
          ans[sv[1][x]] = true;
          sv[1][x] = par[sv[1][x]];
        }
        ans[sv[0][x]] = true;
      }
      return print();
    }
  }
  write("No\n");
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  read(t);
  while (cpp_update(t, "--"))
  {
    read(n, m);
    gclear();
    {
      var i = 0;
      var u: dynamic;
      var v: dynamic;
      while ((i < m))
      {
        read(u, v);
        adj[cpp_update(u, "--")].push_back(cpp_update(v, "--"));
        adj[v].push_back(u);
        i += 1;
      }
    }
    solve();
  }
}
