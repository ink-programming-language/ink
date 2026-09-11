// Translated from solution.cpp.

func sqr(x: dynamic) -> dynamic
{
  return ((x * x));
}

func dist(a: dynamic, b: dynamic) -> dynamic
{
  return (sqr((a.first - b.first)) + sqr((a.second - b.second)));
}

func func_cpp(i: dynamic) -> dynamic
{
  if (((i < 2) || (i & 1)))
  {
    return 1;
  }
  return -1;
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  var a: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%lld %lld %lld", (&x), (&y), (&r));
      a.push_back(make_pair(r, make_pair(x, y)));
      i += 1;
    }
  }
  var deg: dynamic = cpp_array(n);
  memset(deg, 0, cpp_sizeof(deg));
  sort(a.begin(), a.end());
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = (i + 1);
        while ((j < n))
        {
          if (((dist(a[i].second, a[j].second) <= sqr(a[j].first)) && (i != j)))
          {
            deg[i] += 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var answer: dynamic = 0.0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      answer += (((cpp_cast(acos(-1)) * a[i].first) * a[i].first) * func_cpp(deg[i]));
      i += 1;
    }
  }
  printf("%.20lf\n", answer);
  return 0;
}
