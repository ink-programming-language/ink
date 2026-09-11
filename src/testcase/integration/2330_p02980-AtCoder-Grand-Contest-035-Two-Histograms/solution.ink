// Translated from solution.cpp.

var BUFF: dynamic = (1 << 19);

var ibuf: dynamic = cpp_array(BUFF);

var ib: dynamic = ibuf;

var ie: dynamic = ibuf;

func getc() -> dynamic
{
  if ((ib == ie))
  {
    ib = ibuf;
    ie = (ibuf + fread(ibuf, 1, BUFF, stdin));
  }
  return  ((ib == ie)) ? -1 : (*cpp_update(ib, "++"));
}

func read() -> dynamic
{
  var ret: dynamic = 0;
  var pos: dynamic = true;
  var c: dynamic = getc();
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
  return  (pos) ? ret : (-ret);
}

var N: dynamic = (5e5 + 5);

var MOD: dynamic = 998244353;

func qpow(base: dynamic, e: dynamic) -> dynamic
{
  var ret: dynamic = 1;
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

var fac: dynamic = cpp_array(N);

var inv: dynamic = cpp_array(N);

func prep() -> dynamic
{
  fac[0] = 1;
  {
    var i: dynamic = 1;
    while ((i <= (N - 1)))
    {
      fac[i] = ((cpp_cast(fac[(i - 1)]) * i) % MOD);
      i += 1;
    }
  }
  inv[(N - 1)] = qpow(fac[(N - 1)], (MOD - 2));
  {
    var i: dynamic = (N - 1);
    while ((i >= 1))
    {
      inv[(i - 1)] = ((cpp_cast(inv[i]) * i) % MOD);
      i -= 1;
    }
  }
}

func comb(n: dynamic, m: dynamic) -> dynamic
{
  return ((((cpp_cast(fac[n]) * inv[m]) % MOD) * inv[(n - m)]) % MOD);
}

func main() -> dynamic
{
  prep();
  var n: dynamic = read();
  var m: dynamic = read();
  var ans: dynamic = 0;
  if ((n > m))
  {
    swap(n, m);
  }
  var mul1: dynamic = qpow((n + 1), (m - n));
  var mul2: dynamic = 1;
  {
    var i: dynamic = n;
    while ((i >= 0))
    {
      ans = (((ans + ((((((((( ((i & 1)) ? -1 : 1) * comb(n, i)) * comb(m, i)) % MOD) * fac[i]) % MOD) * mul1) % MOD) * mul2))) % MOD);
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
