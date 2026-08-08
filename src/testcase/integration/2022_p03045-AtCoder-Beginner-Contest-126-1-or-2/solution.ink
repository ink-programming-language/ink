// Translated from solution.cpp.

var Maxv = 100005;

var fa = cpp_array(Maxv);

func find(x: dynamic)
{
  if ((fa[x] != x))
  {
    fa[x] = find(fa[x]);
  }
  return fa[x];
}

var visited = cpp_array(Maxv);

func main()
{
  var n: dynamic;
  var m: dynamic;
  var i: dynamic;
  var j: dynamic;
  var x: dynamic;
  var y: dynamic;
  var z: dynamic;
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
  var ans = 0;
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
