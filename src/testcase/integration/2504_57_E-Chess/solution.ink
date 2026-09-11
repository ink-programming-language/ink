// Translated from solution.cpp.

var OX: dynamic = 200;

var OY: dynamic = 200;

var DX: dynamic = [1, 2, -1, -2, -2, -1, 1, 2];

var DY: dynamic = [2, 1, 2, 1, -1, -2, -2, -1];

var MOD: dynamic = (1e9 + 7);

var N: dynamic = 1e5;

var T: dynamic = 50;

var n: dynamic = cpp_uninitialized();

var he: dynamic = cpp_uninitialized();

var ta: dynamic = cpp_uninitialized();

var dep: dynamic = cpp_array((4 * OY), (4 * OX));

var cnt: dynamic = cpp_array((T + 5));

var k: dynamic = cpp_uninitialized();

var q: dynamic = cpp_array(N);

func main() -> dynamic
{
  scanf("%I64d%d", (&k), (&n));
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
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
  var xx: dynamic = cpp_uninitialized();
  var yy: dynamic = cpp_uninitialized();
  var ted: dynamic = 1;
  while ((cpp_update(he, "++") <= ta))
  {
    x = q[he].first;
    y = q[he].second;
    if (((dep[x][y] > k) || (dep[x][y] > T)))
    {
      continue;
    }
    {
      var i: dynamic = 0;
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
    var z: dynamic = (k - T);
    var z_plus_1: dynamic = (z + 1);
    var rem: dynamic = (z % MOD);
    if ((z & 1))
    {
      z_plus_1 >>= 1;
    } else
    {
      z >>= 1;
    }
    z %= MOD;
    z_plus_1 %= MOD;
    var ans: dynamic = (((((z * z_plus_1) % MOD)) * 28) % MOD);
    (cpp_assign(ans, "+=", ((rem * cnt[(T + 1)]) % MOD))) %= MOD;
    (cpp_assign(ans, "+=", ted)) %= MOD;
    printf("%I64d\n", ans);
  }
  return 0;
}
