// Translated from solution.cpp.

var N = 100005;

var n: dynamic;

var q: dynamic;

var f = cpp_array(N);

var c: dynamic;

var m: dynamic;

var v1: dynamic;

var v2: dynamic;

func find(x: dynamic)
{
  return if ((x == f[x])) x else cpp_assign(f[x], "=", find(f[x]));
}

func link(x: dynamic, y: dynamic)
{
  f[find(x)] = find(y);
}

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  read(n, m, q);
  {
    var i = 0;
    while ((i < n))
    {
      f[i] = i;
      i += 1;
    }
  }
  {
    var i = 1;
    var x: dynamic;
    var y: dynamic;
    var z: dynamic;
    while ((i <= q))
    {
      read(x, y, z);
      (if (z) v2 else v1).emplace_back(x, y);
      i += 1;
    }
  }
  for (var e in v1)
  {
    link(e.first, e.second);
  }
  for (var e in v2)
  {
    if ((find(e.first) == find(e.second)))
    {
      return cpp_comma((cout << "No\n"), 0);
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      c += (f[i] == i);
      i += 1;
    }
  }
  write((if ((((if (v2.size()) max(3, c) else (c - 1)) <= (m - ((n - c)))) && ((m - ((n - c))) <= ((cpp_cast(c) * ((c - 1))) / 2)))) "Yes" else "No"), cpp_char("\n"));
  return 0;
}
