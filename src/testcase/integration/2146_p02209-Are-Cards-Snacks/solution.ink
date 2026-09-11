// Translated from solution.cpp.

var i_7: dynamic = cpp_expression("#include <b");

var i_5: dynamic = cpp_expression("#incl");

func mod(a: dynamic) -> dynamic
{
  var c: dynamic = (a % i_7);
  if ((c >= 0))
  {
    return c;
  }
  return (c + i_7);
}

var inf: dynamic = cpp_cast(1E16);

func rep(i: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  cpp_macro("for(ll i=l;i<=r;i++)");
}

var pb: dynamic = cpp_expression("#include");

func max(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    return b;
  } else
  {
    return a;
  }
}

func min(a: dynamic, b: dynamic) -> dynamic
{
  if ((a > b))
  {
    return b;
  } else
  {
    return a;
  }
}

func Max(pos: dynamic, val: dynamic) -> dynamic
{
  pos = max(pos, val);
}

func Min(pos: dynamic, val: dynamic) -> dynamic
{
  pos = min(pos, val);
}

func Add(pos: dynamic, val: dynamic) -> dynamic
{
  pos = mod((pos + val));
}

var EPS: dynamic = 1E-9;

var fastio: dynamic = cpp_expression("#include <bits/stdc++.h> usi");

func main() -> dynamic
{
  var ll: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  var a: dynamic = cpp_array(n);
  rep(i, 0, (n - 1));
  read(a[i]);
  var sum: dynamic = cpp_array((1 << n));
  memset(sum, 0, cpp_sizeof((sum)));
  rep(i, 0, (((1 << n)) - 1));
  {
    rep(j, 0, (n - 1));
    {
      if ((((i >> j)) & 1))
      {
        sum[i] += a[j];
      }
    }
  }
  var f: dynamic = cpp_array((1 << n));
  memset(f, false, cpp_sizeof((f)));
  rep(i, 0, (((1 << n)) - 1));
  {
    if ((sum[i] == k))
    {
      f[i] = true;
    }
  }
  rep(i, 0, (((1 << n)) - 1));
  {
    if (f[i])
    {
      rep(j, 0, (n - 1));
      {
        f[(i | ((1 << j)))] = true;
      }
    }
  }
  var ans: dynamic = 0;
  rep(i, 0, (((1 << n)) - 1));
  {
    if ((!f[i]))
    {
      var c: dynamic = 0;
      rep(j, 0, (n - 1));
      {
        if ((((i >> j)) & 1))
        {
          c += 1;
        }
      }
      Max(ans, c);
    }
  }
  write((n - ans), "\n");
  return 0;
}
