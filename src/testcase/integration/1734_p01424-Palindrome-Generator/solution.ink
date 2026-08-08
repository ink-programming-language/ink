// Translated from solution.cpp.

func min(a: dynamic, b: dynamic)
{
  return min(a, cpp_cast(b));
}

func min(a: dynamic, b: dynamic)
{
  return min(cpp_cast(a), b);
}

class edge
{
  var to: dynamic;
  var cost: dynamic;
  func edge()
  {
    }
  func edge(a: dynamic, b: dynamic)
  {
      this->to = cpp_construct(a);
      this->cost = cpp_construct(b);
    }
}

var G = cpp_array(210210);

var rG = cpp_array(210210);

var V: dynamic;

var N: dynamic;

var strs = cpp_array(100);

func encode(i: dynamic, j: dynamic, a: dynamic, l: dynamic)
{
  if ((l == 0))
  {
    return ((i * 2100) + (j * 21));
  }
  if ((a == 0))
  {
    return (((i * 2100) + (j * 21)) + l);
  }
  if ((a == 1))
  {
    return ((((i * 2100) + (j * 21)) + 10) + l);
  }
}

var prv = cpp_array(100);

var nxt = cpp_array(100);

func isPalin(str: dynamic)
{
  {
    var i = 0;
    while ((i < str.size()))
    {
      if ((str[i] != str[((str.size() - i) - 1)]))
      {
        return false;
      }
      i += 1;
    }
  }
  return true;
}

func match_cpp(str1: dynamic, str2: dynamic)
{
  if ((str1.size() != str2.size()))
  {
    fprintf(stderr, "invalid call for match\n");
    fprintf(stderr, "%s %s\n", str1.c_str(), str2.c_str());
    exit(0);
  }
  {
    var i = 0;
    while ((i < str1.size()))
    {
      if ((str1[i] != str2[((str2.size() - i) - 1)]))
      {
        return false;
      }
      i += 1;
    }
  }
  return true;
}

func getGraph()
{
  V = (((((2100 * ((N - 1))) + (21 * ((N - 1)))) + 21)) + 2);
  {
    var le = 0;
    while ((le < N))
    {
      {
        var ri = 0;
        while ((ri < N))
        {
          var v = encode(le, ri, 0, 0);
          {
            var i = 0;
            while ((i < nxt[ri].size()))
            {
              var id = nxt[ri][i];
              var s = strs[nxt[ri][i]].size();
              var nv = encode(le, id, 1, s);
              G[v].push_back(edge(nv, s));
              i += 1;
            }
          }
          {
            var i = 0;
            while ((i < prv[le].size()))
            {
              var id = prv[le][i];
              var s = strs[id].size();
              var nv = encode(id, ri, 0, s);
              G[v].push_back(edge(nv, s));
              i += 1;
            }
          }
          {
            var l = 1;
            while ((l <= strs[le].size()))
            {
              var v = encode(le, ri, 0, l);
              {
                var i = 0;
                while ((i < nxt[ri].size()))
                {
                  var id = nxt[ri][i];
                  var m = min(l, strs[id].size());
                  var s = strs[id].size();
                  var str1 = strs[le].substr((l - m), m);
                  var str2 = strs[id].substr(0, m);
                  var flg = match_cpp(str1, str2);
                  if ((!flg))
                  {
                    i += 1;
                    continue;
                  }
                  if ((l > m))
                  {
                    var nv = encode(le, id, 0, (l - m));
                    G[v].push_back(edge(nv, s));
                  } else
                  {
                    var nv = encode(le, id, 1, (s - m));
                    G[v].push_back(edge(nv, s));
                  }
                  i += 1;
                }
              }
              l += 1;
            }
          }
          {
            var l = 1;
            while ((l <= strs[ri].size()))
            {
              var v = encode(le, ri, 1, l);
              {
                var i = 0;
                while ((i < prv[le].size()))
                {
                  var id = prv[le][i];
                  var m = min(l, strs[id].size());
                  var str1 = strs[id].substr((strs[id].size() - m), m);
                  var str2 = strs[ri].substr((strs[ri].size() - l), m);
                  var s = strs[id].size();
                  var flg = match_cpp(str1, str2);
                  if ((!flg))
                  {
                    i += 1;
                    continue;
                  }
                  if ((l > m))
                  {
                    var nv = encode(id, ri, 1, (l - m));
                    G[v].push_back(edge(nv, s));
                  } else
                  {
                    var nv = encode(id, ri, 0, (s - m));
                    G[v].push_back(edge(nv, s));
                  }
                  i += 1;
                }
              }
              l += 1;
            }
          }
          ri += 1;
        }
      }
      le += 1;
    }
  }
  {
    var i = 0;
    while ((i < N))
    {
      if (isPalin(strs[i]))
      {
        var v = encode(i, i, 0, 0);
        G[(V - 2)].push_back(edge(v, strs[i].size()));
      }
      {
        var l = 1;
        while ((l <= strs[i].size()))
        {
          var str: dynamic;
          if ((l == strs[i].size()))
          {
            str = "";
          } else
          {
            str = strs[i].substr(l, 1000);
          }
          if ((!isPalin(str)))
          {
            l += 1;
            continue;
          }
          var v = encode(i, i, 0, l);
          G[(V - 2)].push_back(edge(v, strs[i].size()));
          l += 1;
        }
      }
      {
        var l = 1;
        while ((l <= strs[i].size()))
        {
          var str = strs[i].substr(0, (strs[i].size() - l));
          if ((!isPalin(str)))
          {
            l += 1;
            continue;
          }
          var v = encode(i, i, 1, l);
          G[(V - 2)].push_back(edge(v, strs[i].size()));
          l += 1;
        }
      }
      i += 1;
    }
  }
  {
    var le = 0;
    while ((le < N))
    {
      {
        var ri = 0;
        while ((ri < N))
        {
          var v = encode(le, ri, 0, 0);
          G[v].push_back(edge((V - 1), 0));
          ri += 1;
        }
      }
      le += 1;
    }
  }
  {
    var v = 0;
    while ((v < V))
    {
      {
        var j = 0;
        while ((j < G[v].size()))
        {
          rG[G[v][j].to].push_back(edge(v, G[v][j].cost));
          j += 1;
        }
      }
      v += 1;
    }
  }
}

