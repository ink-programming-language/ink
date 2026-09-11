// Translated from solution.cpp.

var N: dynamic = (1e5 + 5);

var LG: dynamic = 17;

var anc: dynamic = cpp_array(LG, N);

var sum: dynamic = cpp_array(LG, N);

var depth: dynamic = cpp_array(N);

var chld: dynamic = cpp_array(N);

var parent: dynamic = cpp_array(N);

var type_cpp: dynamic = cpp_array(N);

var n: dynamic = cpp_uninitialized();

func build(now: dynamic, par: dynamic, val: dynamic) -> dynamic
{
  anc[now][0] = par;
  sum[now][0] = val;
  depth[now] = (depth[par] + 1);
  {
    var i: dynamic = 1;
    while ((((1 << i)) <= depth[now]))
    {
      var par: dynamic = anc[now][(i - 1)];
      var cur_sum: dynamic = (sum[now][(i - 1)] + sum[par][(i - 1)]);
      anc[now][i] = anc[par][(i - 1)];
      sum[now][i] = cur_sum;
      i += 1;
    }
  }
}

func dfs(now: dynamic) -> dynamic
{
  for (var nex: dynamic in chld[now])
  {
    build(nex.first, now, nex.second);
    dfs(nex.first);
  }
}

func read() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d %d", (parent + i), (type_cpp + i));
      i += 1;
    }
  }
}

func init() -> dynamic
{
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((parent[i] != -1))
      {
        chld[parent[i]].push_back([i, type_cpp[i]]);
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((parent[i] == -1))
      {
        dfs(i);
      }
      i += 1;
    }
  }
}

func get_lca(u: dynamic, v: dynamic) -> dynamic
{
  if ((depth[v] > depth[u]))
  {
    swap(u, v);
  }
  var diff: dynamic = (depth[u] - depth[v]);
  {
    var i: dynamic = 0;
    while (diff)
    {
      if ((diff & ((1 << i))))
      {
        u = anc[u][i];
        diff -= ((1 << i));
      }
      i += 1;
    }
  }
  if ((u == v))
  {
    return u;
  }
  {
    var i: dynamic = (LG - 1);
    while ((i >= 0))
    {
      if (((depth[u] >= ((1 << i))) && (anc[u][i] != anc[v][i])))
      {
        u = anc[u][i];
        v = anc[v][i];
      }
      i -= 1;
    }
  }
  if (((depth[u] == 0) || (anc[u][0] != anc[v][0])))
  {
    return -1;
  }
  return anc[u][0];
}

func get_sum(now: dynamic, up: dynamic) -> dynamic
{
  var ret: dynamic = 0;
  {
    var i: dynamic = (LG - 1);
    while ((i >= 0))
    {
      if ((up & ((1 << i))))
      {
        ret += sum[now][i];
        now = anc[now][i];
        up -= ((1 << i));
      }
      i -= 1;
    }
  }
  return ret;
}

func is_special(u: dynamic, v: dynamic) -> dynamic
{
  if ((u == v))
  {
    return false;
  }
  var lca: dynamic = get_lca(u, v);
  if ((lca != u))
  {
    return false;
  }
  var up: dynamic = (depth[v] - depth[lca]);
  if ((get_sum(v, up) != 0))
  {
    return false;
  }
  return true;
}

func is_part(u: dynamic, v: dynamic) -> dynamic
{
  if ((u == v))
  {
    return false;
  }
  var lca: dynamic = get_lca(u, v);
  if ((lca == -1))
  {
    return false;
  }
  if ((lca == v))
  {
    return false;
  }
  var up: dynamic = (depth[v] - depth[lca]);
  if ((get_sum(v, up) != up))
  {
    return false;
  }
  up = (depth[u] - depth[lca]);
  if ((get_sum(u, up) != 0))
  {
    return false;
  }
  return true;
}

func work() -> dynamic
{
  var q: dynamic = cpp_uninitialized();
  scanf("%d", (&q));
  {
    var i: dynamic = 0;
    while ((i < q))
    {
      var t: dynamic = cpp_uninitialized();
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      scanf("%d %d %d", (&t), (&u), (&v));
      var ret: dynamic = cpp_uninitialized();
      if ((t == 1))
      {
        ret = is_special(u, v);
      } else
      {
        ret = is_part(u, v);
      }
      printf("%s\n",  (ret) ? "YES" : "NO");
      i += 1;
    }
  }
}

func main() -> dynamic
{
  read();
  init();
  work();
  return 0;
}
