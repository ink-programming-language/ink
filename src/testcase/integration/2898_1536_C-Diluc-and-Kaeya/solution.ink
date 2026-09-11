// Translated from solution.cpp.

var ll: dynamic = dynamic;

var int_cpp: dynamic = dynamic;

var endl: dynamic = cpp_expression("#inc");

func rep(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=a;i<=b;i++)");
}

var pll: dynamic = cpp_expression("#include<bi");

var pii: dynamic = cpp_expression("#include<bits");

var vpll: dynamic = cpp_expression("#include<bi");

func SZ(x: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/s");
}

var FIO: dynamic = cpp_expression("#include<bits/stdc++.h> #defi");

func watch(x: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++.h> #defi");
}

func watch2(x: dynamic, y: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++.h> #define ll long long #define int");
}

var pb: dynamic = cpp_expression("#include<");

var pf: dynamic = cpp_expression("#include<b");

var ff: dynamic = cpp_expression("#incl");

var ss: dynamic = cpp_expression("#inclu");

var mod: dynamic = cpp_expression("#include<b");

var INF: dynamic = cpp_expression("#include<b");

func all(c: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++.");
}

func power(a: dynamic, b: dynamic) -> dynamic
{
  var res: dynamic = 1;
  a = (a % mod);
  while ((b > 0))
  {
    if ((b & 1))
    {
      res = (((res * a)) % mod);
      b -= 1;
    }
    a = (((a * a)) % mod);
    b >>= 1;
  }
  return res;
}

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  return  (((b == 0))) ? a : gcd(b, (a % b));
}

var mxn: dynamic = 2e5;

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  read(n);
  var s: dynamic = cpp_uninitialized();
  read(s);
  var d: dynamic = 0;
  var k: dynamic = 0;
  var rat: dynamic = cpp_uninitialized();
  var res: dynamic = cpp_uninitialized();
  rep(i, 0, (n - 1));
  {
    if ((s[i] == cpp_char("D")))
    {
      d += 1;
    } else
    {
      k += 1;
    }
    var gc: dynamic = gcd(d, k);
    x = (d / gc);
    y = (k / gc);
    if ((rat.find([x, y]) == rat.end()))
    {
      res.pb(1);
    } else
    {
      var cnt: dynamic = (rat[[x, y]] + 1);
      res.pb(cnt);
    }
    if ((x == 0))
    {
      rat[[0, 1]] += 1;
    } else if ((y == 0))
    {
      rat[[1, 0]] += 1;
    } else
    {
      rat[[x, y]] += 1;
    }
  }
  for (var it: dynamic in res)
  {
    write(it, " ");
  }
  write("\n");
}

func main() -> dynamic
{
  FIO;
  var T: dynamic = 1;
  read(T);
  while (cpp_update(T, "--"))
  {
    solve();
  }
  return 0;
}