var from_s = cpp_array(220220);

var from_t = cpp_array(220220);

var que: dynamic;

func reachable(s: dynamic, res: dynamic, rev: dynamic)
{
  while ((!que.empty()))
  {
    que.pop();
  }
  que.push(s);
  {
    var i = 0;
    while ((i < V))
    {
      res[i] = false;
      i += 1;
    }
  }
  res[s] = true;
  while ((!que.empty()))
  {
    var v = que.front();
    que.pop();
    if ((!rev))
    {
      {
        var i = 0;
        while ((i < G[v].size()))
        {
          var nxt = G[v][i].to;
          if (res[nxt])
          {
            i += 1;
            continue;
          }
          res[nxt] = true;
          que.push(nxt);
          i += 1;
        }
      }
    } else
    {
      {
        var i = 0;
        while ((i < rG[v].size()))
        {
          var nxt = rG[v][i].to;
          if (res[nxt])
          {
            i += 1;
            continue;
          }
          res[nxt] = true;
          que.push(nxt);
          i += 1;
        }
      }
    }
  }
}

var is_valid = cpp_array(220220);

var cmp = cpp_array(220220);

var used = cpp_array(220220);

var vs: dynamic;

func dfs(v: dynamic)
{
  used[v] = true;
  {
    var i = 0;
    while ((i < G[v].size()))
    {
      var nxt = G[v][i].to;
      if ((!is_valid[nxt]))
      {
        i += 1;
        continue;
      }
      if (used[nxt])
      {
        i += 1;
        continue;
      }
      dfs(nxt);
      i += 1;
    }
  }
  vs.push_back(v);
}

func rdfs(v: dynamic, k: dynamic)
{
  used[v] = true;
  cmp[v] = k;
  {
    var i = 0;
    while ((i < rG[v].size()))
    {
      var nxt = rG[v][i].to;
      if ((!is_valid[nxt]))
      {
        i += 1;
        continue;
      }
      if (used[nxt])
      {
        i += 1;
        continue;
      }
      rdfs(nxt, k);
      i += 1;
    }
  }
}

func scc()
{
  {
    var i = 0;
    while ((i < V))
    {
      used[i] = false;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < V))
    {
      if ((!is_valid[i]))
      {
        i += 1;
        continue;
      }
      if (used[i])
      {
        i += 1;
        continue;
      }
      dfs(i);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < V))
    {
      used[i] = false;
      i += 1;
    }
  }
  var k = 0;
  {
    var i = (cpp_cast(vs.size()) - 1);
    while ((i >= 0))
    {
      var v = vs[i];
      if ((!is_valid[v]))
      {
        i -= 1;
        continue;
      }
      if (used[v])
      {
        i -= 1;
        continue;
      }
      rdfs(v, cpp_update(k, "++"));
      i -= 1;
    }
  }
}

var cnt = cpp_array(220220);

func checkInf()
{
  {
    var i = 0;
    while ((i < V))
    {
      if ((!is_valid[i]))
      {
        i += 1;
        continue;
      }
      cnt[cmp[i]] += 1;
      if ((cnt[cmp[i]] >= 2))
      {
        return true;
      }
      i += 1;
    }
  }
  return false;
}

var dp = cpp_array(220220);

var prvs = cpp_array(220220);

func rec(v: dynamic)
{
  if ((dp[v] != -1))
  {
    return;
  }
  var M = 0;
  var p = -1;
  {
    var i = 0;
    while ((i < rG[v].size()))
    {
      var prv = rG[v][i].to;
      if ((!is_valid[prv]))
      {
        i += 1;
        continue;
      }
      var c = rG[v][i].cost;
      rec(prv);
      if ((M < (dp[prv] + c)))
      {
        p = prv;
      }
      M = max(M, (dp[prv] + c));
      i += 1;
    }
  }
  prvs[v] = p;
  dp[v] = M;
}

func solve()
{
  getGraph();
  reachable((V - 2), from_s, false);
  reachable((V - 1), from_t, true);
  if ((from_s[(V - 1)] == false))
  {
    return 0;
  }
  {
    var i = 0;
    while ((i < V))
    {
      is_valid[i] = (from_s[i] & from_t[i]);
      i += 1;
    }
  }
  scc();
  var is_inf = checkInf();
  if (is_inf)
  {
    return -1;
  }
  {
    var i = 0;
    while ((i < V))
    {
      dp[i] = -1;
      i += 1;
    }
  }
  dp[(V - 2)] = 0;
  rec((V - 1));
  var res = dp[(V - 1)];
  return res;
}

func input()
{
  var M: dynamic;
  scanf("%d%d", (&N), (&M));
  var ch = cpp_array(20);
  {
    var i = 0;
    while ((i < N))
    {
      scanf("%s", ch);
      strs[i] = string_cpp(ch);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < M))
    {
      var u: dynamic;
      var v: dynamic;
      scanf("%d%d", (&u), (&v));
      u -= 1;
      v -= 1;
      nxt[u].push_back(v);
      prv[v].push_back(u);
      i += 1;
    }
  }
}

func main()
{
  input();
  var ans = solve();
  printf("%d\n", ans);
  return 0;
}
