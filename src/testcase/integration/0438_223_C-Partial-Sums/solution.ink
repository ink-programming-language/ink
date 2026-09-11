// Translated from solution.cpp.

var mod: dynamic = (1e9 + 7);

var maxn: dynamic = 2005;

func safe_mul(a: dynamic, b: dynamic) -> dynamic
{
  a = ((((a * 1) * b)) % mod);
}

func mypow(a: dynamic, b: dynamic) -> dynamic
{
  var ans: dynamic = 1;
  var tmp: dynamic = a;
  while (b)
  {
    if ((b & 1))
    {
      safe_mul(ans, tmp);
    }
    safe_mul(tmp, tmp);
    b >>= 1;
  }
  return ans;
}

func inv(x: dynamic) -> dynamic
{
  return mypow(x, (mod - 2));
}

var koef: dynamic = cpp_array(maxn);

var a: dynamic = cpp_array(maxn);

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  koef[0] = 1;
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      koef[i] = koef[(i - 1)];
      safe_mul(koef[i], ((((((k + i) - 1)) * 1) * inv(i)) % mod));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var ans: dynamic = 0;
      {
        var j: dynamic = 0;
        while ((j <= i))
        {
          ans = (((ans + ((koef[j] * 1) * a[(i - j)]))) % mod);
          j += 1;
        }
      }
      write(ans, " \n"[(i == (n - 1))]);
      i += 1;
    }
  }
  return 0;
}
