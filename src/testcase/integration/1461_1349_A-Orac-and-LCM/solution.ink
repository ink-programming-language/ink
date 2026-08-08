// Translated from solution.cpp.

var n: dynamic;

var prim = cpp_array(310000);

var ans = 1;

var a = cpp_array(310000);

var pan = cpp_array(310000);

var Fmin = cpp_array(310000);

var Smin = cpp_array(310000);

var b: dynamic;

var maxx: dynamic;

var poi: dynamic;

var tot: dynamic;

var v = cpp_array(310000);

func ksm(c: dynamic, d: dynamic)
{
  var zhi = 1;
  while (d)
  {
    if ((d % 2))
    {
      zhi *= c;
    }
    c *= c;
    d /= 2;
  }
  return zhi;
}

func oula()
{
  {
    var i = 2;
    while ((i <= 300000))
    {
      if ((!v[i]))
      {
        prim[cpp_update(prim[0], "++")] = i;
        a[i] = i;
      }
      {
        var j = 1;
        while (cpp_comma((j <= prim[0]), ((prim[j] * i) <= 200000)))
        {
          v[(i * prim[j])] = 1;
          a[(i * prim[j])] = prim[j];
          if (((i % prim[j]) == 0))
          {
            break;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
}

func fen()
{
  var qian = 0;
  var tot = 0;
  while ((b > 1))
  {
    tot += 1;
    if ((a[b] != qian))
    {
      pan[qian] += 1;
      if ((tot <= Fmin[qian]))
      {
        Smin[qian] = Fmin[qian];
        Fmin[qian] = tot;
      } else if ((tot < Smin[qian]))
      {
        Smin[qian] = tot;
      }
      tot = 0;
      qian = a[b];
    }
    b /= a[b];
  }
  tot += 1;
  if ((a[b] != qian))
  {
    pan[qian] += 1;
    if ((tot <= Fmin[qian]))
    {
      Smin[qian] = Fmin[qian];
      Fmin[qian] = tot;
    } else if ((tot < Smin[qian]))
    {
      Smin[qian] = tot;
    }
    tot = 0;
    qian = a[b];
  }
}

func main()
{
  memset(Fmin, 127, cpp_sizeof((Fmin)));
  scanf("%d", (&n));
  oula();
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&b));
      fen();
      i += 1;
    }
  }
  {
    var i = 1;
    while ((prim[i] <= 200000))
    {
      if ((pan[prim[i]] < (n - 1)))
      {
        i += 1;
        continue;
      }
      if ((pan[prim[i]] == (n - 1)))
      {
        ans *= ksm(prim[i], Fmin[prim[i]]);
      } else if ((pan[prim[i]] == n))
      {
        if (((Smin[prim[i]] == 0) || (Smin[prim[i]] == 2139062143)))
        {
          i += 1;
          continue;
        }
        ans *= ksm(prim[i], Smin[prim[i]]);
      }
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
