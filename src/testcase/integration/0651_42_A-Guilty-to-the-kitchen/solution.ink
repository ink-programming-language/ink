// Translated from solution.cpp.

var v: dynamic = cpp_uninitialized();

func comparar(i: dynamic, j: dynamic) -> dynamic
{
  if (((j - i) > 1e-07))
  {
    return -1;
  }
  if (((i - j) > 1e-07))
  {
    return 1;
  }
  return 0;
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var V: dynamic = cpp_uninitialized();
  scanf("%d %d", (&n), (&V));
  v.clear();
  v.resize(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d", (&v[i].first));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d", (&v[i].second));
      i += 1;
    }
  }
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var min: dynamic = cpp_uninitialized();
  a = v[0].second;
  b = (a / v[0].first);
  min = b;
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      a = v[i].second;
      b = (a / v[i].first);
      if ((comparar(b, min) < 0))
      {
        min = b;
      }
      i += 1;
    }
  }
  var vRes: dynamic = 0;
  var vMax: dynamic = V;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      vRes += ((min * v[i].first));
      i += 1;
    }
  }
  if ((comparar(vRes, vMax) < 0))
  {
    printf("%.5lf\n", vRes);
  } else
  {
    printf("%.5lf\n", vMax);
  }
}
