// Translated from solution.cpp.

var L: dynamic = (((1 << 20)) + 1);

var buf: dynamic = cpp_array(L);

var S: dynamic = cpp_uninitialized();

var T: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

func getchar() -> dynamic
{
  if (builtin_expect((S == T), 0))
  {
    T = ((cpp_assign(S, "=", buf)) + fread(buf, 1, L, stdin));
    return ( ((S == T)) ? EOF : (*cpp_update(S, "++")));
  }
  return (*cpp_update(S, "++"));
}

func inp() -> dynamic
{
  var x: dynamic = 0;
  var f: dynamic = 1;
  var ch: dynamic = cpp_uninitialized();
  {
    ch = getchar();
    while ((!isdigit(ch)))
    {
      if ((ch == cpp_char("-")))
      {
        f = -1;
      }
      ch = getchar();
    }
  }
  {
    while (isdigit(ch))
    {
      x = (((x * 10) + ch) - cpp_char("0"));
      ch = getchar();
    }
  }
  return (x * f);
}

func inpu() -> dynamic
{
  var x: dynamic = 0;
  var ch: dynamic = cpp_uninitialized();
  {
    ch = getchar();
    while ((!isdigit(ch)))
    {
      ch = getchar();
    }
  }
  {
    while (isdigit(ch))
    {
      x = (((x * 10) + ch) - cpp_char("0"));
      ch = getchar();
    }
  }
  return x;
}

func inp_ll() -> dynamic
{
  var x: dynamic = 0;
  var f: dynamic = 1;
  var ch: dynamic = cpp_uninitialized();
  {
    ch = getchar();
    while ((!isdigit(ch)))
    {
      if ((ch == cpp_char("-")))
      {
        f = -1;
      }
      ch = getchar();
    }
  }
  {
    while (isdigit(ch))
    {
      x = (((x * 10) + ch) - cpp_char("0"));
      ch = getchar();
    }
  }
  return (x * f);
}

var B: dynamic = cpp_array(25);

var outs: dynamic = (B + 20);

var outr: dynamic = (B + 20);

func print(a: dynamic, x: dynamic = 0) -> dynamic
{
  if (x)
  {
    (*cpp_update(outs, "--")) = x;
    x = 0;
  }
  if ((!a))
  {
    (*cpp_update(outs, "--")) = cpp_char("0");
  } else
  {
    while (a)
    {
      (*cpp_update(outs, "--")) = (((a % 10)) + 48);
      a /= 10;
    }
  }
  if (x)
  {
    (*cpp_update(outs, "--")) = x;
  }
  fwrite(outs, (outr - outs), 1, stdout);
  outs = outr;
}

func __cpp_top_level_1() -> dynamic
{
}

func power(a: dynamic, b: dynamic, p: dynamic) -> dynamic
{
  if ((!b))
  {
    return 1;
  }
  var t: dynamic = power(a, (b / 2), p);
  t = ((t * t) % p);
  if ((b & 1))
  {
    t = ((t * a) % p);
  }
  return t;
}

func exgcd(a: dynamic, b: dynamic, x: dynamic, y: dynamic) -> dynamic
{
  if ((b == 0))
  {
    x = 1;
    y = 0;
    return a;
  }
  var px: dynamic = cpp_uninitialized();
  var py: dynamic = cpp_uninitialized();
  var d: dynamic = exgcd(b, (a % b), px, py);
  x = py;
  y = (px - ((a / b) * py));
  return d;
}

func freshmin(a: dynamic, b: dynamic) -> dynamic
{
  if ((a > b))
  {
    a = b;
  }
}

func freshmax(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    a = b;
  }
}

var MAXN: dynamic = 200010;

var MAXP: dynamic = 200000;

var MAXK: dynamic = 110;

var MOD: dynamic = 1000000009;

var MI: dynamic = (f80(1) / MOD);

var INF: dynamic = 1000000000;

class mybitset
{
  var LIMIT: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_array(LIMIT);
  func mybitset() -> dynamic
  {
      memset(a, 0, cpp_sizeof((a)));
    }
  func set(x: dynamic) -> dynamic
  {
      a[(x >> 6)] |= (1 << ((x & 63)));
    }
  func reset(x: dynamic) -> dynamic
  {
      a[(x >> 6)] &= (0xffffffffffffffff ^ ((1 << ((x & 63)))));
    }
  func test(x: dynamic) -> dynamic
  {
      return ((a[(x >> 6)] >> ((x & 63))) & 1);
    }
  func orshl(v: dynamic, shift: dynamic) -> dynamic
  {
      if ((((shift & 63)) == 0))
      {
        {
          var i: dynamic = 0;
          while ((i < (LIMIT - ((shift >> 6)))))
          {
            a[(((shift >> 6)) + i)] |= v.a[i];
            i += 1;
          }
        }
      } else
      {
        a[(shift >> 6)] |= (v.a[0] << shift);
        {
          var i: dynamic = 1;
          while ((i < (LIMIT - ((shift >> 6)))))
          {
            a[(((shift >> 6)) + i)] |= (((v.a[(i - 1)] >> ((64 - shift)))) | ((v.a[i] << shift)));
            i += 1;
          }
        }
      }
    }
}

var b: dynamic = cpp_array(MAXN);

var u: dynamic = cpp_uninitialized();

var v: dynamic = cpp_array(MAXK);

var f: dynamic = cpp_array(MAXN);

func init(x: dynamic) -> dynamic
{
  {
    var i: dynamic = 2;
    while ((i <= MAXP))
    {
      if ((!b[i]))
      {
        u.set(i);
        {
          var j: dynamic = (i + i);
          while ((j <= MAXP))
          {
            b[j] = i;
            j += i;
          }
        }
      } else if ((!b[(i / b[i])]))
      {
        u.set(i);
      }
      i += 1;
    }
  }
  u.reset(x);
  v[0] = u;
  {
    var i: dynamic = 1;
    while ((i <= MAXP))
    {
      {
        while (v[f[i]].test(i))
        {
          f[i] += 1;
        }
      }
      v[f[i]].orshl(u, i);
      i += 1;
    }
  }
}

func main() -> dynamic
{
  var n: dynamic = inp();
  init(inp());
  var ans: dynamic = 0;
  while (cpp_update(n, "--"))
  {
    var a: dynamic = inp();
    var b: dynamic = inp();
    var c: dynamic = inp();
    ans ^= (f[((b - a) - 1)] ^ f[((c - b) - 1)]);
  }
  puts( (ans) ? "Alice\nBob" : "Bob\nAlice");
  return 0;
}
