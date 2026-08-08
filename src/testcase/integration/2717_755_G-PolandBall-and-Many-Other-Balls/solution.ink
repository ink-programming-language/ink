// Translated from solution.cpp.

var mod = 998244353;

var N = (1 << 15);

var Jie = cpp_array(N);

var inv = cpp_array(N);

var jie = cpp_array(N);

func mul(a: dynamic, b: dynamic)
{
  return (((1 * a) * b) % mod);
}

func add(a: dynamic, b: dynamic)
{
  a += b;
  if ((a >= mod))
  {
    a -= mod;
  }
  return;
}

func C(m: dynamic, n: dynamic)
{
  return mul(jie[m], mul(inv[n], inv[(m - n)]));
}

func CC(m: dynamic, n: dynamic)
{
  return mul(Jie[m], inv[n]);
}

func main()
{
  var n = 1000000000;
  var k = 32767;
  scanf("%d%d", (&n), (&k));
  Jie[0] = n;
  jie[0] = 1;
  inv[0] = cpp_assign(inv[1], "=", 1);
  {
    var i = (2);
    while ((i < ((k + 1))))
    {
      inv[i] = (((((-1 * mod) / i) * inv[(mod % i)]) % mod) + mod);
      i += 1;
    }
  }
  {
    var i = (1);
    while ((i < ((k + 1))))
    {
      Jie[i] = (n - i);
      jie[i] = mul(jie[(i - 1)], i);
      inv[i] = mul(inv[i], inv[(i - 1)]);
      i += 1;
    }
  }
  {
    var i = (1);
    while ((i < ((min(k, n) + 1))))
    {
      var ans = 0;
      {
        var j = (0);
        while ((j < ((i + 1))))
        {
          if (((n - j) < i))
          {
            break;
          }
          ans = (((ans + ((1 * C(i, j)) * CC(j, i)))) % mod);
          j += 1;
        }
      }
      {
        var j = (0);
        while ((j < ((k + 1))))
        {
          Jie[j] = (((1 * Jie[j]) * (((n - i) - j))) % mod);
          j += 1;
        }
      }
      printf("%d ", ans);
      i += 1;
    }
  }
  {
    var i = ((min(k, n) + 1));
    while ((i < ((k + 1))))
    {
      printf("0 ");
      i += 1;
    }
  }
}
