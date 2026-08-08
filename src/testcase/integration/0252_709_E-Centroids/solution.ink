// Translated from solution.cpp.

var rng = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());

var INF = 1e18;

var PI = acos(-1);

var tam = 1000100;

var MOD = (1e9 + 7);

var cmplog = 29;

var hijos = cpp_array(tam);

var g = cpp_array(tam);

var pcen: dynamic;

var n: dynamic;

func dfs(u: dynamic, pa: dynamic)
{
  hijos[u] = 1;
  var maxx = 0;
  for (var w in g[u])
  {
    if ((w == pa))
    {
      continue;
    }
    dfs(w, u);
    hijos[u] += hijos[w];
    maxx = max(maxx, hijos[w]);
  }
  maxx = max(maxx, (n - hijos[u]));
  pcen = min(pcen, pair(maxx, u));
}

var queries = cpp_array(tam);

var sdown = cpp_array(tam);

func dfs2(u: dynamic, pa: dynamic, idx: dynamic)
{
  hijos[u] = 1;
  for (var w in g[u])
  {
    if ((w == pa))
    {
      continue;
    }
    dfs2(w, u, idx);
    hijos[u] += hijos[w];
  }
  var pup = (n - hijos[u]);
  queries[idx].push_back([pup, u]);
  sdown[idx].insert(hijos[u]);
}

func mejor(s: dynamic, precio: dynamic)
{
  var it = s.upper_bound((precio / 2));
  var ans = precio;
  if ((it != s.end()))
  {
    ans = min(ans, max((precio - (*it)), (*it)));
  }
  if ((it != s.begin()))
  {
    it -= 1;
    ans = min(ans, max((precio - (*it)), (*it)));
  }
  return ans;
}

var fans = cpp_array(tam);

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  read(n);
  var iz: dynamic;
  var der: dynamic;
  {
    var i = 0;
    while ((i < (n - 1)))
    {
      read(iz, der);
      g[iz].push_back(der);
      g[der].push_back(iz);
      i += 1;
    }
  }
  pcen = pair(n, n);
  dfs(1, 1);
  var ucen = pcen.second;
  var siz: dynamic;
  var sder: dynamic;
  {
    var i = 0;
    while ((i < g[ucen].size()))
    {
      var w = g[ucen][i];
      dfs2(w, ucen, i);
      i += 1;
    }
  }
  {
    var idx = 0;
    while ((idx < g[ucen].size()))
    {
      for (var xx in sdown[idx])
      {
        sder.insert(xx);
      }
      idx += 1;
    }
  }
  fans[ucen] = 1;
  {
    var idx = 0;
    while ((idx < g[ucen].size()))
    {
      for (var xx in sdown[idx])
      {
        sder.erase(sder.find(xx));
      }
      for (var par in queries[idx])
      {
        var u = par.second;
        var pup = par.first;
        var bst = min(mejor(siz, pup), mejor(sder, pup));
        var otro = (n - hijos[g[ucen][idx]]);
        bst = min(bst, max((pup - otro), otro));
        if ((bst <= (n / 2)))
        {
          fans[u] = 1;
        } else
        {
          fans[u] = 0;
        }
      }
      for (var xx in sdown[idx])
      {
        siz.insert(xx);
      }
      idx += 1;
    }
  }
  {
    var i = 1;
    while ((i < (n + 1)))
    {
      write(fans[i], cpp_char(" "));
      i += 1;
    }
  }
}
