// Translated from solution.cpp.

var N: dynamic = 1000;

var M: dynamic = ((N * ((N - 1))) / 2);

var dsu: dynamic = cpp_array((N * 2));

func find(i: dynamic) -> dynamic
{
  return  ((dsu[i] < 0)) ? i : (cpp_assign(dsu[i], "=", find(dsu[i])));
}

func join(i: dynamic, j: dynamic) -> dynamic
{
  i = find(i);
  j = find(j);
  if ((i == j))
  {
    return false;
  }
  if ((dsu[i] > dsu[j]))
  {
    dsu[i] = j;
  } else
  {
    if ((dsu[i] == dsu[j]))
    {
      dsu[i] -= 1;
    }
    dsu[j] = i;
  }
  return true;
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  scanf("%d%d%d", (&n), (&m), (&q));
  var ii: dynamic = cpp_array(M);
  var jj: dynamic = cpp_array(M);
  var ww: dynamic = cpp_array(M);
  var hh: dynamic = cpp_array(M);
  {
    var h: dynamic = 0;
    while ((h < m))
    {
      var i: dynamic = cpp_uninitialized();
      var j: dynamic = cpp_uninitialized();
      var w: dynamic = cpp_uninitialized();
      scanf("%d%d%d", (&i), (&j), (&w));
      i -= 1;
      j -= 1;
      ii[h] = i;
      jj[h] = j;
      ww[h] = w;
      hh[h] = h;
      h += 1;
    }
  }
  sort(hh, (hh + m), __cpp_lambda_1);
  while ((cpp_update(q, "--") > 0))
  {
    var l: dynamic = cpp_uninitialized();
    var r: dynamic = cpp_uninitialized();
    scanf("%d%d", (&l), (&r));
    l -= 1;
    r -= 1;
    fill_n(dsu, (n * 2), -1);
    var w: dynamic = -1;
    {
      var h: dynamic = 0;
      while ((h < m))
      {
        var h: dynamic = hh[h];
        if (((l <= h) && (h <= r)))
        {
          var i: dynamic = ii[h];
          var j: dynamic = jj[h];
          var i0: dynamic = (i << 1);
          var i1: dynamic = (i0 | 1);
          var j0: dynamic = (j << 1);
          var j1: dynamic = (j0 | 1);
          if ((join(i0, j1) && (!join(i1, j0))))
          {
            w = ww[h];
            break;
          }
        }
        h += 1;
      }
    }
    printf("%d\n", w);
  }
}

func __cpp_lambda_1(a: dynamic, b: dynamic) -> dynamic
{
  return (ww[a] > ww[b]);
}
