// Translated from solution.cpp.

var MOD = cpp_expression("#include<ios");

var cmemo = cpp_array(4000, 4000);

func C(x: dynamic, y: dynamic)
{
  if ((x < y))
  {
    return 0;
  }
  var res = cmemo[x][y];
  if ((res != -1))
  {
    return res;
  }
  if (((y == 0) || (x == y)))
  {
    return cpp_assign(res, "=", 1);
  }
  return cpp_assign(res, "=", (((C((x - 1), y) + C((x - 1), (y - 1)))) % MOD));
}

func cnt(n: dynamic, k: dynamic)
{
  if ((n < 0))
  {
    return 0;
  }
  return C(((n + k) - 1), n);
}

func solve(n: dynamic, ban: dynamic, k: dynamic)
{
  var res = 0;
  var p = 1;
  {
    var i = 1;
    while ((i <= ban))
    {
      res += (((p * C(ban, i)) * cnt((n - (2 * i)), k)) % MOD);
      res %= MOD;
      p *= -1;
      i += 1;
    }
  }
  if ((res < 0))
  {
    res += MOD;
  }
  return (cnt(n, k) - res);
}

func cnthoge(x: dynamic, k: dynamic)
{
  var res = 0;
  {
    var i = 1;
    while ((i <= k))
    {
      var kk = (x - i);
      if ((((0 < kk) && (kk <= k)) && (kk != i)))
      {
        res += 1;
      }
      i += 1;
    }
  }
  return (res / 2);
}

func main()
{
  var k: dynamic;
  var n: dynamic;
  {
    var i = 0;
    while ((i < 4000))
    {
      {
        var j = 0;
        while ((j < 4000))
        {
          cmemo[i][j] = -1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  read(k, n);
  {
    var i = 2;
    while ((i <= (2 * k)))
    {
      var res = 0;
      if (((i % 2) == 0))
      {
        res += solve(n, cnthoge(i, k), (k - 1));
        res += solve((n - 1), cnthoge(i, k), (k - 1));
        res %= MOD;
      } else
      {
        res += solve(n, cnthoge(i, k), k);
      }
      res %= MOD;
      if ((res < 0))
      {
        res += MOD;
      }
      write(res, "\n");
      i += 1;
    }
  }
  return 0;
}
