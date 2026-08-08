// Translated from solution.cpp.

var maxn = 1020;

var maxx = 10000;

var MOd = (1e9 + 7);

var K = 750;

var n: dynamic;

var k: dynamic;

var dn = cpp_array(maxn, maxn);

func mul(a: dynamic, b: dynamic)
{
  return ((cpp_cast(a) * b) % MOd);
}

func main()
{
  scanf("%d %d", (&n), (&k));
  var t = k;
  {
    var i = 1;
    while ((i <= (n - k)))
    {
      t = mul(t, (n - k));
      i += 1;
    }
  }
  var ans = 0;
  k -= 1;
  {
    var i = 0;
    while ((i < ((1 << k))))
    {
      dn[(((1 << k)) - 1)][i] = 1;
      i += 1;
    }
  }
  {
    var i = (((1 << k)) - 2);
    while ((i >= 0))
    {
      {
        var j = (((1 << k)) - 2);
        while ((j >= 0))
        {
          if ((((i | j)) == i))
          {
            var h = (((((1 << k)) - 1)) ^ i);
            {
              var k = h;
              while (k)
              {
                var l = k;
                var x = builtin_popcount(k);
                var y = builtin_popcount(j);
                var p = 1;
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
