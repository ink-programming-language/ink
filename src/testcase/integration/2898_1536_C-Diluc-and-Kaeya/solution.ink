// Translated from solution.cpp.

var ll = dynamic;

var int_cpp = dynamic;

var endl = cpp_expression("#inc");

func rep(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=a;i<=b;i++)");
}

var pll = cpp_expression("#include<bi");

var pii = cpp_expression("#include<bits");

var vpll = cpp_expression("#include<bi");

func SZ(x: dynamic)
{
  return cpp_expression("#include<bits/s");
}

var FIO = cpp_expression("#include<bits/stdc++.h> #defi");

func watch(x: dynamic)
{
  return cpp_expression("#include<bits/stdc++.h> #defi");
}

func watch2(x: dynamic, y: dynamic)
{
  return cpp_expression("#include<bits/stdc++.h> #define ll long long #define int");
}

var pb = cpp_expression("#include<");

var pf = cpp_expression("#include<b");

var ff = cpp_expression("#incl");

var ss = cpp_expression("#inclu");

var mod = cpp_expression("#include<b");

var INF = cpp_expression("#include<b");

func all(c: dynamic)
{
  return cpp_expression("#include<bits/stdc++.");
}

func power(a: dynamic, b: dynamic)
{
  var res = 1;
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

func gcd(a: dynamic, b: dynamic)
{
  return if (((b == 0))) a else gcd(b, (a % b));
}

var mxn = 2e5;

func solve()
{
  var n: dynamic;
  var x: dynamic;
  var y: dynamic;
  read(n);
  var s: dynamic;
  read(s);
  var d = 0;
  var k = 0;
  var rat: dynamic;
  var res: dynamic;
  rep(i, 0, (n - 1));
  {
    if ((s[i] == cpp_char("D")))
    {
      d += 1;
    } else
    {
      k += 1;
    }
    var gc = gcd(d, k);
    x = (d / gc);
    y = (k / gc);
    if ((rat.find([x, y]) == rat.end()))
    {
      res.pb(1);
    } else
    {
      var cnt = (rat[[x, y]] + 1);
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
  for (var it in res)
  {
    write(it, " ");
  }
  write("\n");
}

func main()
{
  FIO;
  var T = 1;
  read(T);
  while (cpp_update(T, "--"))
  {
    solve();
  }
  return 0;
}
