// Translated from solution.cpp.

var N: dynamic = 100005;

var n: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var f: dynamic = cpp_array(N);

var c: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var v1: dynamic = cpp_uninitialized();

var v2: dynamic = cpp_uninitialized();

func find(x: dynamic) -> dynamic
{
  return  ((x == f[x])) ? x : cpp_assign(f[x], "=", find(f[x]));
}

func link(x: dynamic, y: dynamic) -> dynamic
{
  f[find(x)] = find(y);
}

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  read(n, m, q);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      f[i] = i;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    var x: dynamic = cpp_uninitialized();
    var y: dynamic = cpp_uninitialized();
    var z: dynamic = cpp_uninitialized();
    while ((i <= q))
    {
      read(x, y, z);
      ( (z) ? v2 : v1).emplace_back(x, y);
      i += 1;
    }
  }
  for (var e: dynamic in v1)
  {
    link(e.first, e.second);
  }
  for (var e: dynamic in v2)
  {
    if ((find(e.first) == find(e.second)))
    {
      return cpp_comma((cout << "No\n"), 0);
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      c += (f[i] == i);
      i += 1;
    }
  }
  write(( (((( (v2.size()) ? max(3, c) : (c - 1)) <= (m - ((n - c)))) && ((m - ((n - c))) <= ((cpp_cast(c) * ((c - 1))) / 2)))) ? "Yes" : "No"), cpp_char("\n"));
  return 0;
}
