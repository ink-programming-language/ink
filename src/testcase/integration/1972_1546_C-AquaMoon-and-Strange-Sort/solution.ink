// Translated from solution.cpp.

func amax(a: dynamic, b: dynamic) -> dynamic
{
  if ((b > a))
  {
    a = b;
  }
  return a;
}

func amin(a: dynamic, b: dynamic) -> dynamic
{
  if ((b < a))
  {
    a = b;
  }
  return a;
}

var ll: dynamic = dynamic;

var ld: dynamic = dynamic;

var INF: dynamic = cpp_expression("#include<b");

var INFL: dynamic = cpp_expression("#include<b");

var md: dynamic = cpp_expression("#include<b");

var mk: dynamic = cpp_expression("#include<");

var pi: dynamic = cpp_expression("#include<bits");

var ss: dynamic = cpp_expression("#inclu");

var ff: dynamic = cpp_expression("#incl");

var pb: dynamic = cpp_expression("#include<");

var eb: dynamic = cpp_expression("#include<bit");

func ppcl(x: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++.h>");
}

func ppc(x: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++.");
}

func maxi(a: dynamic) -> dynamic
{
  cpp_macro("*max_element(a.begin(),a.end());");
}

func mini(a: dynamic) -> dynamic
{
  cpp_macro("*min_element(a.begin(),a.end());");
}

func all(s: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/std");
}

func rall(x: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++.h>");
}

func sz(x: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++.");
}

func mez(s: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++");
}

func mex(s: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++.");
}

func metr(s: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++.h>");
}

func rep(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=a;i<b;i++)");
}

func fr(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=a;i<=b;i++)");
}

func rrep(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=a;i>b;i--)");
}

func rfr(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=a;i>=b;i--)");
}

var mxn: dynamic = (1e5 + 1);

var odd: dynamic = cpp_array(mxn);

var even: dynamic = cpp_array(mxn);

var new_odd: dynamic = cpp_array(mxn);

var new_even: dynamic = cpp_array(mxn);

func solve() -> dynamic
{
  var n: dynamic = 0;
  var m: dynamic = 0;
  var k: dynamic = 0;
  var x: dynamic = 0;
  var y: dynamic = 0;
  var z: dynamic = 0;
  read(n);
  memset(odd, 0, cpp_sizeof(odd));
  memset(even, 0, cpp_sizeof(even));
  memset(new_odd, 0, cpp_sizeof(new_odd));
  memset(new_even, 0, cpp_sizeof(new_even));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      if ((i % 2))
      {
        odd[a[i]] += 1;
      } else
      {
        even[a[i]] += 1;
      }
      i += 1;
    }
  }
  sort(a.begin(), a.end());
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((i % 2))
      {
        new_odd[a[i]] += 1;
      } else
      {
        new_even[a[i]] += 1;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < mxn))
    {
      if (((odd[i] != new_odd[i]) || (even[i] != new_even[i])))
      {
        write("NO\n");
        return;
      }
      i += 1;
    }
  }
  write("YES\n");
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var t: dynamic = 1;
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
