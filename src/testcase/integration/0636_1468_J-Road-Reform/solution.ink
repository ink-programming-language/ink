// Translated from solution.cpp.

var startt = cpp_expression("#include <bits/stdc++.h> #define");

var vint = cpp_expression("#include <b");

func all(v: dynamic)
{
  return cpp_expression("#include <bits/std");
}

var MOD = cpp_expression("#include <");

var MOD2 = cpp_expression("#include");

var MX = cpp_expression("#include <");

var MXL = cpp_expression("#include <bits/stdc");

var PI = cpp_expression("#include <");

var pb = cpp_expression("#include");

var sc = cpp_expression("#inclu");

var fr = cpp_expression("#incl");

var int_cpp = cpp_expression("#i");

var endl = cpp_expression("#inc");

var ld = dynamic;

func ceildiv(one: dynamic, two: dynamic)
{
  if (((one % two) == 0))
  {
    return (one / two);
  } else
  {
    return ((one / two) + 1);
  }
}

func power(n: dynamic, pow: dynamic, m: dynamic)
{
  if ((pow == 0))
  {
    return 1;
  }
  if (((pow % 2) == 0))
  {
    var x = power(n, (pow / 2), m);
    return (((x * x)) % m);
  } else
  {
    return (((power(n, (pow - 1), m) * n)) % m);
  }
}

func gcd(a: dynamic, b: dynamic)
{
  if ((!b))
  {
    return a;
  }
  return gcd(b, (a % b));
}

func factorial(n: dynamic, mod: dynamic)
{
  if ((n > 1))
  {
    return (((n * factorial((n - 1), mod))) % mod);
  } else
  {
    return 1;
  }
}

func lcm(a: dynamic, b: dynamic)
{
  return (((a * b)) / gcd(a, b));
}

func read(n: dynamic)
{
  var a: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      var x: dynamic;
      read(x);
      a.pb(x);
      i += 1;
    }
  }
  return a;
}

var adj: dynamic;

func init(n: dynamic)
{
  {
    var i = 0;
    while ((i <= n))
    {
      var a: dynamic;
      adj.pb(a);
      i += 1;
    }
  }
}

var MAXARR = 200005;

var id = cpp_array(MAXARR);

var edges: dynamic;

var p = cpp_array(MAXARR);

func initialize(n: dynamic)
{
  {
    var i = 0;
    while ((i <= n))
    {
      id[i] = i;
      i += 1;
    }
  }
}

func root(x: dynamic)
{
  while ((id[x] != x))
  {
    id[x] = id[id[x]];
    x = id[x];
  }
  return x;
}

func union1(x: dynamic, y: dynamic)
{
  var p = root(x);
  var q = root(y);
  id[p] = id[q];
}

func kruskal(p: dynamic)
{
  var x: dynamic;
  var y: dynamic;
  var cost: dynamic;
  var minimumCost = 0;
  {
    var i = 0;
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

func solve()
{
  var n: dynamic;
  var m: dynamic;
  var k: dynamic;
  read(n, m, k);
  initialize(n);
  edges = m;
  var best = MXL;
  {
    var i = 0;
    while ((i < m))
    {
      var u: dynamic;
      var v: dynamic;
      var w: dynamic;
      read(u, v, w);
      best = min(best, abs((w - k)));
      p[i] = make_pair(max(0, (w - k)), make_pair(u, v));
      i += 1;
    }
  }
  sort(p, (p + m));
  var minimumcost = kruskal(p);
  if ((minimumcost == 0))
  {
    write(best, "\n");
  } else
  {
    write(minimumcost, "\n");
  }
}

func main()
{
  startt;
  var t = 1;
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
}
