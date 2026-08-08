// Translated from solution.cpp.

var N = (1e5 + 7);

var M = 110;

class node
{
  var u: dynamic;
  var v: dynamic;
  var id: dynamic;
  var s: dynamic;
}

var n: dynamic;

var Q: dynamic;

var tot: dynamic;

var cnt: dynamic;

var m: dynamic;

var hd = cpp_array(N);

var fa = cpp_array(20, N);

var dep = cpp_array(N);

var ans = cpp_array(N);

var v = cpp_array((N << 1));

var nxt = cpp_array((N << 1));

var rt = cpp_array(N);

var sum = cpp_array((N * 50));

var lc = cpp_array((N * 50));

var rc = cpp_array((N * 50));

var val = cpp_array(M, N);

var nv = cpp_array(N);

var dv = cpp_array(M);

var rv = cpp_array(M);

var c = cpp_array((N << 1));

var str: dynamic;

var q = cpp_array(M);

var h: dynamic;

func add(x: dynamic, y: dynamic, z: dynamic)
{
  v[cpp_update(tot, "++")] = y;
  nxt[tot] = hd[x];
  c[tot] = z;
  hd[x] = tot;
}

func dfs(u: dynamic, f: dynamic)
{
  dep[u] = (dep[f] + 1);
  fa[u][0] = f;
  {
    var i = 2;
    var j = f;
    while ((i <= 100))
    {
      val[u][i] = ((val[u][(i - 1)] * 233) + val[j][1]);
      i += 1;
      j = fa[j][0];
    }
  }
  {
    var i = 1;
    while (((i < 20) && fa[u][(i - 1)]))
    {
      fa[u][i] = fa[fa[u][(i - 1)]][(i - 1)];
      i += 1;
    }
  }
  {
    var i = hd[u];
    while (i)
    {
      if ((v[i] != f))
      {
        val[v[i]][1] = (c[i] - cpp_char("a"));
        dfs(v[i], u);
      }
      i = nxt[i];
    }
  }
}

func lca(x: dynamic, y: dynamic)
{
  if ((dep[x] < dep[y]))
  {
    swap(x, y);
  }
  {
    var i = 19;
    while ((~i))
    {
      if ((dep[fa[x][i]] >= dep[y]))
      {
        x = fa[x][i];
      }
      i -= 1;
    }
  }
  if ((x == y))
  {
    return x;
  }
  {
    var i = 19;
    while ((~i))
    {
      if ((fa[x][i] != fa[y][i]))
      {
        x = fa[x][i];
        y = fa[y][i];
      }
      i -= 1;
    }
  }
  return fa[x][0];
}

func up(x: dynamic, k: dynamic)
{
  {
    var i = 19;
    while ((~i))
    {
      if ((((k >> i)) & 1))
      {
        x = fa[x][i];
      }
      i -= 1;
    }
  }
  return x;
}

func update(x: dynamic, y: dynamic, k: dynamic, l: dynamic, r: dynamic)
{
  x = cpp_update(cnt, "++");
  lc[x] = lc[y];
  rc[x] = rc[y];
  sum[x] = (sum[y] + 1);
  if ((l == r))
  {
    return;
  }
  var mid = (((l + r)) >> 1);
  if ((k <= mid))
  {
    update(lc[x], lc[y], k, l, mid);
  } else
  {
    update(rc[x], rc[y], k, (mid + 1), r);
  }
}

func query(k: dynamic, x: dynamic, y: dynamic, l: dynamic, r: dynamic)
{
  if ((l == r))
  {
    return (sum[x] - sum[y]);
  }
  var mid = (((l + r)) >> 1);
  if ((k <= mid))
  {
    return query(k, lc[x], lc[y], l, mid);
  }
  return query(k, rc[x], rc[y], (mid + 1), r);
}

func solve(pos: dynamic)
{
  var ret = 0;
  var z = lca(pos.u, pos.v);
  var l = pos.s.size();
  dv[0] = 0;
  {
    var i = 0;
    while ((i < l))
    {
      dv[(i + 1)] = (((dv[i] * 233) + pos.s[i]) - cpp_char("a"));
      i += 1;
    }
  }
  rv[0] = 0;
  {
    var i = 0;
    while ((i < l))
    {
      rv[(i + 1)] = (((rv[i] * 233) + pos.s[((l - i) - 1)]) - cpp_char("a"));
      i += 1;
    }
  }
  if (((dep[pos.u] - dep[z]) >= l))
  {
    var v = ((lower_bound(h.begin(), h.end(), dv[l]) - h.begin()) + 1);
    if (((v <= m) && (dv[l] == h[(v - 1)])))
    {
      ret += query(v, rt[pos.u], rt[up(pos.u, (((dep[pos.u] - dep[z]) - l) + 1))], 1, m);
    }
  }
  if (((dep[pos.v] - dep[z]) >= l))
  {
    var v = ((lower_bound(h.begin(), h.end(), rv[l]) - h.begin()) + 1);
    if (((v <= m) && (rv[l] == h[(v - 1)])))
    {
      ret += query(v, rt[pos.v], rt[up(pos.v, (((dep[pos.v] - dep[z]) - l) + 1))], 1, m);
    }
  }
  {
    var i = up(pos.u, max(0, (((dep[pos.u] - dep[z]) - l) + 1)));
    while ((i != z))
    {
      if ((((((dep[pos.v] - dep[z]) - l) + dep[i]) - dep[z]) >= 0))
      {
        var j = up(pos.v, ((((dep[pos.v] - dep[z]) - l) + dep[i]) - dep[z]));
        if (((val[i][(dep[i] - dep[z])] == dv[(dep[i] - dep[z])]) && (val[j][(dep[j] - dep[z])] == rv[(dep[j] - dep[z])])))
        {
          ret += 1;
        }
      }
      i = fa[i][0];
    }
  }
  return ret;
}

func modify(u: dynamic)
{
  update(rt[u], rt[fa[u][0]], nv[u], 1, m);
  {
    var i = hd[u];
    while (i)
    {
      if ((v[i] != fa[u][0]))
      {
        modify(v[i]);
      }
      i = nxt[i];
    }
  }
}

func main()
{
  scanf("%d", (&n));
  var z: dynamic;
  {
    var i = 1;
    var x: dynamic;
    var y: dynamic;
    while ((i < n))
    {
      scanf("%d%d %c", (&x), (&y), (&z));
      add(x, y, z);
      add(y, x, z);
      i += 1;
    }
  }
  dfs(1, 0);
  scanf("%d", (&Q));
  {
    var i = 1;
    var x: dynamic;
    var y: dynamic;
    while ((i <= Q))
    {
      read(x, y, str);
      q[str.size()].push_back([x, y, i, str]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= 100))
    {
      if (q[i].size())
      {
        h.clear();
        cnt = 1;
        {
          var j = 1;
          while ((j <= n))
          {
            h.push_back(val[j][i]);
            j += 1;
          }
        }
        sort(h.begin(), h.end());
        h.erase(unique(h.begin(), h.end()), h.end());
        {
          var j = 1;
          while ((j <= n))
          {
            nv[j] = ((lower_bound(h.begin(), h.end(), val[j][i]) - h.begin()) + 1);
            j += 1;
          }
        }
        m = h.size();
        modify(1);
        {
          var j = 0;
          while ((j < q[i].size()))
          {
            ans[q[i][j].id] = solve(q[i][j]);
            j += 1;
          }
        }
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= Q))
    {
      printf("%d\n", ans[i]);
      i += 1;
    }
  }
}
