// Translated from solution.cpp.

var EPS: dynamic = 1e-9;

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

func gao(a: dynamic, b: dynamic, xl: dynamic, xr: dynamic) -> dynamic
{
  if ((xl > xr))
  {
    return 0;
  }
  if (((((a * xl) + b) < 0) && (((a * xr) + b) < 0)))
  {
    return 0;
  }
  var key: dynamic = (((-b) * 1.0) / a);
  var keyx: dynamic = cpp_uninitialized();
  if ((a > 0))
  {
    keyx = ((key - EPS));
    keyx += 1;
  } else
  {
    keyx = ((key + EPS));
  }
  if ((((a * xl) + b) < 0))
  {
    xl = keyx;
  }
  if ((((a * xr) + b) < 0))
  {
    xr = keyx;
  }
  var ans: dynamic = (b * (((xr - xl) + 1)));
  var tmp: dynamic = ((((xr + xl)) * (((xr - xl) + 1))) / 2);
  ans += (tmp * a);
  return ans;
}

func first_old(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  var ans: dynamic = 0;
  var i1: dynamic = cpp_uninitialized();
  var i2: dynamic = cpp_uninitialized();
  var jl: dynamic = cpp_uninitialized();
  var jr: dynamic = cpp_uninitialized();
  var tmp: dynamic = cpp_uninitialized();
  var oldAns: dynamic = cpp_uninitialized();
  {
    var x: dynamic = 0;
    while ((x <= s))
    {
      tmp = 0;
      oldAns = ans;
      var keyi: dynamic = min(((a + x) - b), (s - x));
      if ((keyi < 0))
      {
        x += 1;
        continue;
      }
      i1 = ((((a + x) + 1) - b) - c);
      i2 = (((s - (2 * x)) - a) + c);
      i1 = min(keyi, i1);
      i2 = min(keyi, i2);
      if (((i1 < 0) && (i2 < 0)))
      {
        jl = 0;
        jr = keyi;
        ans += gao(-1, ((s - x) + 1), jl, jr);
      } else if ((i1 < 0))
      {
        jl = 0;
        jr = i2;
        ans += gao(0, (((a + x) - c) + 1), jl, jr);
        jl = (i2 + 1);
        jr = keyi;
        ans += gao(-1, ((s - x) + 1), jl, jr);
      } else if ((i2 < 0))
      {
        jl = 0;
        jr = i1;
        ans += gao(0, ((((((s - (2 * x)) - a) - 1) + b) + c) + 1), jl, jr);
        jl = (i1 + 1);
        jr = keyi;
        ans += gao(-1, ((s - x) + 1), jl, jr);
      } else
      {
        if ((i1 == i2))
        {
          jl = 0;
          jr = i1;
          ans += gao(1, ((b - 1) + 1), jl, jr);
          jl = (i1 + 1);
          jr = keyi;
          ans += gao(-1, ((s - x) + 1), jl, jr);
        } else if ((i1 < i2))
        {
          jl = 0;
          jr = i1;
          ans += gao(1, ((b - 1) + 1), jl, jr);
          jl = (i1 + 1);
          jr = i2;
          ans += gao(0, (((a + x) - c) + 1), jl, jr);
          jl = (i2 + 1);
          jr = keyi;
          ans += gao(-1, ((s - x) + 1), jl, jr);
        } else
        {
          jl = 0;
          jr = i2;
          ans += gao(1, ((b - 1) + 1), jl, jr);
          jl = (i2 + 1);
          jr = i1;
          ans += gao(0, ((((((s - (2 * x)) - a) - 1) + b) + c) + 1), jl, jr);
          jl = (i1 + 1);
          jr = keyi;
          ans += gao(-1, ((s - x) + 1), jl, jr);
        }
      }
      x += 1;
    }
  }
  return ans;
}

func first(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  var ans: dynamic = 0;
  {
    var x: dynamic = 0;
    while ((x <= s))
    {
      if (((b > (a + x)) || (c > (a + x))))
      {
        x += 1;
        continue;
      }
      var tmp: dynamic = (((((s - x) + 2)) * (((s - x) + 1))) / 2);
      if ((((s - x) - ((((a + x) + 1) - b))) >= 0))
      {
        tmp -= ((((((s - x) - ((((a + x) + 1) - b))) + 2)) * ((((s - x) - ((((a + x) + 1) - b))) + 1))) / 2);
      }
      if ((((s - x) - ((((a + x) + 1) - c))) >= 0))
      {
        tmp -= ((((((s - x) - ((((a + x) + 1) - c))) + 2)) * ((((s - x) - ((((a + x) + 1) - c))) + 1))) / 2);
      }
      if (((((s - x) - ((((a + x) + 1) - b))) - ((((a + x) + 1) - c))) >= 0))
      {
        tmp += (((((((s - x) - ((((a + x) + 1) - b))) - ((((a + x) + 1) - c))) + 2)) * (((((s - x) - ((((a + x) + 1) - b))) - ((((a + x) + 1) - c))) + 1))) / 2);
      }
      var sum: dynamic = (((a + x) - b) - c);
      if ((sum >= 0))
      {
        sum = min(sum, (s - x));
        tmp -= ((((sum + 2)) * ((sum + 1))) / 2);
      }
      ans += tmp;
      x += 1;
    }
  }
  return ans;
}

func second(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  var ans: dynamic = 0;
  {
    var i: dynamic = max(a, b);
    while (true)
    {
      if ((i < c))
      {
        i += 1;
        continue;
      }
      var t: dynamic = (((i - a) + i) - b);
      t = (s - t);
      if ((t < 0))
      {
        break;
      }
      if (((c + t) > i))
      {
        ans += ((i - c) + 1);
      } else
      {
        ans += (t + 1);
      }
      i += 1;
    }
  }
  return ans;
}

func third(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  var ans: dynamic = 0;
  var t: dynamic = max(a, b);
  t = max(t, c);
  {
    var i: dynamic = t;
    while (true)
    {
      var used: dynamic = ((((3 * i) - a) - b) - c);
      if ((used > s))
      {
        break;
      }
      ans += 1;
      i += 1;
    }
  }
  return ans;
}

func solve() -> dynamic
{
  var ans: dynamic = 0;
  ans += first(a, b, c);
  ans += first(b, a, c);
  ans += first(c, a, b);
  ans -= second(a, b, c);
  ans -= second(b, c, a);
  ans -= second(a, c, b);
  ans += third(a, b, c);
  printf("%I64d\n", ans);
}

func main() -> dynamic
{
  while ((scanf("%I64d%I64d%I64d%I64d", (&a), (&b), (&c), (&s)) != EOF))
  {
    solve();
  }
  return 0;
}
