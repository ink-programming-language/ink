// Translated from solution.cpp.

var maxn = (100000 + 10);

var n: dynamic;

var m: dynamic;

var a = cpp_array(maxn);

class Edge
{
  var x: dynamic;
  var y: dynamic;
  var w: dynamic;
  func operator_less(rhs: dynamic)
  {
      return (w < rhs.w);
    }
}

var e = cpp_array(maxn);

var sum = cpp_array(maxn);

var fa = cpp_array(maxn);

func find(x: dynamic)
{
  return if ((fa[x] == x)) x else cpp_assign(fa[x], "=", find(fa[x]));
}

func main(argc: dynamic, argv: dynamic)
{
  read(n, m);
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      read(e[i].x, e[i].y);
      e[i].w = min(a[e[i].x], a[e[i].y]);
      i += 1;
    }
  }
  sort(e, (e + m));
  reverse(e, (e + m));
  {
    var i = 1;
    while ((i <= n))
    {
      fa[i] = i;
      sum[i] = 1;
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 0;
    while ((i < m))
    {
      var fx = find(e[i].x);
      var fy = find(e[i].y);
      if ((fx != fy))
      {
        ans += (((1 * sum[fx]) * sum[fy]) * e[i].w);
        fa[fx] = fy;
        sum[fy] += sum[fx];
      }
      i += 1;
    }
  }
  ans <<= 1;
  cout.precision(8);
  write((cpp_cast(ans) / (((1 * n) * ((n - 1))))), "\n");
  return 0;
}
