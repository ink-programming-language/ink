// Translated from solution.cpp.

var Mod: dynamic = (cpp_cast(1e9) + 7);

var MX: dynamic = 2147483647;

var MXLL: dynamic = 9223372036854775807;

var Sz: dynamic = 1110111;

func Read_rap() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
}

var a: dynamic = cpp_array(Sz);

var n: dynamic = cpp_uninitialized();

var f: dynamic = cpp_array(Sz);

var ans: dynamic = 1;

var len: dynamic = 0;

func binpow(a: dynamic, b: dynamic) -> dynamic
{
  var res: dynamic = 1;
  while (b)
  {
    if ((b & 1))
    {
      res *= a;
      res %= Mod;
      b -= 1;
    }
    a *= a;
    a %= Mod;
    b >>= 1;
  }
  return res;
}

func C(x: dynamic, y: dynamic) -> dynamic
{
  var res: dynamic = f[((x + y) - 1)];
  res *= binpow(f[x], (Mod - 2));
  res %= Mod;
  res *= binpow(f[(y - 1)], (Mod - 2));
  res %= Mod;
  return res;
}

func main() -> dynamic
{
  Read_rap();
  f[0] = 1;
  {
    var i: dynamic = 1;
    while ((i < Sz))
    {
      f[i] = (((f[(i - 1)] * i)) % Mod);
      i += 1;
    }
  }
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(a[i]);
      ans *= C(len, a[i]);
      ans %= Mod;
      len += a[i];
      i += 1;
    }
  }
  write(ans);
  return 0;
}
