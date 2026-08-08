// Translated from solution.cpp.

var n: dynamic;

var q: dynamic;

var nn: dynamic;

var arra: dynamic;

var arrw: dynamic;

var suml: dynamic;

var sumr: dynamic;

var sum: dynamic;

var mod = (1E9 + 7);

var mx: dynamic;

var sm: dynamic;

var TEST = 0;

func read()
{
  read(n, q);
  nn = ((ceil(log2(n)) * n) - 1);
  if ((nn < 5))
  {
    nn = 5;
  }
  arra = cpp_new();
  arrw = cpp_new();
  suml = cpp_new();
  sumr = cpp_new();
  sum = cpp_new();
  sm = 0;
  {
    var i = 0;
    while ((i < n))
    {
      read(arra[i]);
      arra[i] -= 1;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      read(arrw[i]);
      sm += arrw[i];
      i += 1;
    }
  }
  mx = arra[(n - 1)];
}

func getValLeft(ind: dynamic)
{
  var w = arrw[ind];
  w *= (arra[ind] - (cpp_cast(ind)));
  w %= mod;
  return w;
}

func getVal(ind: dynamic)
{
  return arrw[ind];
}

func getValRight(ind: dynamic)
{
  var w = arrw[ind];
  w *= (((cpp_cast(mx)) - (cpp_cast(arra[ind]))) - (cpp_cast((((n - ind) - 1)))));
  w %= mod;
  return w;
}

func buildSeg(arr: dynamic, val: dynamic, s: dynamic, e: dynamic, i: dynamic, ismod: dynamic = true)
{
  if ((s == e))
  {
    arr[i] = val(s);
    if (ismod)
    {
      arr[i] %= mod;
    }
    return arr[i];
  }
  var m = (((s + e)) / 2);
  arr[i] = ((buildSeg(arr, val, s, m, ((i * 2) + 1), ismod) + buildSeg(arr, val, (m + 1), e, ((i * 2) + 2), ismod)));
  if (ismod)
  {
    arr[i] %= mod;
  }
  return arr[i];
}

func sumSeg(arr: dynamic, s: dynamic, e: dynamic, i: dynamic, rs: dynamic, re: dynamic, ismod: dynamic = true)
{
  if (((s >= rs) && (e <= re)))
  {
    return arr[i];
  }
  if (((e < rs) || (s > re)))
  {
    return 0;
  }
  var m = (((s + e)) / 2);
  var res = sumSeg(arr, s, m, ((i * 2) + 1), rs, re, ismod);
  res += sumSeg(arr, (m + 1), e, ((i * 2) + 2), rs, re, ismod);
  if (ismod)
  {
    res %= mod;
  }
  return res;
}

func updateSeg(arr: dynamic, s: dynamic, e: dynamic, i: dynamic, ind: dynamic, diff: dynamic, ismod: dynamic = true)
{
  if (((ind < s) || (ind > e)))
  {
    return;
  }
  arr[i] = ((arr[i] + diff));
  if (ismod)
  {
    arr[i] %= mod;
  }
  if ((s == e))
  {
    return;
  }
  var m = (((s + e)) / 2);
  updateSeg(arr, s, m, ((i * 2) + 1), ind, diff, ismod);
  updateSeg(arr, (m + 1), e, ((i * 2) + 2), ind, diff, ismod);
}

func segIndex(arr: dynamic, s: dynamic, e: dynamic, i: dynamic, sml: dynamic)
{
  if ((s == e))
  {
    return s;
  }
  var m = (((s + e)) / 2);
  if ((arr[((i * 2) + 1)] > sml))
  {
    return segIndex(arr, s, m, ((i * 2) + 1), sml);
  }
  return segIndex(arr, (m + 1), e, ((i * 2) + 2), (sml - arr[((i * 2) + 1)]));
}

