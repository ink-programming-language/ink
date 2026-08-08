// Translated from solution.cpp.

var OX = 200;

var OY = 200;

var DX = [1, 2, -1, -2, -2, -1, 1, 2];

var DY = [2, 1, 2, 1, -1, -2, -2, -1];

var MOD = (1e9 + 7);

var N = 1e5;

var T = 50;

var n: dynamic;

var he: dynamic;

var ta: dynamic;

var dep = cpp_array((4 * OY), (4 * OX));

var cnt = cpp_array((T + 5));

var k: dynamic;

var q = cpp_array(N);

func main()
{
  scanf("%I64d%d", (&k), (&n));
  var x: dynamic;
  var y: dynamic;
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d%d", (&x), (&y));
      dep[(x + OX)][(y + OY)] = -1;
      i += 1;
    }
  }
  dep[(0 + OX)][(0 + OY)] = 1;
  cnt[1] += 1;
  q[cpp_update(ta, "++")] = [(0 + OX), (0 + OY)];
  var xx: dynamic;
  var yy: dynamic;
  var ted = 1;
  while ((cpp_update(he, "++") <= ta))
  {
    x = q[he].first;
    y = q[he].second;
    if (((dep[x][y] > k) || (dep[x][y] > T)))
    {
      continue;
    }
    {
      var i = 0;
      while ((i < 8))
      {
        xx = (x + DX[i]);
        yy = (y + DY[i]);
        if ((dep[xx][yy] == 0))
        {
          dep[xx][yy] = (dep[x][y] + 1);
          cnt[dep[xx][yy]] += 1;
          ted += 1;
          q[cpp_update(ta, "++")] = [xx, yy];
        }
        i += 1;
      }
    }
  }
  if (((k <= T) || (cnt[T] == 0)))
  {
    printf("%I64d\n", ted);
  } else
  {
    var z = (k - T);
    var z_plus_1 = (z + 1);
    var rem = (z % MOD);
    if ((z & 1))
    {
      z_plus_1 >>= 1;
    } else
    {
      z >>= 1;
    }
    z %= MOD;
    z_plus_1 %= MOD;
    var ans = (((((z * z_plus_1) % MOD)) * 28) % MOD);
    (cpp_assign(ans, "+=", ((rem * cnt[(T + 1)]) % MOD))) %= MOD;
    (cpp_assign(ans, "+=", ted)) %= MOD;
    printf("%I64d\n", ans);
  }
  return 0;
}
