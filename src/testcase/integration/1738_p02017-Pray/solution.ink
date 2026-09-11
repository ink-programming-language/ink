// Translated from solution.cpp.

func lp(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<n;i++)");
}

func lps(i: dynamic, j: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=j;i<n;i++)");
}

var fordebug: dynamic = cpp_expression("#include<");

var DEKAI: dynamic = cpp_expression("#include<bi");

var INF: dynamic = cpp_expression("#includ");

var int_cpp: dynamic = dynamic;

var cpp_double: dynamic = dynamic;

var floot10: dynamic = cpp_expression("#include<bits/stdc++.h> using");

func main() -> dynamic
{
  var h: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  read(h, w, x, y);
  if (((((h * w) % 2) == 1) && ((((x + y)) % 2) == 1)))
  {
    write("No", "\n");
  } else
  {
    write("Yes", "\n");
  }
  return 0;
}
