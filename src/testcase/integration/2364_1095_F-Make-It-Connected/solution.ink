// Translated from solution.cpp.

var N = (2e5 + 5);

var edges: dynamic;

var par = cpp_array(N);

func find(x: dynamic)
{
  return if ((x == par[x])) x else cpp_assign(par[x], "=", find(par[x]));
}

func unite(x: dynamic, y: dynamic)
{
  x = find(x);
  y = find(y);
  if ((x != y))
  {
    par[x] = y;
  }
}

func main()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
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
      edges.push_back([w, u, v]);
      i += 1;
    }
  }
  var root = (min_element(a.begin(), a.end()) - a.begin());
  {
    var i = 0;
    while ((i < n))
    {
      if ((i != root))
      {
        edges.push_back([(a[i] + a[root]), root, i]);
      }
      i += 1;
    }
  }
  sort(edges.begin(), edges.end());
  {
    var i = 0;
    while ((i < n))
    {
      par[i] = i;
      i += 1;
    }
  }
  var ans = 0;
  for (var e in edges)
  {
    var w = e[0];
    var u = e[1];
    var v = e[2];
    if ((find(u) != find(v)))
    {
      ans += w;
      unite(u, v);
    }
  }
  write(ans);
  return 0;
}
