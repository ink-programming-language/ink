// Translated from solution.cpp.

var MOD = 1000000007;

var UNDEF = -1;

var INF = (1 << 30);

func chkmax(aa: dynamic, bb: dynamic)
{
  return if ((aa < bb)) cpp_comma(cpp_assign(aa, "=", bb), true) else false;
}

func chkmin(aa: dynamic, bb: dynamic)
{
  return if ((aa > bb)) cpp_comma(cpp_assign(aa, "=", bb), true) else false;
}

var mn = 1002;

var dp = cpp_array(mn, mn);

func stirling(n: dynamic, k: dynamic)
{
  if ((dp[n][k] != -1))
  {
    return dp[n][k];
  }
  var ans: dynamic;
  if (((n == 0) && (k == 0)))
  {
    ans = 1;
  } else if (((n == 0) || (k == 0)))
  {
    ans = 0;
  } else
  {
    ans = ((((k * cpp_cast(stirling((n - 1), k))) + stirling((n - 1), (k - 1)))) % MOD);
  }
  return cpp_assign(dp[n][k], "=", ans);
}

class mint
{
  var x: dynamic;
  func norm(x: dynamic)
  {
      if ((x < 0))
      {
        x += MOD;
      }
      return x;
    }
  func mint()
  {
      this->x = cpp_construct(0);
    }
  func mint(sig: dynamic)
  {
      sig = norm(sig);
      x = sig;
    }
  func mint(sig: dynamic)
  {
      sig = norm((sig % MOD));
      x = sig;
    }
  func get()
  {
      return cpp_cast(x);
    }
  func operator_add_assign(that: dynamic)
  {
      if (((cpp_assign(x, "+=", that.x)) >= MOD))
      {
        x -= MOD;
      }
      return (*this);
    }
  func operator_subtract_assign(that: dynamic)
  {
      if (((cpp_assign(x, "+=", (MOD - that.x))) >= MOD))
      {
        x -= MOD;
      }
      return (*this);
    }
  func operator(that: dynamic)
  {
      x = (((cpp_cast(x) * that.x)) % MOD);
      return (*this);
    }
  func operator(that: dynamic)
  {
      return cpp_assign((*this), "*=", that.inverse());
    }
  func operator_add_assign(that: dynamic)
  {
      that = norm(that);
      if (((cpp_assign(x, "+=", that)) >= MOD))
      {
        x -= MOD;
      }
      return (*this);
    }
  func operator_subtract_assign(that: dynamic)
  {
      that = norm(that);
      if (((cpp_assign(x, "+=", (MOD - that))) >= MOD))
      {
        x -= MOD;
      }
      return (*this);
    }
  func operator(that: dynamic)
  {
      that = norm(that);
      x = (((cpp_cast(x) * that)) % MOD);
      return (*this);
    }
  func operator(that: dynamic)
  {
      that = norm(that);
      return cpp_assign((*this), "*=", mint(that).inverse());
    }
  func operator_add(that: dynamic)
  {
      return cpp_assign(mint((*this)), "+=", that);
    }
  func operator_subtract(that: dynamic)
  {
      return cpp_assign(mint((*this)), "-=", that);
    }
  func operator_multiply(that: dynamic)
  {
      return cpp_assign(mint((*this)), "*=", that);
    }
  func operator_divide(that: dynamic)
  {
      return cpp_assign(mint((*this)), "/=", that);
    }
  func operator_add(that: dynamic)
  {
      return cpp_assign(mint((*this)), "+=", that);
    }
  func operator_subtract(that: dynamic)
  {
      return cpp_assign(mint((*this)), "-=", that);
    }
  func operator_multiply(that: dynamic)
  {
      return cpp_assign(mint((*this)), "*=", that);
    }
  func operator_divide(that: dynamic)
  {
      return cpp_assign(mint((*this)), "/=", that);
    }
  func inverse()
  {
      var a = x;
      var b = MOD;
      var u = 1;
      var v = 0;
      while (b)
      {
        var t = (a / b);
        a -= (t * b);
        swap(a, b);
        u -= (t * v);
        swap(u, v);
      }
      if ((u < 0))
      {
        u += MOD;
      }
      var res: dynamic;
      res.x = cpp_cast(u);
      return res;
    }
  func operator_equal(that: dynamic)
  {
      return (x == that.x);
    }
  func operator_not_equal(that: dynamic)
  {
      return (x != that.x);
    }
  func operator_subtract()
  {
      var t: dynamic;
      t.x = if ((x == 0)) 0 else (MOD - x);
      return t;
    }
  func operator(k: dynamic)
  {
      var a = ((*this));
      var r = 1;
      while (k)
      {
        if ((k & 1))
        {
          r *= a;
        }
        a *= a;
        k >>= 1;
      }
      return r;
    }
}

var MAXFACT = (1e6 + 4);

