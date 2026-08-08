// Translated from solution.cpp.

var i_7 = cpp_expression("#include <b");

var i_5 = cpp_expression("#incl");

func mod(a: dynamic)
{
  var c = (a % i_7);
  if ((c >= 0))
  {
    return c;
  }
  return (c + i_7);
}

var inf = cpp_cast(1E16);

func rep(i: dynamic, l: dynamic, r: dynamic)
{
  cpp_macro("for(ll i=l;i<=r;i++)");
}

var pb = cpp_expression("#include");

func max(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    return b;
  } else
  {
    return a;
  }
}

func min(a: dynamic, b: dynamic)
{
  if ((a > b))
  {
    return b;
  } else
  {
    return a;
  }
}

func Max(pos: dynamic, val: dynamic)
{
  pos = max(pos, val);
}

func Min(pos: dynamic, val: dynamic)
{
  pos = min(pos, val);
}

func Add(pos: dynamic, val: dynamic)
{
  pos = mod((pos + val));
}

var EPS = 1E-9;

var fastio = cpp_expression("#include <bits/stdc++.h> usi");

func main()
{
  var ll: dynamic;
  var k: dynamic;
  read(n, k);
  var a = cpp_array(n);
  rep(i, 0, (n - 1));
  read(a[i]);
  var sum = cpp_array((1 << n));
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
  var f = cpp_array((1 << n));
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
  var ans = 0;
  rep(i, 0, (((1 << n)) - 1));
  {
    if ((!f[i]))
    {
      var c = 0;
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
