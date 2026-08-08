// Translated from solution.cpp.

var L = (((1 << 20)) + 1);

var buf = cpp_array(L);

var S: dynamic;

var T: dynamic;

var c: dynamic;

func getchar()
{
  if (builtin_expect((S == T), 0))
  {
    T = ((cpp_assign(S, "=", buf)) + fread(buf, 1, L, stdin));
    return (if ((S == T)) EOF else (*cpp_update(S, "++")));
  }
  return (*cpp_update(S, "++"));
}

func inp()
{
  var x = 0;
  var f = 1;
  var ch: dynamic;
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

func inpu()
{
  var x = 0;
  var ch: dynamic;
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

func inp_ll()
{
  var x = 0;
  var f = 1;
  var ch: dynamic;
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

var B = cpp_array(25);

var outs = (B + 20);

var outr = (B + 20);

func print(a: dynamic, x: dynamic = 0)
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

func __cpp_top_level_1()
{
}

func power(a: dynamic, b: dynamic, p: dynamic)
{
  if ((!b))
  {
    return 1;
  }
  var t = power(a, (b / 2), p);
  t = ((t * t) % p);
  if ((b & 1))
  {
    t = ((t * a) % p);
  }
  return t;
}

func exgcd(a: dynamic, b: dynamic, x: dynamic, y: dynamic)
{
  if ((b == 0))
  {
    x = 1;
    y = 0;
    return a;
  }
  var px: dynamic;
  var py: dynamic;
  var d = exgcd(b, (a % b), px, py);
  x = py;
  y = (px - ((a / b) * py));
  return d;
}

func freshmin(a: dynamic, b: dynamic)
{
  if ((a > b))
  {
    a = b;
  }
}

func freshmax(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
  }
}

var MAXN = 200010;

var MAXP = 200000;

var MAXK = 110;

var MOD = 1000000009;

var MI = (f80(1) / MOD);

var INF = 1000000000;

class mybitset
{
  var LIMIT: dynamic;
  var a: dynamic = cpp_array(LIMIT);
  func mybitset()
  {
      memset(a, 0, cpp_sizeof((a)));
    }
  func set(x: dynamic)
  {
      a[(x >> 6)] |= (1 << ((x & 63)));
    }
  func reset(x: dynamic)
  {
      a[(x >> 6)] &= (0xffffffffffffffff ^ ((1 << ((x & 63)))));
    }
  func test(x: dynamic)
  {
      return ((a[(x >> 6)] >> ((x & 63))) & 1);
    }
  func orshl(v: dynamic, shift: dynamic)
  {
      if ((((shift & 63)) == 0))
      {
        {
          var i = 0;
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
          var i = 1;
          while ((i < (LIMIT - ((shift >> 6)))))
          {
            a[(((shift >> 6)) + i)] |= (((v.a[(i - 1)] >> ((64 - shift)))) | ((v.a[i] << shift)));
            i += 1;
          }
        }
      }
    }
}

var b = cpp_array(MAXN);

var u: dynamic;

var v = cpp_array(MAXK);

var f = cpp_array(MAXN);

func init(x: dynamic)
{
  {
    var i = 2;
    while ((i <= MAXP))
    {
      if ((!b[i]))
      {
        u.set(i);
        {
          var j = (i + i);
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
    var i = 1;
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

func main()
{
  var n = inp();
  init(inp());
  var ans = 0;
  while (cpp_update(n, "--"))
  {
    var a = inp();
    var b = inp();
    var c = inp();
    ans ^= (f[((b - a) - 1)] ^ f[((c - b) - 1)]);
  }
  puts(if (ans) "Alice\nBob" else "Bob\nAlice");
  return 0;
}
