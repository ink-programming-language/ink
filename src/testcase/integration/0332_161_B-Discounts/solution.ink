// Translated from solution.cpp.

var ans = cpp_array(1010, 1010);

var al = cpp_array(1010);

class node
{
  var id: dynamic;
  var v: dynamic;
}

var den = cpp_array(1010);

var qian = cpp_array(1010);

var dl = 0;

var ql = 0;

func cmp(a: dynamic, b: dynamic)
{
  return (a.v > b.v);
}

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  var m: dynamic;
  scanf("%d", (&m));
  var cost = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      var x: dynamic;
      scanf("%d", (&x));
      var y: dynamic;
      scanf("%d", (&y));
      if ((y == 1))
      {
        den[dl].id = i;
        den[cpp_update(dl, "++")].v = x;
      } else
      {
        qian[ql].id = i;
        qian[cpp_update(ql, "++")].v = x;
      }
      i += 1;
    }
  }
  sort(den, (den + dl), cmp);
  var i: dynamic;
  {
    i = 0;
    while (((i < (m - 1)) && (i < dl)))
    {
      ans[i][0] = den[i].id;
      al[i] = 1;
      cost += (den[i].v / 2.0);
      i += 1;
    }
  }
  if ((i < (m - 1)))
  {
    var j: dynamic;
    {
      j = 0;
      while ((i < (m - 1)))
      {
        ans[i][0] = qian[j].id;
        al[i] = 1;
        cost += qian[j].v;
        i += 1;
        j += 1;
      }
    }
    {
      while ((j < ql))
      {
        ans[(m - 1)][cpp_update(al[(m - 1)], "++")] = qian[j].id;
        cost += qian[j].v;
        j += 1;
      }
    }
  } else
  {
    var vmin = 0x3FFFFFFF;
    {
      var j = 0;
      while ((i < dl))
      {
        ans[(m - 1)][j] = den[i].id;
        al[(m - 1)] += 1;
        cost += den[i].v;
        if ((den[i].v < vmin))
        {
          vmin = den[i].v;
        }
        i += 1;
        j += 1;
      }
    }
    {
      var j = 0;
      while ((j < ql))
      {
        ans[(m - 1)][cpp_update(al[(m - 1)], "++")] = qian[j].id;
        cost += qian[j].v;
        if (((vmin != 0x3FFFFFFF) && (qian[j].v < vmin)))
        {
          vmin = qian[j].v;
        }
        j += 1;
      }
    }
    if ((vmin != 0x3FFFFFFF))
    {
      cost -= (vmin / 2.0);
    }
  }
  printf("%.1lf\n", cost);
  {
    var i = 0;
    while ((i < m))
    {
      printf("%d", al[i]);
      {
        var j = 0;
        while ((j < al[i]))
        {
          printf(" %d", ans[i][j]);
          j += 1;
        }
      }
      printf("\n");
      i += 1;
    }
  }
  return 0;
}
