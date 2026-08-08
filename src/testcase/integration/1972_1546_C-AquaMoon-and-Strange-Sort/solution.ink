// Translated from solution.cpp.

func amax(a: dynamic, b: dynamic)
{
  if ((b > a))
  {
    a = b;
  }
  return a;
}

func amin(a: dynamic, b: dynamic)
{
  if ((b < a))
  {
    a = b;
  }
  return a;
}

var ll = dynamic;

var ld = dynamic;

var INF = cpp_expression("#include<b");

var INFL = cpp_expression("#include<b");

var md = cpp_expression("#include<b");

var mk = cpp_expression("#include<");

var pi = cpp_expression("#include<bits");

var ss = cpp_expression("#inclu");

var ff = cpp_expression("#incl");

var pb = cpp_expression("#include<");

var eb = cpp_expression("#include<bit");

func ppcl(x: dynamic)
{
  return cpp_expression("#include<bits/stdc++.h>");
}

func ppc(x: dynamic)
{
  return cpp_expression("#include<bits/stdc++.");
}

func maxi(a: dynamic)
{
  cpp_macro("*max_element(a.begin(),a.end());");
}

func mini(a: dynamic)
{
  cpp_macro("*min_element(a.begin(),a.end());");
}

func all(s: dynamic)
{
  return cpp_expression("#include<bits/std");
}

func rall(x: dynamic)
{
  return cpp_expression("#include<bits/stdc++.h>");
}

func sz(x: dynamic)
{
  return cpp_expression("#include<bits/stdc++.");
}

func mez(s: dynamic)
{
  return cpp_expression("#include<bits/stdc++");
}

func mex(s: dynamic)
{
  return cpp_expression("#include<bits/stdc++.");
}

func metr(s: dynamic)
{
  return cpp_expression("#include<bits/stdc++.h>");
}

func rep(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=a;i<b;i++)");
}

func fr(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=a;i<=b;i++)");
}

func rrep(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=a;i>b;i--)");
}

func rfr(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=a;i>=b;i--)");
}

var mxn = (1e5 + 1);

var odd = cpp_array(mxn);

var even = cpp_array(mxn);

var new_odd = cpp_array(mxn);

var new_even = cpp_array(mxn);

func solve()
{
  var n = 0;
  var m = 0;
  var k = 0;
  var x = 0;
  var y = 0;
  var z = 0;
  read(n);
  memset(odd, 0, cpp_sizeof(odd));
  memset(even, 0, cpp_sizeof(even));
  memset(new_odd, 0, cpp_sizeof(new_odd));
  memset(new_even, 0, cpp_sizeof(new_even));
  {
    var i = 0;
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
    var i = 0;
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
    var i = 1;
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

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  var t = 1;
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