var fact = cpp_array((MAXFACT + 1));

var invfact = cpp_array((MAXFACT + 1));

func init()
{
  var got = 1;
  {
    var x = 0;
    while ((x <= MAXFACT))
    {
      fact[x] = got;
      got *= ((x + 1));
      x += 1;
    }
  }
  got = got.inverse();
  {
    var x = MAXFACT;
    while ((x >= 0))
    {
      got *= ((x + 1));
      invfact[x] = got;
      x -= 1;
    }
  }
}

func binom(n: dynamic, k: dynamic)
{
  if ((n < k))
  {
    return mint(0);
  }
  if (((n < 0) || (k < 0)))
  {
    return mint(0);
  }
  return ((fact[n] * invfact[k]) * invfact[(n - k)]);
}

func distinctObjectsDistinctNonemptyBins(n: dynamic, bins: dynamic)
{
  var ans = 0;
  var sign = 1;
  {
    var k = bins;
    while ((k > 0))
    {
      var got = (binom(bins, k) * ((mint(k) ^ n)));
      if ((sign == 1))
      {
        ans += got;
      } else
      {
        ans -= got;
      }
      sign = (-sign);
      k -= 1;
    }
  }
  return ans;
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  init();
  memset(dp, -1, cpp_sizeof(dp));
  var r = rint();
  var c = rint();
  var klim = rint();
  var final_cpp = 0;
  if (((c == 1) || (c == 2)))
  {
    var ans = 0;
    {
      var k = 1;
      while ((k <= min(r, klim)))
      {
        var got = ((binom(klim, k) * stirling(r, k)) * fact[k]);
        if ((c == 1))
        {
          ans += got;
        }
        if ((c == 2))
        {
          ans += (got * got);
        }
        k += 1;
      }
    }
    final_cpp = ans;
  } else
  {
    var ans = 0;
    {
      var k = 1;
      while ((k <= min(r, klim)))
      {
        var inside = (binom(klim, k) * distinctObjectsDistinctNonemptyBins((((c - 2)) * r), k));
        var got = 0;
        {
          var d = 0;
          while ((d <= (min(r, klim) - k)))
          {
            var side = (binom((klim - k), d) * ((fact[(k + d)] * stirling(r, (k + d)))));
            got += (side * side);
            d += 1;
          }
        }
        ans += (got * inside);
        k += 1;
      }
    }
    final_cpp = ans;
  }
  printf("%d\n", final_cpp.get());
}

var stdinBuffer = cpp_array(1024);

var stdinDataEnd = (stdinBuffer + cpp_sizeof((stdinBuffer)));

var stdinPos = stdinDataEnd;

func readAhead(amount: dynamic)
{
  var remaining = (stdinDataEnd - stdinPos);
  if ((remaining < amount))
  {
    memmove(stdinBuffer, stdinPos, remaining);
    var sz = fread((stdinBuffer + remaining), 1, (cpp_sizeof((stdinBuffer)) - remaining), stdin);
    stdinPos = stdinBuffer;
    stdinDataEnd = ((stdinBuffer + remaining) + sz);
    if ((stdinDataEnd != (stdinBuffer + cpp_sizeof((stdinBuffer)))))
    {
      (*stdinDataEnd) = 0;
    }
  }
}

func rint()
{
  readAhead(16);
  var x = 0;
  var neg = false;
  while ((((*stdinPos) == cpp_char(" ")) || ((*stdinPos) == cpp_char("\n"))))
  {
    stdinPos += 1;
  }
  if (((*stdinPos) == cpp_char("-")))
  {
    stdinPos += 1;
    neg = true;
  }
  while ((((*stdinPos) >= cpp_char("0")) && ((*stdinPos) <= cpp_char("9"))))
  {
    x *= 10;
    x += ((*stdinPos) - cpp_char("0"));
    stdinPos += 1;
  }
  return if (neg) (-x) else x;
}

func rch()
{
  readAhead(16);
  while ((((*stdinPos) == cpp_char(" ")) || ((*stdinPos) == cpp_char("\n"))))
  {
    stdinPos += 1;
  }
  var ans = (*stdinPos);
  stdinPos += 1;
  return ans;
}

func rlong()
{
  readAhead(32);
  var x = 0;
  var neg = false;
  while ((((*stdinPos) == cpp_char(" ")) || ((*stdinPos) == cpp_char("\n"))))
  {
    stdinPos += 1;
  }
  if (((*stdinPos) == cpp_char("-")))
  {
    stdinPos += 1;
    neg = true;
  }
  while ((((*stdinPos) >= cpp_char("0")) && ((*stdinPos) <= cpp_char("9"))))
  {
    x *= 10;
    x += ((*stdinPos) - cpp_char("0"));
    stdinPos += 1;
  }
  return if (neg) (-x) else x;
}
