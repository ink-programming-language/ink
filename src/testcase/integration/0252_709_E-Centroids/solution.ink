// Translated from solution.cpp.

var rng: dynamic = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());

var INF: dynamic = 1e18;

var PI: dynamic = acos(-1);

var tam: dynamic = 1000100;

var MOD: dynamic = (1e9 + 7);

var cmplog: dynamic = 29;

var hijos: dynamic = cpp_array(tam);

var g: dynamic = cpp_array(tam);

var pcen: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

func dfs(u: dynamic, pa: dynamic) -> dynamic
{
  hijos[u] = 1;
  var maxx: dynamic = 0;
  for (var w: dynamic in g[u])
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

var queries: dynamic = cpp_array(tam);

var sdown: dynamic = cpp_array(tam);

func dfs2(u: dynamic, pa: dynamic, idx: dynamic) -> dynamic
{
  hijos[u] = 1;
  for (var w: dynamic in g[u])
  {
    if ((w == pa))
    {
      continue;
    }
    dfs2(w, u, idx);
    hijos[u] += hijos[w];
  }
  var pup: dynamic = (n - hijos[u]);
  queries[idx].push_back([pup, u]);
  sdown[idx].insert(hijos[u]);
}

func mejor(s: dynamic, precio: dynamic) -> dynamic
{
  var it: dynamic = s.upper_bound((precio / 2));
  var ans: dynamic = precio;
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

var fans: dynamic = cpp_array(tam);

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  read(n);
  var iz: dynamic = cpp_uninitialized();
  var der: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
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
  var ucen: dynamic = pcen.second;
  var siz: dynamic = cpp_uninitialized();
  var sder: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < g[ucen].size()))
    {
      var w: dynamic = g[ucen][i];
      dfs2(w, ucen, i);
      i += 1;
    }
  }
  {
    var idx: dynamic = 0;
    while ((idx < g[ucen].size()))
    {
      for (var xx: dynamic in sdown[idx])
      {
        sder.insert(xx);
      }
      idx += 1;
    }
  }
  fans[ucen] = 1;
  {
    var idx: dynamic = 0;
    while ((idx < g[ucen].size()))
    {
      for (var xx: dynamic in sdown[idx])
      {
        sder.erase(sder.find(xx));
      }
      for (var par: dynamic in queries[idx])
      {
        var u: dynamic = par.second;
        var pup: dynamic = par.first;
        var bst: dynamic = min(mejor(siz, pup), mejor(sder, pup));
        var otro: dynamic = (n - hijos[g[ucen][idx]]);
        bst = min(bst, max((pup - otro), otro));
        if ((bst <= (n / 2)))
        {
          fans[u] = 1;
        } else
        {
          fans[u] = 0;
        }
      }
      for (var xx: dynamic in sdown[idx])
      {
        siz.insert(xx);
      }
      idx += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < (n + 1)))
    {
      write(fans[i], cpp_char(" "));
      i += 1;
    }
  }
}
