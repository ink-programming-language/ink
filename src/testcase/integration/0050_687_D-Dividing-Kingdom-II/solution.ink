// Translated from solution.cpp.

var N = 1000;

var M = ((N * ((N - 1))) / 2);

var dsu = cpp_array((N * 2));

func find(i: dynamic)
{
  return if ((dsu[i] < 0)) i else (cpp_assign(dsu[i], "=", find(dsu[i])));
}

func join(i: dynamic, j: dynamic)
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

func main()
{
  var n: dynamic;
  var m: dynamic;
  var q: dynamic;
  scanf("%d%d%d", (&n), (&m), (&q));
  var ii = cpp_array(M);
  var jj = cpp_array(M);
  var ww = cpp_array(M);
  var hh = cpp_array(M);
  {
    var h = 0;
    while ((h < m))
    {
      var i: dynamic;
      var j: dynamic;
      var w: dynamic;
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
    var l: dynamic;
    var r: dynamic;
    scanf("%d%d", (&l), (&r));
    l -= 1;
    r -= 1;
    fill_n(dsu, (n * 2), -1);
    var w = -1;
    {
      var h = 0;
      while ((h < m))
      {
        var h = hh[h];
        if (((l <= h) && (h <= r)))
        {
          var i = ii[h];
          var j = jj[h];
          var i0 = (i << 1);
          var i1 = (i0 | 1);
          var j0 = (j << 1);
          var j1 = (j0 | 1);
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

func __cpp_lambda_1(a: dynamic, b: dynamic)
{
  return (ww[a] > ww[b]);
}
