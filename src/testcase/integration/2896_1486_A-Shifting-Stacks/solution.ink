// Translated from solution.cpp.

var IOS = cpp_expression("#include<bits/stdc++.h>");

var int_cpp = dynamic;

var endl = cpp_expression("#inc");

var pb = cpp_expression("#include<");

var ppb = cpp_expression("#include");

var pf = cpp_expression("#include<b");

var ppf = cpp_expression("#include<");

func all(x: dynamic)
{
  return cpp_expression("#include<bits/stdc++.");
}

func uniq(v: dynamic)
{
  return cpp_expression("#include<bits/stdc++.h> using names");
}

func sz(x: dynamic)
{
  return cpp_expression("#include<bits/std");
}

var fr = cpp_expression("#incl");

var sc = cpp_expression("#inclu");

var pii = cpp_expression("#include<bits");

var vi = cpp_expression("#include<bi");

var vpi = cpp_expression("#include<bits/stdc++.");

var mii = cpp_expression("#include<bit");

func rep(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=a;i<b;i++)");
}

func repe(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=a;i<=b;i++)");
}

func mem1(a: dynamic)
{
  return cpp_expression("#include<bits/stdc++.h");
}

func mem0(a: dynamic)
{
  return cpp_expression("#include<bits/stdc++.");
}

var ppc = cpp_expression("#include<bits/stdc");

var ppcll = cpp_expression("#include<bits/stdc++");

var INF = cpp_expression("#include<bits/stdc");

var mod = cpp_expression("#include<b");

var esp = cpp_expression("#incl");

var mx = (1e2 + 7);

var a = cpp_array(mx);

func check(n: dynamic)
{
  var sum = 0;
  {
    var i = 0;
    while ((i < n))
    {
      if ((i == 0))
      {
        sum += a[i];
      } else
      {
        var tmp = (sum + a[i]);
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

func main()
{
  var int_cpp = 1;
  var n: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    read(n);
    {
      var i = 0;
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
