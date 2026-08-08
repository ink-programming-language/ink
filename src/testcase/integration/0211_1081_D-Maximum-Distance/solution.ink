// Translated from solution.cpp.

var MOD = 998244353;

var p: dynamic;

var gr: dynamic;

var x: dynamic;

func dsu_get(v: dynamic)
{
  return if (((v == p[v]))) v else (cpp_assign(p[v], "=", dsu_get(p[v])));
}

func dsu_unite(a: dynamic, b: dynamic)
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

var marked: dynamic;

var used: dynamic;

func dfs(v: dynamic)
{
  marked[v] = 1;
  {
    var i = 0;
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

func main()
{
  srand(time(null));
  var n: dynamic;
  var m: dynamic;
  var k: dynamic;
  read(n, m, k);
  gr.resize(n);
  x.resize(n, 0);
  var xf = 0;
  {
    var i = 0;
    while ((i < k))
    {
      var ff: dynamic;
      read(ff);
      ff -= 1;
      x[ff] = 1;
      xf = ff;
      i += 1;
    }
  }
  var g: dynamic;
  {
    var i = 0;
    while ((i < m))
    {
      var u: dynamic;
      var v: dynamic;
      var w: dynamic;
      read(u, v, w);
      u -= 1;
      v -= 1;
      g.push_back(make_pair(w, make_pair(v, u)));
      i += 1;
    }
  }
  var cost = 0;
  var res: dynamic;
  sort(g.begin(), g.end());
  p.resize(n, 0);
  {
    var i = 0;
    while ((i < n))
    {
      p[i] = i;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      var a = g[i].second.first;
      var b = g[i].second.second;
      var l = g[i].first;
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
    var i = 0;
    while ((i < res.size()))
    {
      gr[res[i].second.first].push_back(make_pair(res[i].second.second, i));
      gr[res[i].second.second].push_back(make_pair(res[i].second.first, i));
      i += 1;
    }
  }
  dfs(xf);
  var ans = 0;
  {
    var i = (res.size() - 1);
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
    var i = 0;
    while ((i < k))
    {
      write(ans, " ");
      i += 1;
    }
  }
}
