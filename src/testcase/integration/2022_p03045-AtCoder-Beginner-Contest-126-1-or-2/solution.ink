// Translated from solution.cpp.

var Maxv: dynamic = 100005;

var fa: dynamic = cpp_array(Maxv);

func find(x: dynamic) -> dynamic
{
  if ((fa[x] != x))
  {
    fa[x] = find(fa[x]);
  }
  return fa[x];
}

var visited: dynamic = cpp_array(Maxv);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var z: dynamic = cpp_uninitialized();
  read(n, m);
  {
    i = 1;
    while ((i <= n))
    {
      fa[i] = i;
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= m))
    {
      read(x, y, z);
      if ((find(x) != find(y)))
      {
        fa[find(y)] = find(x);
      }
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    i = 1;
    while ((i <= n))
    {
      if ((!visited[find(i)]))
      {
        ans += 1;
        visited[find(i)] = true;
      }
      i += 1;
    }
  }
  write(ans);
  return 0;
}
