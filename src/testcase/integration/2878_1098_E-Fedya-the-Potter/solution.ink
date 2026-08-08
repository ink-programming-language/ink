// Translated from solution.cpp.

var inf = 0x3f3f3f3f;

var Inf = 0x7fffffff;

var INF = 0x3f3f3f3f3f3f3f3f;

func rnd()
{
  var seed = 416;
  return cpp_comma(cpp_assign(seed, "+=", 0x71dad4bf), cpp_comma(cpp_assign(seed, "^=", (seed >> 5)), cpp_comma(cpp_assign(seed, "+=", 0xc6f74d88), cpp_comma(cpp_assign(seed, "^=", (seed << 17)), cpp_comma(cpp_assign(seed, "+=", 0x25e6561), cpp_assign(seed, "^=", (seed >> 13)))))));
}

func gcd(a: dynamic, b: dynamic)
{
  return if (((!b))) a else gcd(b, (a % b));
}

func abs(a: dynamic)
{
  return if ((a >= 0)) a else (-a);
}

func chmax(a: dynamic, b: dynamic)
{
  (((a < b)) && (cpp_assign(a, "=", b)));
}

func chmin(a: dynamic, b: dynamic)
{
  (((b < a)) && (cpp_assign(a, "=", b)));
}

func read(x: dynamic)
{
  var f = cpp_construct(false);
  while ((!isdigit(ch)))
  {
    f |= (ch == 45);
    ch = getchar();
  }
  x = (ch & 15);
  ch = getchar();
  while (isdigit(ch))
  {
    x = (((((((x << 2)) + x)) << 1)) + ((ch & 15)));
    ch = getchar();
  }
  (f && (cpp_assign(x, "=", (-x))));
}

func read(t: dynamic, args: dynamic...)
{
  read(t);
  read(cpp_expand(args));
}

func min(a: dynamic, b: dynamic, args: dynamic...)
{
  return if ((a < b)) min(a, cpp_expand(args)) else min(b, cpp_expand(args));
}

func max(a: dynamic, b: dynamic, args: dynamic...)
{
  return if ((a < b)) max(b, cpp_expand(args)) else max(a, cpp_expand(args));
}

func read_str(s: dynamic)
{
  while ((((ch == cpp_char(" ")) || (ch == cpp_char("\r"))) || (ch == cpp_char("\n"))))
  {
    ch = getchar();
  }
  var tar = s;
  (*tar) = ch;
  ch = getchar();
  while (((((ch != cpp_char(" ")) && (ch != cpp_char("\r"))) && (ch != cpp_char("\n"))) && (ch != EOF)))
  {
    (*(cpp_update(tar, "++"))) = ch;
    ch = getchar();
  }
  return ((tar - s) + 1);
}

var N = 50005;

var MAXN = 100005;

var a = cpp_array(N);

var g = cpp_array(21, N);

var Log2 = cpp_array(N);

func query(l: dynamic, r: dynamic)
{
  var k = Log2[((r - l) + 1)];
  return gcd(g[l][k], g[((r - ((1 << k))) + 1)][k]);
}

func s(x: dynamic)
{
  return (((1 * x) * ((x + 1))) >> 1);
}

func f(n: dynamic, a: dynamic, b: dynamic, c: dynamic)
{
  if ((!a))
  {
    return (((n + 1)) * ((b / c)));
  }
  if (((((a < 0) || (b < 0)) || (a >= c)) || (b >= c)))
  {
    var A = (a % c);
    var B = (b % c);
    (((A < 0)) && (cpp_assign(A, "+=", c)));
    (((B < 0)) && (cpp_assign(B, "+=", c)));
    return ((f(n, A, B, c) + (((((a - A)) / c)) * s(n))) + (((((b - B)) / c)) * ((n + 1))));
  }
  var m = ((((a * n) + b)) / c);
  return ((n * m) - f((m - 1), c, ((c - b) - 1), a));
}

var cnt = cpp_array(MAXN);

var val = cpp_array(MAXN);

func calc(mid: dynamic)
{
  var ans = 0;
  var sum = 0;
  var qwq = 0;
  var pos = 100000;
  {
    var i = 100000;
    while ((i >= 1))
    {
      if ((!cnt[i]))
      {
        i -= 1;
        continue;
      }
      var l = 1;
      while ((l <= cnt[i]))
      {
        while ((((sum + ((1 * l) * i)) > mid) && (pos > i)))
        {
          sum -= (cnt[pos] * pos);
          qwq -= cnt[pos];
          pos -= 1;
        }
        if (((pos > i) || (((pos == i) && (((1 * l) * i) <= mid)))))
        {
          var r = min((((mid - sum)) / i), cpp_cast(cnt[i]));
          ans += (((+s(r)) - s((l - 1))) + ((1 * (((r - l) + 1))) * qwq));
          if (cnt[(pos + 1)])
          {
            ans += f((r - l), (-i), ((mid - sum) - ((1 * i) * l)), (pos + 1));
          }
          l = (r + 1);
        } else
        {
          var tmp = (mid / i);
          if ((l >= tmp))
          {
            ans += ((((cnt[i] - l) + 1)) * tmp);
          } else
          {
            chmin(tmp, cnt[i]);
            ans += (s(tmp) - s((l - 1)));
            ans += (((cnt[i] - tmp)) * tmp);
          }
          l = (cnt[i] + 1);
        }
      }
      sum += (cnt[i] * i);
      qwq += cnt[i];
      i -= 1;
    }
  }
  return ans;
}

func main()
{
  {
    var i = 2;
    while ((i < N))
    {
      Log2[i] = (Log2[(i >> 1)] + 1);
      i += 1;
    }
  }
  var n: dynamic;
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i = n;
    while ((i >= 1))
    {
      g[i][0] = a[i];
      {
        var j = 1;
        while (((j <= 20) && ((i + ((1 << ((j - 1))))) <= n)))
        {
          g[i][j] = gcd(g[i][(j - 1)], g[(i + ((1 << ((j - 1)))))][(j - 1)]);
          j += 1;
        }
      }
      i -= 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      var cur = i;
      while ((cur <= n))
      {
        var l = cur;
        var r = n;
        var tmp = query(i, cur);
        while ((l < r))
        {
          var mid = ((((l + r) + 1)) >> 1);
          if ((query(i, mid) == tmp))
          {
            l = mid;
          } else
          {
            r = (mid - 1);
          }
        }
        cnt[tmp] += ((l - cur) + 1);
        cur = (l + 1);
      }
      i += 1;
    }
  }
  var cnt = ((((((s(n) * ((s(n) + 1))) / 2)) + 1)) / 2);
  var l = 1;
  var r = 1e18;
  while ((l < r))
  {
    var mid = (((l + r)) >> 1);
    if ((calc(mid) >= cnt))
    {
      r = mid;
    } else
    {
      l = (mid + 1);
    }
  }
  printf("%lld\n", l);
  return 0;
}
