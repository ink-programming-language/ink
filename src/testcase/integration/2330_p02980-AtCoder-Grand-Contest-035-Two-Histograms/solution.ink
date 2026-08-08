// Translated from solution.cpp.

var BUFF = (1 << 19);

var ibuf = cpp_array(BUFF);

var ib = ibuf;

var ie = ibuf;

func getc()
{
  if ((ib == ie))
  {
    ib = ibuf;
    ie = (ibuf + fread(ibuf, 1, BUFF, stdin));
  }
  return if ((ib == ie)) -1 else (*cpp_update(ib, "++"));
}

func read()
{
  var ret = 0;
  var pos = true;
  var c = getc();
  {
    while (((((c < cpp_char("0")) || (c > cpp_char("9")))) && (c != cpp_char("-"))))
    {
      assert((~c));
      c = getc();
    }
  }
  if ((c == cpp_char("-")))
  {
    pos = false;
    c = getc();
  }
  {
    while (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
    {
      ret = ((((ret << 3)) + ((ret << 1))) + ((c ^ 48)));
      c = getc();
    }
  }
  return if (pos) ret else (-ret);
}

var N = (5e5 + 5);

var MOD = 998244353;

func qpow(base: dynamic, e: dynamic)
{
  var ret = 1;
  {
    while (e)
    {
      if ((e & 1))
      {
        ret = ((cpp_cast(ret) * base) % MOD);
      }
      base = ((cpp_cast(base) * base) % MOD);
      e >>= 1;
    }
  }
  return ret;
}

var fac = cpp_array(N);

var inv = cpp_array(N);

func prep()
{
  fac[0] = 1;
  {
    var i = 1;
    while ((i <= (N - 1)))
    {
      fac[i] = ((cpp_cast(fac[(i - 1)]) * i) % MOD);
      i += 1;
    }
  }
  inv[(N - 1)] = qpow(fac[(N - 1)], (MOD - 2));
  {
    var i = (N - 1);
    while ((i >= 1))
    {
      inv[(i - 1)] = ((cpp_cast(inv[i]) * i) % MOD);
      i -= 1;
    }
  }
}

func comb(n: dynamic, m: dynamic)
{
  return ((((cpp_cast(fac[n]) * inv[m]) % MOD) * inv[(n - m)]) % MOD);
}

func main()
{
  prep();
  var n = read();
  var m = read();
  var ans = 0;
  if ((n > m))
  {
    swap(n, m);
  }
  var mul1 = qpow((n + 1), (m - n));
  var mul2 = 1;
  {
    var i = n;
    while ((i >= 0))
    {
      ans = (((ans + (((((((((if ((i & 1)) -1 else 1) * comb(n, i)) * comb(m, i)) % MOD) * fac[i]) % MOD) * mul1) % MOD) * mul2))) % MOD);
      mul1 = ((cpp_cast(mul1) * ((n + 1))) % MOD);
      mul2 = ((cpp_cast(mul2) * ((m + 1))) % MOD);
      i -= 1;
    }
  }
  if ((ans < 0))
  {
    ans += MOD;
  }
  write(ans);
  return 0;
}
