// Translated from solution.cpp.

var inf: dynamic = 0x3f3f3f3f;

var Inf: dynamic = 0x7fffffff;

var INF: dynamic = 0x3f3f3f3f3f3f3f3f;

func rnd() -> dynamic
{
  var seed: dynamic = 416;
  return cpp_comma(cpp_assign(seed, "+=", 0x71dad4bf), cpp_comma(cpp_assign(seed, "^=", (seed >> 5)), cpp_comma(cpp_assign(seed, "+=", 0xc6f74d88), cpp_comma(cpp_assign(seed, "^=", (seed << 17)), cpp_comma(cpp_assign(seed, "+=", 0x25e6561), cpp_assign(seed, "^=", (seed >> 13)))))));
}

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  return  (((!b))) ? a : gcd(b, (a % b));
}

func abs(a: dynamic) -> dynamic
{
  return  ((a >= 0)) ? a : (-a);
}

func chmax(a: dynamic, b: dynamic) -> dynamic
{
  (((a < b)) && (cpp_assign(a, "=", b)));
}

func chmin(a: dynamic, b: dynamic) -> dynamic
{
  (((b < a)) && (cpp_assign(a, "=", b)));
}

func read(x: dynamic) -> dynamic
{
  var f: dynamic = cpp_construct(false);
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

func read(t: dynamic, args: dynamic...) -> dynamic
{
  read(t);
  read(cpp_expand(args));
}

func min(a: dynamic, b: dynamic, args: dynamic...) -> dynamic
{
  return  ((a < b)) ? min(a, cpp_expand(args)) : min(b, cpp_expand(args));
}

func max(a: dynamic, b: dynamic, args: dynamic...) -> dynamic
{
  return  ((a < b)) ? max(b, cpp_expand(args)) : max(a, cpp_expand(args));
}

func read_str(s: dynamic) -> dynamic
{
  while ((((ch == cpp_char(" ")) || (ch == cpp_char("\r"))) || (ch == cpp_char("\n"))))
  {
    ch = getchar();
  }
  var tar: dynamic = s;
  (*tar) = ch;
  ch = getchar();
  while (((((ch != cpp_char(" ")) && (ch != cpp_char("\r"))) && (ch != cpp_char("\n"))) && (ch != EOF)))
  {
    (*(cpp_update(tar, "++"))) = ch;
    ch = getchar();
  }
  return ((tar - s) + 1);
}

var N: dynamic = 50005;

var MAXN: dynamic = 100005;

var a: dynamic = cpp_array(N);

var g: dynamic = cpp_array(21, N);

var Log2: dynamic = cpp_array(N);

func query(l: dynamic, r: dynamic) -> dynamic
{
  var k: dynamic = Log2[((r - l) + 1)];
  return gcd(g[l][k], g[((r - ((1 << k))) + 1)][k]);
}

func s(x: dynamic) -> dynamic
{
  return (((1 * x) * ((x + 1))) >> 1);
}

func f(n: dynamic, a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  if ((!a))
  {
    return (((n + 1)) * ((b / c)));
  }
  if (((((a < 0) || (b < 0)) || (a >= c)) || (b >= c)))
  {
    var A: dynamic = (a % c);
    var B: dynamic = (b % c);
    (((A < 0)) && (cpp_assign(A, "+=", c)));
    (((B < 0)) && (cpp_assign(B, "+=", c)));
    return ((f(n, A, B, c) + (((((a - A)) / c)) * s(n))) + (((((b - B)) / c)) * ((n + 1))));
  }
  var m: dynamic = ((((a * n) + b)) / c);
  return ((n * m) - f((m - 1), c, ((c - b) - 1), a));
}

var cnt: dynamic = cpp_array(MAXN);

var val: dynamic = cpp_array(MAXN);

func calc(mid: dynamic) -> dynamic
{
  var ans: dynamic = 0;
  var sum: dynamic = 0;
  var qwq: dynamic = 0;
  var pos: dynamic = 100000;
  {
    var i: dynamic = 100000;
    while ((i >= 1))
    {
      if ((!cnt[i]))
      {
        i -= 1;
        continue;
      }
      var l: dynamic = 1;
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
          var r: dynamic = min((((mid - sum)) / i), cpp_cast(cnt[i]));
          ans += (((+s(r)) - s((l - 1))) + ((1 * (((r - l) + 1))) * qwq));
          if (cnt[(pos + 1)])
          {
            ans += f((r - l), (-i), ((mid - sum) - ((1 * i) * l)), (pos + 1));
          }
          l = (r + 1);
        } else
        {
          var tmp: dynamic = (mid / i);
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

func main() -> dynamic
{
  {
    var i: dynamic = 2;
    while ((i < N))
    {
      Log2[i] = (Log2[(i >> 1)] + 1);
      i += 1;
    }
  }
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = n;
    while ((i >= 1))
    {
      g[i][0] = a[i];
      {
        var j: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= n))
    {
      var cur: dynamic = i;
      while ((cur <= n))
      {
        var l: dynamic = cur;
        var r: dynamic = n;
        var tmp: dynamic = query(i, cur);
        while ((l < r))
        {
          var mid: dynamic = ((((l + r) + 1)) >> 1);
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
  var cnt: dynamic = ((((((s(n) * ((s(n) + 1))) / 2)) + 1)) / 2);
  var l: dynamic = 1;
  var r: dynamic = 1e18;
  while ((l < r))
  {
    var mid: dynamic = (((l + r)) >> 1);
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
