// Translated from solution.cpp.

var n: dynamic;

var k: dynamic;

var c1: dynamic;

var c2: dynamic;

var cache = cpp_array(201, 2, 52, 52);

var cache1 = cpp_array(201, 2, 52, 52);

var mod = (1e9 + 7);

var fact = cpp_array(300005);

func inv(x: dynamic)
{
  var r: dynamic;
  var y: dynamic;
  {
    r = 1;
    y = (mod - 2);
    while ((y > 0))
    {
      if (((y % 2) == 1))
      {
        r = ((r * x) % mod);
      }
      x = (((x * x)) % mod);
      y /= 2;
    }
  }
  return r;
}

func nCr(n: dynamic, m: dynamic)
{
  return ((((fact[n] * inv(fact[m])) % mod) * inv(fact[(n - m)])) % mod);
}

func dp(lt1: dynamic, lt2: dynamic, side: dynamic, tk: dynamic)
{
  if ((tk > 200))
  {
    return 1e18;
  }
  if ((((lt1 + lt2) == 0) && side))
  {
    return tk;
  }
  var ans = cache[lt1][lt2][side][tk];
  if ((ans != -1))
  {
    return ans;
  }
  ans = 1e18;
  if ((side == 1))
  {
    var has1 = (c1 - lt1);
    var has2 = (c2 - lt2);
    if (((has1 == 0) && (k >= 100)))
    {
      ans = dp(lt1, (lt2 + 1), 0, (tk + 1));
    } else if (((has2 == 0) && (k >= 50)))
    {
      ans = dp((lt1 + 1), lt2, 0, (tk + 1));
    } else if ((k >= 100))
    {
      ans = min(dp((lt1 + 1), lt2, 0, (tk + 1)), dp(lt1, (lt2 + 1), 0, (tk + 1)));
    }
  } else
  {
    {
      var tk1 = 0;
      while ((tk1 <= lt1))
      {
        {
          var tk2 = lt2;
          while ((tk2 >= 0))
          {
            if (((!(((tk1 == 0) && (tk2 == 0)))) && (((50 * tk1) + (100 * tk2)) <= k)))
            {
              ans = min(ans, dp((lt1 - tk1), (lt2 - tk2), 1, (tk + 1)));
            }
            tk2 -= 1;
          }
        }
        tk1 += 1;
      }
    }
  }
  return ans;
}

func path(lt1: dynamic, lt2: dynamic, side: dynamic, left: dynamic)
{
  if ((left < 0))
  {
    return 0;
  }
  if ((((lt1 + lt2) == 0) && (left == 0)))
  {
    return 1;
  }
  var ans1 = cache1[lt1][lt2][side][left];
  if ((ans1 != -1))
  {
    return ans1;
  }
  ans1 = 0;
  var ans = 1e18;
  if ((side == 1))
  {
    var has1 = (c1 - lt1);
    var has2 = (c2 - lt2);
    {
      var i = 0;
      while ((i <= has1))
      {
        {
          var j = 0;
          while ((j <= has2))
          {
            if (((((i + j) != 0) && ((i + j) != (has1 + has2))) && (((i * 50) + (j * 100)) <= k)))
            {
              var ct = (((nCr(has1, i) * nCr(has2, j))) % mod);
              ans1 += (ct * path((lt1 + i), (lt2 + j), 0, (left - 1)));
              if ((ans1 >= mod))
              {
                ans1 %= mod;
              }
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
  } else
  {
    {
      var tk1 = lt1;
      while ((tk1 >= 0))
      {
        {
          var tk2 = lt2;
          while ((tk2 >= 0))
          {
            if (((!(((tk1 == 0) && (tk2 == 0)))) && (((50 * tk1) + (100 * tk2)) <= k)))
            {
              var ct = (((nCr(lt1, tk1) * nCr(lt2, tk2))) % mod);
              ans1 += (((ct * path((lt1 - tk1), (lt2 - tk2), 1, (left - 1)))) % mod);
            }
            tk2 -= 1;
          }
        }
        tk1 -= 1;
      }
    }
  }
  ans1 %= mod;
  return ans1;
}

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  fact[0] = 1;
  {
    var i = 1;
    while ((i < 300005))
    {
      fact[i] = (((i * fact[(i - 1)])) % mod);
      i += 1;
    }
  }
  read(n, k);
  {
    var i = 0;
    while ((i < n))
    {
      var a: dynamic;
      read(a);
      if ((a == 50))
      {
        c1 += 1;
      } else
      {
        c2 += 1;
      }
      i += 1;
    }
  }
  memset(cache, -1, cpp_sizeof((cache)));
  if ((dp(c1, c2, 0, 0) > 1e15))
  {
    return cpp_comma((cout << "-1\n0"), 0);
  }
  var ans = dp(c1, c2, 0, 0);
  write(ans, cpp_char("\n"));
  memset(cache1, -1, cpp_sizeof((cache1)));
  write(path(c1, c2, 0, ans), cpp_char("\n"));
}
