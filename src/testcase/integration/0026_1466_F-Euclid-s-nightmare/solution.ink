// Translated from solution.cpp.

var fst: dynamic = cpp_expression("#incl");

var snd: dynamic = cpp_expression("#inclu");

func fore(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=a,ThxDem=b;i<ThxDem;++i)");
}

var pb: dynamic = cpp_expression("#include");

func ALL(s: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/st");
}

var FIN: dynamic = cpp_expression("#include <bits/stdc++.h> #def");

func SZ(s: dynamic) -> dynamic
{
  return cpp_expression("#include <bit");
}

var MAXN: dynamic = (5e5 + 10);

var MOD: dynamic = (1e9 + 7);

func add(a: dynamic, b: dynamic) -> dynamic
{
  a += b;
  if ((a >= MOD))
  {
    a -= MOD;
  }
  return a;
}

func sub(a: dynamic, b: dynamic) -> dynamic
{
  a -= b;
  if ((a < 0))
  {
    a += MOD;
  }
  return a;
}

func mul(a: dynamic, b: dynamic) -> dynamic
{
  return ((a * b) % MOD);
}

func fpow(a: dynamic, b: dynamic) -> dynamic
{
  var r: dynamic = 1;
  while (b)
  {
    if ((b & 1))
    {
      r = mul(r, a);
    }
    b >>= 1;
    a = mul(a, a);
  }
  return r;
}

var p: dynamic = cpp_array(MAXN);

var am: dynamic = cpp_array(MAXN);

var did: dynamic = cpp_array(MAXN);

func find(x: dynamic) -> dynamic
{
  return cpp_assign(p[x], "=",  ((p[x] == x)) ? x : find(p[x]));
}

func join(x: dynamic, y: dynamic) -> dynamic
{
  x = find(x);
  y = find(y);
  if ((x == y))
  {
    return 0;
  }
  if ((x != y))
  {
    p[x] = y;
    am[y] |= am[x];
  }
  return 1;
}

func main() -> dynamic
{
  FIN;
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var tot: dynamic = 0;
  read(n, m);
  fore(i, 0, m)[i] = i;
  fore(i, 0, n);
  {
    var k: dynamic = cpp_uninitialized();
    read(k);
    if ((k == 1))
    {
      var x: dynamic = cpp_uninitialized();
      read(x);
      x -= 1;
      if ((!am[find(x)]))
      {
        am[find(x)] = 1;
        did[i] = 1;
        tot += 1;
      }
    }
    if ((k == 2))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      read(x, y);
      x -= 1;
      y -= 1;
      var has: dynamic = (am[find(x)] && am[find(y)]);
      if ((join(x, y) && (!has)))
      {
        did[i] = 1;
        tot += 1;
      }
    }
  }
  write(fpow(2, tot), " ", tot, "\n");
  fore(i, 0, n);
  if (did[i])
  {
    write((i + 1), " ");
  }
  write("\n");
}
