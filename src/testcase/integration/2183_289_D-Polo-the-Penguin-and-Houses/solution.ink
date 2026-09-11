// Translated from solution.cpp.

var maxn: dynamic = 1020;

var maxx: dynamic = 10000;

var MOd: dynamic = (1e9 + 7);

var K: dynamic = 750;

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var dn: dynamic = cpp_array(maxn, maxn);

func mul(a: dynamic, b: dynamic) -> dynamic
{
  return ((cpp_cast(a) * b) % MOd);
}

func main() -> dynamic
{
  scanf("%d %d", (&n), (&k));
  var t: dynamic = k;
  {
    var i: dynamic = 1;
    while ((i <= (n - k)))
    {
      t = mul(t, (n - k));
      i += 1;
    }
  }
  var ans: dynamic = 0;
  k -= 1;
  {
    var i: dynamic = 0;
    while ((i < ((1 << k))))
    {
      dn[(((1 << k)) - 1)][i] = 1;
      i += 1;
    }
  }
  {
    var i: dynamic = (((1 << k)) - 2);
    while ((i >= 0))
    {
      {
        var j: dynamic = (((1 << k)) - 2);
        while ((j >= 0))
        {
          if ((((i | j)) == i))
          {
            var h: dynamic = (((((1 << k)) - 1)) ^ i);
            {
              var k: dynamic = h;
              while (k)
              {
                var l: dynamic = k;
                var x: dynamic = builtin_popcount(k);
                var y: dynamic = builtin_popcount(j);
                var p: dynamic = 1;
                while (cpp_update(y, "--"))
                {
                  p = mul(p, x);
                }
                p %= MOd;
                dn[i][j] += mul(dn[(i | k)][k], p);
                dn[i][j] %= MOd;
                k = (((k - 1)) & h);
              }
            }
          }
          j -= 1;
        }
      }
      i -= 1;
    }
  }
  ans = mul(t, dn[0][0]);
  write(ans, "\n");
  return 0;
}
