// Translated from solution.cpp.

var MOD: dynamic = 998244353;

var p: dynamic = cpp_uninitialized();

var gr: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

func dsu_get(v: dynamic) -> dynamic
{
  return  (((v == p[v]))) ? v : (cpp_assign(p[v], "=", dsu_get(p[v])));
}

func dsu_unite(a: dynamic, b: dynamic) -> dynamic
{
  a = dsu_get(a);
  b = dsu_get(b);
  if ((rand() & 1))
  {
    swap(a, b);
  }
  if ((a != b))
  {
    p[a] = b;
  }
}

var marked: dynamic = cpp_uninitialized();

var used: dynamic = cpp_uninitialized();

func dfs(v: dynamic) -> dynamic
{
  marked[v] = 1;
  {
    var i: dynamic = 0;
    while ((i < gr[v].size()))
    {
      if ((!marked[gr[v][i].first]))
      {
        dfs(gr[v][i].first);
        if ((marked[gr[v][i].first] == 2))
        {
          used[gr[v][i].second] = true;
          marked[v] = 2;
        }
      }
      i += 1;
    }
  }
  if (x[v])
  {
    marked[v] = 2;
  }
}

func main() -> dynamic
{
  srand(time(null));
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, m, k);
  gr.resize(n);
  x.resize(n, 0);
  var xf: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < k))
    {
      var ff: dynamic = cpp_uninitialized();
      read(ff);
      ff -= 1;
      x[ff] = 1;
      xf = ff;
      i += 1;
    }
  }
  var g: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      var w: dynamic = cpp_uninitialized();
      read(u, v, w);
      u -= 1;
      v -= 1;
      g.push_back(make_pair(w, make_pair(v, u)));
      i += 1;
    }
  }
  var cost: dynamic = 0;
  var res: dynamic = cpp_uninitialized();
  sort(g.begin(), g.end());
  p.resize(n, 0);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      p[i] = i;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var a: dynamic = g[i].second.first;
      var b: dynamic = g[i].second.second;
      var l: dynamic = g[i].first;
      if ((dsu_get(a) != dsu_get(b)))
      {
        cost += l;
        res.push_back(make_pair(g[i].first, g[i].second));
        dsu_unite(a, b);
      }
      i += 1;
    }
  }
  sort(res.begin(), res.end());
  marked.resize(n, false);
  used.resize(res.size(), false);
  {
    var i: dynamic = 0;
    while ((i < res.size()))
    {
      gr[res[i].second.first].push_back(make_pair(res[i].second.second, i));
      gr[res[i].second.second].push_back(make_pair(res[i].second.first, i));
      i += 1;
    }
  }
  dfs(xf);
  var ans: dynamic = 0;
  {
    var i: dynamic = (res.size() - 1);
    while ((i >= 0))
    {
      if (used[i])
      {
        ans = res[i].first;
        break;
      }
      i -= 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < k))
    {
      write(ans, " ");
      i += 1;
    }
  }
}
