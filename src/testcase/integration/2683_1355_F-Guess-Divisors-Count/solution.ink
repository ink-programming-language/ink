// Translated from solution.cpp.

var wmem: dynamic;

var memarr = cpp_array(96000000);

func min_L(a: dynamic, b: dynamic)
{
  return if ((a <= b)) a else b;
}

func max_L(a: dynamic, b: dynamic)
{
  return if ((a >= b)) a else b;
}

func walloc1d(arr: dynamic, x: dynamic, mem: dynamic = (&wmem))
{
  var skip = [0, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1];
  ((*mem)) = cpp_cast((((cpp_cast(((*mem)))) + skip[((cpp_cast(((*mem)))) & 15)])));
  ((*arr)) = cpp_cast(((*mem)));
  ((*mem)) = ((((*arr)) + x));
}

func Prime_L(N: dynamic, res: dynamic, mem: dynamic = wmem)
{
  var i: dynamic;
  var a: dynamic;
  var b: dynamic;
  var sz = 1;
  var r = 23000;
  var isprime: dynamic;
  var sf: dynamic;
  var ss = 1;
  walloc1d((&isprime), r, (&mem));
  walloc1d((&sf), r, (&mem));
  isprime = cpp_cast(mem);
  sf = cpp_cast(((isprime + r)));
  N /= 2;
  res[0] = 2;
  b = min_L(r, N);
  {
    i = (1);
    while ((i < (b)))
    {
      isprime[i] = 1;
      i += 1;
    }
  }
  {
    i = (1);
    while ((i < (b)))
    {
      if (isprime[i])
      {
        res[cpp_update(sz, "++")] = ((2 * i) + 1);
        sf[ss] = ((2 * i) * ((i + 1)));
        if ((sf[ss] < N))
        {
          while ((sf[ss] < r))
          {
            isprime[sf[ss]] = 0;
            sf[ss] += res[ss];
          }
          ss += 1;
        }
      }
      i += 1;
    }
  }
  {
    a = r;
    while ((a < N))
    {
      b = min_L((a + r), N);
      isprime -= r;
      {
        i = (a);
        while ((i < (b)))
        {
          isprime[i] = 1;
          i += 1;
        }
      }
      {
        i = (1);
        while ((i < (ss)))
        {
          while ((sf[i] < b))
          {
            isprime[sf[i]] = 0;
            sf[i] += res[i];
          }
          i += 1;
        }
      }
      {
        i = (a);
        while ((i < (b)))
        {
          if (isprime[i])
          {
            res[cpp_update(sz, "++")] = ((2 * i) + 1);
          }
          i += 1;
        }
      }
      a += r;
    }
  }
  return sz;
}

func Factor_L(N: dynamic, fac: dynamic, fs: dynamic)
{
  var i: dynamic;
  var sz = 0;
  if (((N % 2) == 0))
  {
    fac[sz] = 2;
    fs[sz] = 1;
    N /= 2;
    while (((N % 2) == 0))
    {
      N /= 2;
      fs[sz] += 1;
    }
    sz += 1;
  }
  {
    i = 3;
    while (((i * i) <= N))
    {
      if (((N % i) == 0))
      {
        fac[sz] = i;
        fs[sz] = 1;
        N /= i;
        while (((N % i) == 0))
        {
          N /= i;
          fs[sz] += 1;
        }
        sz += 1;
      }
      i += 2;
    }
  }
  if ((N > 1))
  {
    fac[sz] = N;
    fs[sz] = 1;
    sz += 1;
  }
  return sz;
}

func Divisor_L(N: dynamic, res: dynamic, mem: dynamic = wmem)
{
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var s: dynamic;
  var sz = 0;
  var fc: dynamic;
  var fs: dynamic;
  var fsz: dynamic;
  walloc1d((&fc), 100, (&mem));
  walloc1d((&fs), 100, (&mem));
  fsz = Factor_L(N, fc, fs);
  res[cpp_update(sz, "++")] = 1;
  {
    i = (0);
    while ((i < (fsz)))
    {
      s = sz;
      k = (s * fs[i]);
      {
        j = (0);
        while ((j < (k)))
        {
          res[cpp_update(sz, "++")] = (res[j] * fc[i]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  sort(res, (res + sz));
  return sz;
}

func query(Q: dynamic)
{
  printf("? %lld\n", Q);
  fflush(stdout);
  scanf("%lld", (&Q));
  return Q;
}

func answer(ans: dynamic)
{
  printf("! %lld\n", ans);
  fflush(stdout);
}

var ps: dynamic;

var p = cpp_array(10000);

var tmp = cpp_array(100000);

func main()
{
  var Lj4PdHRW: dynamic;
  wmem = memarr;
  var T: dynamic;
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var s: dynamic;
  var res: dynamic;
  var Q: dynamic;
  var X: dynamic;
  var now: dynamic;
  ps = Prime_L(10000, p);
  scanf("%d", (&T));
  {
    Lj4PdHRW = (0);
    while ((Lj4PdHRW < (T)))
    {
      var WYIGIcGE: dynamic;
      var e98WHCEY: dynamic;
      s = 0;
      X = 1;
      {
        e98WHCEY = (0);
        while ((e98WHCEY < (18)))
        {
          Q = 1;
          while (((cpp_cast(Q) * p[s]) < 1e18))
          {
            Q *= p[cpp_update(s, "++")];
          }
          X *= query(Q);
          e98WHCEY += 1;
        }
      }
      now = X;
      {
        WYIGIcGE = (0);
        while ((WYIGIcGE < (4)))
        {
          now *= now;
          now = query(now);
          WYIGIcGE += 1;
        }
      }
      res = Divisor_L(now, tmp);
      answer(max_L(8, (2 * res)));
      Lj4PdHRW += 1;
    }
  }
  return 0;
}
