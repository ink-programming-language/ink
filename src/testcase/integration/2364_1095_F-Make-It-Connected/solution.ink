// Translated from solution.cpp.

var N: dynamic = (2e5 + 5);

var edges: dynamic = cpp_uninitialized();

var par: dynamic = cpp_array(N);

func find(x: dynamic) -> dynamic
{
  return  ((x == par[x])) ? x : cpp_assign(par[x], "=", find(par[x]));
}

func unite(x: dynamic, y: dynamic) -> dynamic
{
  x = find(x);
  y = find(y);
  if ((x != y))
  {
    par[x] = y;
  }
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
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
      edges.push_back([w, u, v]);
      i += 1;
    }
  }
  var root: dynamic = (min_element(a.begin(), a.end()) - a.begin());
  {
    var i: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < n))
    {
      par[i] = i;
      i += 1;
    }
  }
  var ans: dynamic = 0;
  for (var e: dynamic in edges)
  {
    var w: dynamic = e[0];
    var u: dynamic = e[1];
    var v: dynamic = e[2];
    if ((find(u) != find(v)))
    {
      ans += w;
      unite(u, v);
    }
  }
  write(ans);
  return 0;
}
