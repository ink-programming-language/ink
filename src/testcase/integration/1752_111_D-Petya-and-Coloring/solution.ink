// Translated from solution.cpp.

var MOD: dynamic = 1000000007;

var UNDEF: dynamic = -1;

var INF: dynamic = (1 << 30);

func chkmax(aa: dynamic, bb: dynamic) -> dynamic
{
  return  ((aa < bb)) ? cpp_comma(cpp_assign(aa, "=", bb), true) : false;
}

func chkmin(aa: dynamic, bb: dynamic) -> dynamic
{
  return  ((aa > bb)) ? cpp_comma(cpp_assign(aa, "=", bb), true) : false;
}

var mn: dynamic = 1002;

var dp: dynamic = cpp_array(mn, mn);

func stirling(n: dynamic, k: dynamic) -> dynamic
{
  if ((dp[n][k] != -1))
  {
    return dp[n][k];
  }
  var ans: dynamic = cpp_uninitialized();
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
  var x: dynamic = cpp_uninitialized();
  func norm(x: dynamic) -> dynamic
  {
      if ((x < 0))
      {
        x += MOD;
      }
      return x;
    }
  func mint() -> dynamic
  {
      self->x = cpp_construct(0);
    }
  func mint(sig: dynamic) -> dynamic
  {
      sig = norm(sig);
      x = sig;
    }
  func mint(sig: dynamic) -> dynamic
  {
      sig = norm((sig % MOD));
      x = sig;
    }
  func get() -> dynamic
  {
      return cpp_cast(x);
    }
  func operator_add_assign(that: dynamic) -> dynamic
  {
      if (((cpp_assign(x, "+=", that.x)) >= MOD))
      {
        x -= MOD;
      }
      return (*self);
    }
  func operator_subtract_assign(that: dynamic) -> dynamic
  {
      if (((cpp_assign(x, "+=", (MOD - that.x))) >= MOD))
      {
        x -= MOD;
      }
      return (*self);
    }
  func operator(that: dynamic) -> dynamic
  {
      x = (((cpp_cast(x) * that.x)) % MOD);
      return (*self);
    }
  func operator(that: dynamic) -> dynamic
  {
      return cpp_assign((*self), "*=", that.inverse());
    }
  func operator_add_assign(that: dynamic) -> dynamic
  {
      that = norm(that);
      if (((cpp_assign(x, "+=", that)) >= MOD))
      {
        x -= MOD;
      }
      return (*self);
    }
  func operator_subtract_assign(that: dynamic) -> dynamic
  {
      that = norm(that);
      if (((cpp_assign(x, "+=", (MOD - that))) >= MOD))
      {
        x -= MOD;
      }
      return (*self);
    }
  func operator(that: dynamic) -> dynamic
  {
      that = norm(that);
      x = (((cpp_cast(x) * that)) % MOD);
      return (*self);
    }
  func operator(that: dynamic) -> dynamic
  {
      that = norm(that);
      return cpp_assign((*self), "*=", mint(that).inverse());
    }
  func operator_add(that: dynamic) -> dynamic
  {
      return cpp_assign(mint((*self)), "+=", that);
    }
  func operator_subtract(that: dynamic) -> dynamic
  {
      return cpp_assign(mint((*self)), "-=", that);
    }
  func operator_multiply(that: dynamic) -> dynamic
  {
      return cpp_assign(mint((*self)), "*=", that);
    }
  func operator_divide(that: dynamic) -> dynamic
  {
      return cpp_assign(mint((*self)), "/=", that);
    }
  func operator_add(that: dynamic) -> dynamic
  {
      return cpp_assign(mint((*self)), "+=", that);
    }
  func operator_subtract(that: dynamic) -> dynamic
  {
      return cpp_assign(mint((*self)), "-=", that);
    }
  func operator_multiply(that: dynamic) -> dynamic
  {
      return cpp_assign(mint((*self)), "*=", that);
    }
  func operator_divide(that: dynamic) -> dynamic
  {
      return cpp_assign(mint((*self)), "/=", that);
    }
  func inverse() -> dynamic
  {
      var a: dynamic = x;
      var b: dynamic = MOD;
      var u: dynamic = 1;
      var v: dynamic = 0;
      while (b)
      {
        var t: dynamic = (a / b);
        a -= (t * b);
        swap(a, b);
        u -= (t * v);
        swap(u, v);
      }
      if ((u < 0))
      {
        u += MOD;
      }
      var res: dynamic = cpp_uninitialized();
      res.x = cpp_cast(u);
      return res;
    }
  func operator_equal(that: dynamic) -> dynamic
  {
      return (x == that.x);
    }
  func operator_not_equal(that: dynamic) -> dynamic
  {
      return (x != that.x);
    }
  func operator_subtract() -> dynamic
  {
      var t: dynamic = cpp_uninitialized();
      t.x =  ((x == 0)) ? 0 : (MOD - x);
      return t;
    }
  func operator(k: dynamic) -> dynamic
  {
      var a: dynamic = ((*self));
      var r: dynamic = 1;
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

var MAXFACT: dynamic = (1e6 + 4);

var fact: dynamic = cpp_array((MAXFACT + 1));

var invfact: dynamic = cpp_array((MAXFACT + 1));

func init() -> dynamic
{
  var got: dynamic = 1;
  {
    var x: dynamic = 0;
    while ((x <= MAXFACT))
    {
      fact[x] = got;
      got *= ((x + 1));
      x += 1;
    }
  }
  got = got.inverse();
  {
    var x: dynamic = MAXFACT;
    while ((x >= 0))
    {
      got *= ((x + 1));
      invfact[x] = got;
      x -= 1;
    }
  }
}

func binom(n: dynamic, k: dynamic) -> dynamic
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

func distinctObjectsDistinctNonemptyBins(n: dynamic, bins: dynamic) -> dynamic
{
  var ans: dynamic = 0;
  var sign: dynamic = 1;
  {
    var k: dynamic = bins;
    while ((k > 0))
    {
      var got: dynamic = (binom(bins, k) * ((mint(k) ^ n)));
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

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  init();
  memset(dp, -1, cpp_sizeof(dp));
  var r: dynamic = rint();
  var c: dynamic = rint();
  var klim: dynamic = rint();
  var final_cpp: dynamic = 0;
  if (((c == 1) || (c == 2)))
  {
    var ans: dynamic = 0;
    {
      var k: dynamic = 1;
      while ((k <= min(r, klim)))
      {
        var got: dynamic = ((binom(klim, k) * stirling(r, k)) * fact[k]);
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
    var ans: dynamic = 0;
    {
      var k: dynamic = 1;
      while ((k <= min(r, klim)))
      {
        var inside: dynamic = (binom(klim, k) * distinctObjectsDistinctNonemptyBins((((c - 2)) * r), k));
        var got: dynamic = 0;
        {
          var d: dynamic = 0;
          while ((d <= (min(r, klim) - k)))
          {
            var side: dynamic = (binom((klim - k), d) * ((fact[(k + d)] * stirling(r, (k + d)))));
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

var stdinBuffer: dynamic = cpp_array(1024);

var stdinDataEnd: dynamic = (stdinBuffer + cpp_sizeof((stdinBuffer)));

var stdinPos: dynamic = stdinDataEnd;

func readAhead(amount: dynamic) -> dynamic
{
  var remaining: dynamic = (stdinDataEnd - stdinPos);
  if ((remaining < amount))
  {
    memmove(stdinBuffer, stdinPos, remaining);
    var sz: dynamic = fread((stdinBuffer + remaining), 1, (cpp_sizeof((stdinBuffer)) - remaining), stdin);
    stdinPos = stdinBuffer;
    stdinDataEnd = ((stdinBuffer + remaining) + sz);
    if ((stdinDataEnd != (stdinBuffer + cpp_sizeof((stdinBuffer)))))
    {
      (*stdinDataEnd) = 0;
    }
  }
}

func rint() -> dynamic
{
  readAhead(16);
  var x: dynamic = 0;
  var neg: dynamic = false;
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
  return  (neg) ? (-x) : x;
}

func rch() -> dynamic
{
  readAhead(16);
  while ((((*stdinPos) == cpp_char(" ")) || ((*stdinPos) == cpp_char("\n"))))
  {
    stdinPos += 1;
  }
  var ans: dynamic = (*stdinPos);
  stdinPos += 1;
  return ans;
}

func rlong() -> dynamic
{
  readAhead(32);
  var x: dynamic = 0;
  var neg: dynamic = false;
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
  return  (neg) ? (-x) : x;
}
