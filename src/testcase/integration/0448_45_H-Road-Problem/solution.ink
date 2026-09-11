// Translated from solution.cpp.

var eps: dynamic = 1e-7;

var inf: dynamic = 1000000010;

var INF: dynamic = 10000000000000010;

var mod: dynamic = 1000000007;

var MAXN: dynamic = 100010;

var LOG: dynamic = 20;

class DSU
{
  var par: dynamic = cpp_array(901);
  var vec: dynamic = cpp_array(901);
  func DSU() -> dynamic
  {
      {
        var i: dynamic = 1;
        while ((i <= 900))
        {
          par[i] = i;
          vec[i].push_back(i);
          i += 1;
        }
      }
    }
  func get(x: dynamic) -> dynamic
  {
      if ((par[x] == x))
      {
        return x;
      }
      return cpp_assign(par[x], "=", get(par[x]));
    }
  func join(x: dynamic, y: dynamic) -> dynamic
  {
      x = get(x);
      y = get(y);
      if ((x == y))
      {
        return;
      }
      if ((vec[x].size() < vec[y].size()))
      {
        swap(x, y);
      }
      for (var v: dynamic in vec[y])
      {
        vec[x].push_back(v);
      }
      par[y] = x;
      vec[y].clear();
    }
}

var dsu: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var u: dynamic = cpp_uninitialized();

var v: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var h: dynamic = cpp_array(901);

var connected: dynamic = cpp_array(901, 901);

var E: dynamic = cpp_array(MAXN);

var G1: dynamic = cpp_array(MAXN);

var G2: dynamic = cpp_array(MAXN);

var cutedge: dynamic = cpp_uninitialized();

var leaf: dynamic = cpp_uninitialized();

func bridge(node: dynamic, par: dynamic) -> dynamic
{
  var res: dynamic = cpp_assign(h[node], "=", cpp_assign(h[node], "=", (h[par] + 1)));
  for (var v: dynamic in G1[node])
  {
    if ((v != par))
    {
      if (h[v])
      {
        res = min(res, h[v]);
      } else
      {
        res = min(res, bridge(v, node));
      }
    }
  }
  if (((node != 1) && (res >= h[node])))
  {
    cutedge.push_back([par, node]);
  } else
  {
    dsu.join(par, node);
  }
  return res;
}

func dfs(node: dynamic, par: dynamic) -> dynamic
{
  if ((G2[node].size() == 1))
  {
    leaf.push_back(node);
    return;
  }
  for (var v: dynamic in G2[node])
  {
    if ((v != par))
    {
      dfs(v, node);
    }
  }
}

func connect(u: dynamic, v: dynamic) -> dynamic
{
  for (var x: dynamic in dsu.vec[u])
  {
    for (var y: dynamic in dsu.vec[v])
    {
      if ((!connected[x][y]))
      {
        write(x, cpp_char(" "), y, cpp_char("\n"));
        connected[x][y] = 1;
        return;
      }
    }
  }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  read(n, m);
  if ((n == 2))
  {
    return cpp_comma(((cout << -1) << cpp_char("\n")), 0);
  }
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      read(u, v);
      G1[u].push_back(v);
      G1[v].push_back(u);
      E[i] = [u, v];
      connected[u][v] = 1;
      i += 1;
    }
  }
  bridge(1, 1);
  for (var p: dynamic in cutedge)
  {
    var u: dynamic = dsu.get(p.first);
    var v: dynamic = dsu.get(p.second);
    G2[v].push_back(u);
    G2[u].push_back(v);
  }
  var root: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if (G2[i].size())
      {
        if ((G2[i].size() == 1))
        {
          leaf.push_back(i);
        } else
        {
          root = i;
        }
      }
      i += 1;
    }
  }
  if ((!leaf.size()))
  {
    return cpp_comma(((cout << 0) << cpp_char("\n")), 0);
  }
  write((((leaf.size() + 1)) / 2), cpp_char("\n"));
  if ((!root))
  {
    connect(leaf[0], leaf[1]);
    return 0;
  }
  leaf.clear();
  dfs(root, root);
  if ((leaf.size() & 1))
  {
    var v: dynamic = leaf.back();
    leaf.pop_back();
    connect(v, leaf.back());
  }
  {
    var i: dynamic = 0;
    while (((2 * i) < leaf.size()))
    {
      connect(leaf[i], leaf[(i + (leaf.size() / 2))]);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      write("(dsu.vec[i])", " : ");
      for (var SHIT: dynamic in (dsu.vec[i]))
      {
        write(SHIT, cpp_char(" "));
      }
      write("\n");
      i += 1;
    }
  }
  return 0;
}
