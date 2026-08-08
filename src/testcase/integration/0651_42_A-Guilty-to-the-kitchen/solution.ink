// Translated from solution.cpp.

var v: dynamic;

func comparar(i: dynamic, j: dynamic)
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

func main()
{
  var n: dynamic;
  var V: dynamic;
  scanf("%d %d", (&n), (&V));
  v.clear();
  v.resize(n);
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d", (&v[i].first));
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d", (&v[i].second));
      i += 1;
    }
  }
  var a: dynamic;
  var b: dynamic;
  var min: dynamic;
  a = v[0].second;
  b = (a / v[0].first);
  min = b;
  {
    var i = 1;
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
  var vRes = 0;
  var vMax = V;
  {
    var i = 0;
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
