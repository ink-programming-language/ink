// Translated from solution.cpp.

var maxn: dynamic = (100000 + 10);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(maxn);

class Edge
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
  func operator_less(rhs: dynamic) -> dynamic
  {
      return (w < rhs.w);
    }
}

var e: dynamic = cpp_array(maxn);

var sum: dynamic = cpp_array(maxn);

var fa: dynamic = cpp_array(maxn);

func find(x: dynamic) -> dynamic
{
  return  ((fa[x] == x)) ? x : cpp_assign(fa[x], "=", find(fa[x]));
}

func main(argc: dynamic, argv: dynamic) -> dynamic
{
  read(n, m);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
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
    var i: dynamic = 1;
    while ((i <= n))
    {
      fa[i] = i;
      sum[i] = 1;
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var fx: dynamic = find(e[i].x);
      var fy: dynamic = find(e[i].y);
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
