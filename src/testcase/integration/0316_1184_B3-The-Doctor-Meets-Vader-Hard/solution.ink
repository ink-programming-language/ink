// Translated from solution.cpp.

var N: dynamic = (1e2 + 10);

var maxs: dynamic = (1e5 + 10);

var maxn: dynamic = (2e3 + 10);

var maxm: dynamic = (6e3 + 10);

var INF: dynamic = (1e14 + 10);

var INF_CAP: dynamic = INF;

class spaceship
{
  var x: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var f: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
}

var sp: dynamic = cpp_array(maxs);

class base
{
  var d: dynamic = cpp_uninitialized();
  var g: dynamic = cpp_uninitialized();
  func operator_less(b: dynamic) -> dynamic
  {
      return (d < b.d);
    }
}

var best_goal: dynamic = cpp_array(maxs);

var ba: dynamic = cpp_array(N);

var pre: dynamic = cpp_array(N);

var g: dynamic = cpp_array(maxs);

var used: dynamic = cpp_array(maxs);

var w: dynamic = cpp_array(N, N);

var id: dynamic = cpp_array(maxs);

class Dinic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var pos: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_array(maxn);
  var head: dynamic = cpp_array(maxn);
  var que: dynamic = cpp_array(maxn);
  var cpp_ptr: dynamic = cpp_array(maxn);
  var to: dynamic = cpp_array(maxm);
  var nxt: dynamic = cpp_array(maxm);
  var cap: dynamic = cpp_array(maxm);
  func init() -> dynamic
  {
      memset(head, -1, cpp_sizeof(head));
    }
  func addedge(a: dynamic, b: dynamic, c: dynamic) -> dynamic
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
  func bfs() -> dynamic
  {
      pos = 0;
      memset(d, -1, cpp_sizeof(d));
      que[cpp_update(pos, "++")] = s;
      d[s] = 0;
      {
        var i: dynamic = 0;
        while ((i < pos))
        {
          var x: dynamic = que[i];
          {
            var u: dynamic = head[x];
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
  func dfs(o: dynamic, mi: dynamic) -> dynamic
  {
      if (((o == t) || (mi == 0)))
      {
        return mi;
      }
      var res: dynamic = 0;
      {
        var x: dynamic = cpp_ptr[o];
        while ((~x))
        {
          if (((d[to[x]] == (d[o] + 1)) && cap[x]))
          {
            var tmp: dynamic = dfs(to[x], min(mi, cap[x]));
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
  func maxflow(s: dynamic, t: dynamic) -> dynamic
  {
      self->s = s;
      self->t = t;
      var res: dynamic = 0;
      while (bfs())
      {
        memcpy(cpp_ptr, head, cpp_sizeof(head));
        res += dfs(s, INF_CAP);
      }
      return res;
    }
}

var D: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  D.init();
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  scanf("%d%d", (&n), (&m));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      fill((w[i] + 1), ((w[i] + 1) + n), n);
      w[i][i] = 0;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      scanf("%d%d", (&a), (&b));
      w[b][a] = cpp_assign(w[a][b], "=", min(w[a][b], 1));
      i += 1;
    }
  }
  {
    var d: dynamic = 1;
    while ((d <= n))
    {
      {
        var i: dynamic = 1;
        while ((i <= n))
        {
          {
            var j: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= s))
    {
      scanf("%d%d%d%d", (&sp[i].x), (&sp[i].a), (&sp[i].f), (&sp[i].p));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < b))
    {
      var x: dynamic = cpp_uninitialized();
      var d: dynamic = cpp_uninitialized();
      var g: dynamic = cpp_uninitialized();
      scanf("%d%d%d", (&x), (&d), (&g));
      ba[x].push_back([d, g]);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if (ba[i].size())
      {
        var sz: dynamic = cpp_cast(ba[i].size());
        sort(ba[i].begin(), ba[i].end());
        pre[i].resize(sz);
        pre[i][0] = ba[i][0].g;
        {
          var j: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= s))
    {
      best_goal[i] = (-INF_CAP);
      var x: dynamic = sp[i].x;
      {
        var j: dynamic = 1;
        while ((j <= n))
        {
          if ((w[x][j] <= sp[i].f))
          {
            var pos: dynamic = (upper_bound(ba[j].begin(), ba[j].end(), [sp[i].a, 0]) - ba[j].begin());
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
    var i: dynamic = 0;
    while ((i < k))
    {
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      scanf("%d%d", (&a), (&b));
      g[a].push_back(b);
      used[a] = cpp_assign(used[b], "=", 1);
      i += 1;
    }
  }
  var ans: dynamic = 0;
  var cnt: dynamic = 0;
  {
    var i: dynamic = 1;
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
  var st: dynamic = 0;
  var ed: dynamic = (cnt + 1);
  cnt = 0;
  {
    var i: dynamic = 1;
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
        for (var u: dynamic in g[i])
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
