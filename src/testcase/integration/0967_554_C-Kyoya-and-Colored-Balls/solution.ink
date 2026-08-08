// Translated from solution.cpp.

var Mod = (cpp_cast(1e9) + 7);

var MX = 2147483647;

var MXLL = 9223372036854775807;

var Sz = 1110111;

func Read_rap()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
}

var a = cpp_array(Sz);

var n: dynamic;

var f = cpp_array(Sz);

var ans = 1;

var len = 0;

func binpow(a: dynamic, b: dynamic)
{
  var res = 1;
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

func C(x: dynamic, y: dynamic)
{
  var res = f[((x + y) - 1)];
  res *= binpow(f[x], (Mod - 2));
  res %= Mod;
  res *= binpow(f[(y - 1)], (Mod - 2));
  res %= Mod;
  return res;
}

func main()
{
  Read_rap();
  f[0] = 1;
  {
    var i = 1;
    while ((i < Sz))
    {
      f[i] = (((f[(i - 1)] * i)) % Mod);
      i += 1;
    }
  }
  read(n);
  {
    var i = 1;
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
