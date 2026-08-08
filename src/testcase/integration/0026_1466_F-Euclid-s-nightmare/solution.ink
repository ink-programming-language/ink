// Translated from solution.cpp.

var fst = cpp_expression("#incl");

var snd = cpp_expression("#inclu");

func fore(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=a,ThxDem=b;i<ThxDem;++i)");
}

var pb = cpp_expression("#include");

func ALL(s: dynamic)
{
  return cpp_expression("#include <bits/st");
}

var FIN = cpp_expression("#include <bits/stdc++.h> #def");

func SZ(s: dynamic)
{
  return cpp_expression("#include <bit");
}

var MAXN = (5e5 + 10);

var MOD = (1e9 + 7);

func add(a: dynamic, b: dynamic)
{
  a += b;
  if ((a >= MOD))
  {
    a -= MOD;
  }
  return a;
}

func sub(a: dynamic, b: dynamic)
{
  a -= b;
  if ((a < 0))
  {
    a += MOD;
  }
  return a;
}

func mul(a: dynamic, b: dynamic)
{
  return ((a * b) % MOD);
}

func fpow(a: dynamic, b: dynamic)
{
  var r = 1;
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

var p = cpp_array(MAXN);

var am = cpp_array(MAXN);

var did = cpp_array(MAXN);

func find(x: dynamic)
{
  return cpp_assign(p[x], "=", if ((p[x] == x)) x else find(p[x]));
}

func join(x: dynamic, y: dynamic)
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

func main()
{
  FIN;
  var n: dynamic;
  var m: dynamic;
  var tot = 0;
  read(n, m);
  fore(i, 0, m)[i] = i;
  fore(i, 0, n);
  {
    var k: dynamic;
    read(k);
    if ((k == 1))
    {
      var x: dynamic;
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
      var x: dynamic;
      var y: dynamic;
      read(x, y);
      x -= 1;
      y -= 1;
      var has = (am[find(x)] && am[find(y)]);
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
