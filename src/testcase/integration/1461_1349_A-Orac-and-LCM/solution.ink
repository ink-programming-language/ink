// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var prim: dynamic = cpp_array(310000);

var ans: dynamic = 1;

var a: dynamic = cpp_array(310000);

var pan: dynamic = cpp_array(310000);

var Fmin: dynamic = cpp_array(310000);

var Smin: dynamic = cpp_array(310000);

var b: dynamic = cpp_uninitialized();

var maxx: dynamic = cpp_uninitialized();

var poi: dynamic = cpp_uninitialized();

var tot: dynamic = cpp_uninitialized();

var v: dynamic = cpp_array(310000);

func ksm(c: dynamic, d: dynamic) -> dynamic
{
  var zhi: dynamic = 1;
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

func oula() -> dynamic
{
  {
    var i: dynamic = 2;
    while ((i <= 300000))
    {
      if ((!v[i]))
      {
        prim[cpp_update(prim[0], "++")] = i;
        a[i] = i;
      }
      {
        var j: dynamic = 1;
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

func fen() -> dynamic
{
  var qian: dynamic = 0;
  var tot: dynamic = 0;
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

func main() -> dynamic
{
  memset(Fmin, 127, cpp_sizeof((Fmin)));
  scanf("%d", (&n));
  oula();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&b));
      fen();
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
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
