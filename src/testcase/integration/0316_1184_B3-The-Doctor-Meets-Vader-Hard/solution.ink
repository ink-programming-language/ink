// Translated from solution.cpp.

var N = (1e2 + 10);

var maxs = (1e5 + 10);

var maxn = (2e3 + 10);

var maxm = (6e3 + 10);

var INF = (1e14 + 10);

var INF_CAP = INF;

class spaceship
{
  var x: dynamic;
  var a: dynamic;
  var f: dynamic;
  var p: dynamic;
}

var sp = cpp_array(maxs);

class base
{
  var d: dynamic;
  var g: dynamic;
  func operator_less(b: dynamic)
  {
      return (d < b.d);
    }
}

var best_goal = cpp_array(maxs);

var ba = cpp_array(N);

var pre = cpp_array(N);

var g = cpp_array(maxs);

var used = cpp_array(maxs);

var w = cpp_array(N, N);

var id = cpp_array(maxs);

class Dinic
{
  var n: dynamic;
  var m: dynamic;
  var s: dynamic;
  var t: dynamic;
  var pos: dynamic;
  var d: dynamic = cpp_array(maxn);
  var head: dynamic = cpp_array(maxn);
  var que: dynamic = cpp_array(maxn);
  var ptr: dynamic = cpp_array(maxn);
  var to: dynamic = cpp_array(maxm);
  var nxt: dynamic = cpp_array(maxm);
  var cap: dynamic = cpp_array(maxm);
  func init()
  {
      memset(head, -1, cpp_sizeof(head));
    }
  func addedge(a: dynamic, b: dynamic, c: dynamic)
  {
      cap[m] = c;
      to[m] = b;
      nxt[m] = head[a];
      head[a] = cpp_update(m, "++");
      cap[m] = 0;
      to[m] = a;
      nxt[m] = head[b];
      head[b] = cpp_update(m, "++");
    }
  func bfs()
  {
      pos = 0;
      memset(d, -1, cpp_sizeof(d));
      que[cpp_update(pos, "++")] = s;
      d[s] = 0;
      {
        var i = 0;
        while ((i < pos))
        {
          var x = que[i];
          {
            var u = head[x];
            while ((~u))
            {
              if (((d[to[u]] == -1) && cap[u]))
              {
                d[to[u]] = (d[x] + 1);
                que[cpp_update(pos, "++")] = to[u];
                if ((d[t] != -1))
                {
                  return true;
                }
              }
              u = nxt[u];
            }
          }
          i += 1;
        }
      }
      return (d[t] != -1);
    }
  func dfs(o: dynamic, mi: dynamic)
  {
      if (((o == t) || (mi == 0)))
      {
        return mi;
      }
      var res = 0;
      {
        var x = ptr[o];
        while ((~x))
        {
          if (((d[to[x]] == (d[o] + 1)) && cap[x]))
          {
            var tmp = dfs(to[x], min(mi, cap[x]));
            cap[x] -= tmp;
            cap[(x ^ 1)] += tmp;
            if ((tmp > 0))
            {
              return tmp;
            }
          }
          x = nxt[x];
        }
      }
      return res;
    }
  func maxflow(s: dynamic, t: dynamic)
  {
      this->s = s;
      this->t = t;
      var res = 0;
      while (bfs())
      {
        memcpy(ptr, head, cpp_sizeof(head));
        res += dfs(s, INF_CAP);
      }
      return res;
    }
}

var D: dynamic;

func main()
{
  D.init();
  var n: dynamic;
  var m: dynamic;
  var s: dynamic;
  var b: dynamic;
  var k: dynamic;
  scanf("%d%d", (&n), (&m));
  {
    var i = 1;
    while ((i <= n))
    {
      fill((w[i] + 1), ((w[i] + 1) + n), n);
      w[i][i] = 0;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      var a: dynamic;
      var b: dynamic;
      scanf("%d%d", (&a), (&b));
      w[b][a] = cpp_assign(w[a][b], "=", min(w[a][b], 1));
      i += 1;
    }
  }
  {
    var d = 1;
    while ((d <= n))
    {
      {
        var i = 1;
        while ((i <= n))
        {
          {
            var j = 1;
            while ((j <= n))
            {
              w[i][j] = min(w[i][j], (w[i][d] + w[d][j]));
              j += 1;
            }
          }
          i += 1;
        }
      }
      d += 1;
    }
  }
  scanf("%d%d%d", (&s), (&b), (&k));
  {
    var i = 1;
    while ((i <= s))
    {
      scanf("%d%d%d%d", (&sp[i].x), (&sp[i].a), (&sp[i].f), (&sp[i].p));
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < b))
    {
      var x: dynamic;
      var d: dynamic;
      var g: dynamic;
      scanf("%d%d%d", (&x), (&d), (&g));
      ba[x].push_back([d, g]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if (ba[i].size())
      {
        var sz = cpp_cast(ba[i].size());
        sort(ba[i].begin(), ba[i].end());
        pre[i].resize(sz);
        pre[i][0] = ba[i][0].g;
        {
          var j = 1;
          while ((j < sz))
          {
            pre[i][j] = max(pre[i][(j - 1)], ba[i][j].g);
            j += 1;
          }
        }
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= s))
    {
      best_goal[i] = (-INF_CAP);
      var x = sp[i].x;
      {
        var j = 1;
        while ((j <= n))
        {
          if ((w[x][j] <= sp[i].f))
          {
            var pos = (upper_bound(ba[j].begin(), ba[j].end(), [sp[i].a, 0]) - ba[j].begin());
            pos -= 1;
            if ((pos >= 0))
            {
              best_goal[i] = max(best_goal[i], (cpp_cast(pre[j][pos]) - sp[i].p));
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < k))
    {
      var a: dynamic;
      var b: dynamic;
      scanf("%d%d", (&a), (&b));
      g[a].push_back(b);
      used[a] = cpp_assign(used[b], "=", 1);
      i += 1;
    }
  }
  var ans = 0;
  var cnt = 0;
  {
    var i = 1;
    while ((i <= s))
    {
      if ((!used[i]))
      {
        if ((best_goal[i] > 0))
        {
          ans += best_goal[i];
        }
      } else
      {
        id[i] = cpp_update(cnt, "++");
      }
      i += 1;
    }
  }
  var st = 0;
  var ed = (cnt + 1);
  cnt = 0;
  {
    var i = 1;
    while ((i <= s))
    {
      if (used[i])
      {
        if ((best_goal[i] >= 0))
        {
          ans += best_goal[i];
          D.addedge(st, id[i], best_goal[i]);
        } else
        {
          D.addedge(id[i], ed, (-best_goal[i]));
        }
        for (var u in g[i])
        {
          D.addedge(id[i], id[u], INF_CAP);
        }
      }
      i += 1;
    }
  }
  ans -= D.maxflow(st, ed);
  printf("%lld\n", ans);
  return 0;
}
