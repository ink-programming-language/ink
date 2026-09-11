// Translated from solution.cpp.

var IOS: dynamic = cpp_expression("#include<bits/stdc++.h>");

var int_cpp: dynamic = dynamic;

var endl: dynamic = cpp_expression("#inc");

var pb: dynamic = cpp_expression("#include<");

var ppb: dynamic = cpp_expression("#include");

var pf: dynamic = cpp_expression("#include<b");

var ppf: dynamic = cpp_expression("#include<");

func all(x: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++.");
}

func uniq(v: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++.h> using names");
}

func sz(x: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/std");
}

var fr: dynamic = cpp_expression("#incl");

var sc: dynamic = cpp_expression("#inclu");

var pii: dynamic = cpp_expression("#include<bits");

var vi: dynamic = cpp_expression("#include<bi");

var vpi: dynamic = cpp_expression("#include<bits/stdc++.");

var mii: dynamic = cpp_expression("#include<bit");

func rep(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=a;i<b;i++)");
}

func repe(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=a;i<=b;i++)");
}

func mem1(a: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++.h");
}

func mem0(a: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++.");
}

var ppc: dynamic = cpp_expression("#include<bits/stdc");

var ppcll: dynamic = cpp_expression("#include<bits/stdc++");

var INF: dynamic = cpp_expression("#include<bits/stdc");

var mod: dynamic = cpp_expression("#include<b");

var esp: dynamic = cpp_expression("#incl");

var mx: dynamic = (1e2 + 7);

var a: dynamic = cpp_array(mx);

func check(n: dynamic) -> dynamic
{
  var sum: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((i == 0))
      {
        sum += a[i];
      } else
      {
        var tmp: dynamic = (sum + a[i]);
        if ((tmp < i))
        {
          return false;
        }
        sum = (tmp - i);
      }
      i += 1;
    }
  }
  return true;
}

func main() -> dynamic
{
  var int_cpp: dynamic = 1;
  var n: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    read(n);
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        read(a[i]);
        i += 1;
      }
    }
    if ((!check(n)))
    {
      write("NO", "\n");
    } else
    {
      write("YES", "\n");
    }
  }
  return 0;
}