func getMedian(s: dynamic, e: dynamic)
{
  var sml = sumSeg(sum, 0, (n - 1), 0, s, e, false);
  var smlb = 0;
  if ((s > 0))
  {
    smlb = sumSeg(sum, 0, (n - 1), 0, 0, (s - 1), false);
  }
  var ind = segIndex(sum, 0, (n - 1), 0, (smlb + (sml / 2)));
  return ind;
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  read();
  buildSeg(suml, getValLeft, 0, (n - 1), 0);
  buildSeg(sumr, getValRight, 0, (n - 1), 0);
  buildSeg(sum, getVal, 0, (n - 1), 0, false);
  if (TEST)
  {
    write("\n", "--------------", "\n");
    {
      var i = 0;
      while ((i < nn))
      {
        write(sum[i], " ");
        i += 1;
      }
    }
    write("\n");
    {
      var i = 0;
      while ((i < n))
      {
        write(getVal(i), " ");
        i += 1;
      }
    }
    write("\n");
    {
      var i = 0;
      while ((i < n))
      {
        write(getValLeft(i), " ");
        i += 1;
      }
    }
    write("\n");
    {
      var i = 0;
      while ((i < n))
      {
        write(getValRight(i), " ");
        i += 1;
      }
    }
    write("\n");
    write("--------------", "\n");
  }
  {
    var i = 0;
    while ((i < q))
    {
      var x: dynamic;
      var y: dynamic;
      read(x, y);
      if ((x < 0))
      {
        x = (-x);
        x -= 1;
        var vl = getValLeft(x);
        var vr = getValRight(x);
        var vm = getVal(x);
        arrw[x] = y;
        var nl = getValLeft(x);
        var nr = getValRight(x);
        var nm = y;
        var dl = (((((nl - vl)) + (mod * 2))) % mod);
        var dr = (((((nr - vr)) + (mod * 2))) % mod);
        var dm = (nm - vm);
        updateSeg(suml, 0, (n - 1), 0, x, dl);
        updateSeg(sumr, 0, (n - 1), 0, x, dr);
        updateSeg(sum, 0, (n - 1), 0, x, dm, false);
      } else
      {
        x -= 1;
        y -= 1;
        var med = getMedian(x, y);
        var need = 0;
        var pb: dynamic;
        var pa: dynamic;
        var tb: dynamic;
        var ta: dynamic;
        pa = (med + 1);
        pb = med;
        var smm = sumSeg(sum, 0, (n - 1), 0, x, y, false);
        if (TEST)
        {
          write("-------------", "\n");
          write(smm, " ", med, "\n");
        }
        if ((smm % 2))
        {
          pb -= 1;
          tb = (arra[med] - 1);
          ta = (arra[med] + 1);
        } else
        {
          var sb = 0;
          if ((med > x))
          {
            sb = sumSeg(sum, 0, (n - 1), 0, x, (med - 1), false);
          }
          if ((sb == (smm / 2)))
          {
            pa = pb;
            pb = (pb - 1);
            tb = (((arra[pb] + arra[pa])) / 2);
            ta = (tb + 1);
          } else
          {
            pb -= 1;
            tb = (arra[med] - 1);
            ta = (arra[med] + 1);
          }
        }
        if (TEST)
        {
          write(pb, " ", pa, "\n");
          write(tb, " ", ta, "\n");
        }
        if ((pb >= x))
        {
          if (TEST)
          {
            write("L: ", ((sumSeg(sumr, 0, (n - 1), 0, x, pb) % mod)), " - ", (sumSeg(sum, 0, (n - 1), 0, x, pb)), " * ", (((((((mx - tb) - ((n - pb)))) + 1)) % mod)), "\n");
          }
          need = ((((((need + (sumSeg(sumr, 0, (n - 1), 0, x, pb) % mod)) - (((sumSeg(sum, 0, (n - 1), 0, x, pb) * (((((((mx - tb) - ((n - pb)))) + 1)) % mod)))) % mod))) + mod)) % mod);
        }
        if ((pa <= y))
        {
          if (TEST)
          {
            write("R: ", ((sumSeg(suml, 0, (n - 1), 0, pa, y) % mod)), " - ", (sumSeg(sum, 0, (n - 1), 0, pa, y)), " * ", ((ta - pa)), "\n");
          }
          need = ((((((need + (sumSeg(suml, 0, (n - 1), 0, pa, y) % mod)) - (((sumSeg(sum, 0, (n - 1), 0, pa, y) * ((ta - pa)))) % mod))) + mod)) % mod);
        }
        if (TEST)
        {
          write("-------------", "\n");
        }
        write(need, "\n");
      }
      i += 1;
    }
  }
  return 0;
}
