// Translated from solution.cpp.

func lp(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<n;i++)");
}

func lps(i: dynamic, j: dynamic, n: dynamic)
{
  cpp_macro("for(int i=j;i<n;i++)");
}

var fordebug = cpp_expression("#include<");

var DEKAI = cpp_expression("#include<bi");

var INF = cpp_expression("#includ");

var int_cpp = dynamic;

var double = dynamic;

var floot10 = cpp_expression("#include<bits/stdc++.h> using");

func main()
{
  var h: dynamic;
  var w: dynamic;
  var x: dynamic;
  var y: dynamic;
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
