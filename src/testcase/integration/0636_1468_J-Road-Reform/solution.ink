// Translated from solution.cpp.

var startt: dynamic = cpp_expression("#include <bits/stdc++.h> #define");

var vint: dynamic = cpp_expression("#include <b");

func all(v: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/std");
}

var MOD: dynamic = cpp_expression("#include <");

var MOD2: dynamic = cpp_expression("#include");

var MX: dynamic = cpp_expression("#include <");

var MXL: dynamic = cpp_expression("#include <bits/stdc");

var PI: dynamic = cpp_expression("#include <");

var pb: dynamic = cpp_expression("#include");

var sc: dynamic = cpp_expression("#inclu");

var fr: dynamic = cpp_expression("#incl");

var int_cpp: dynamic = cpp_expression("#i");

var endl: dynamic = cpp_expression("#inc");

var ld: dynamic = dynamic;

func ceildiv(one: dynamic, two: dynamic) -> dynamic
{
  if (((one % two) == 0))
  {
    return (one / two);
  } else
  {
    return ((one / two) + 1);
  }
}

func power(n: dynamic, pow: dynamic, m: dynamic) -> dynamic
{
  if ((pow == 0))
  {
    return 1;
  }
  if (((pow % 2) == 0))
  {
    var x: dynamic = power(n, (pow / 2), m);
    return (((x * x)) % m);
  } else
  {
    return (((power(n, (pow - 1), m) * n)) % m);
  }
}

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  if ((!b))
  {
    return a;
  }
  return gcd(b, (a % b));
}

func factorial(n: dynamic, mod: dynamic) -> dynamic
{
  if ((n > 1))
  {
    return (((n * factorial((n - 1), mod))) % mod);
  } else
  {
    return 1;
  }
}

func lcm(a: dynamic, b: dynamic) -> dynamic
{
  return (((a * b)) / gcd(a, b));
}

func read(n: dynamic) -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var x: dynamic = cpp_uninitialized();
      read(x);
      a.pb(x);
      i += 1;
    }
  }
  return a;
}

var adj: dynamic = cpp_uninitialized();

func init(n: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i <= n))
    {
      var a: dynamic = cpp_uninitialized();
      adj.pb(a);
      i += 1;
    }
  }
}

var MAXARR: dynamic = 200005;

var id: dynamic = cpp_array(MAXARR);

var edges: dynamic = cpp_uninitialized();

var p: dynamic = cpp_array(MAXARR);

func initialize(n: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i <= n))
    {
      id[i] = i;
      i += 1;
    }
  }
}

func root(x: dynamic) -> dynamic
{
  while ((id[x] != x))
  {
    id[x] = id[id[x]];
    x = id[x];
  }
  return x;
}

func union1(x: dynamic, y: dynamic) -> dynamic
{
  var p: dynamic = root(x);
  var q: dynamic = root(y);
  id[p] = id[q];
}

func kruskal(p: dynamic) -> dynamic
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var cost: dynamic = cpp_uninitialized();
  var minimumCost: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < edges))
    {
      x = p[i].second.first;
      y = p[i].second.second;
      cost = p[i].first;
      if ((root(x) != root(y)))
      {
        minimumCost += cost;
        union1(x, y);
      }
      i += 1;
    }
  }
  return minimumCost;
}

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, m, k);
  initialize(n);
  edges = m;
  var best: dynamic = MXL;
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      var w: dynamic = cpp_uninitialized();
      read(u, v, w);
      best = min(best, abs((w - k)));
      p[i] = make_pair(max(0, (w - k)), make_pair(u, v));
      i += 1;
    }
  }
  sort(p, (p + m));
  var minimumcost: dynamic = kruskal(p);
  if ((minimumcost == 0))
  {
    write(best, "\n");
  } else
  {
    write(minimumcost, "\n");
  }
}

func main() -> dynamic
{
  startt;
  var t: dynamic = 1;
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
}
