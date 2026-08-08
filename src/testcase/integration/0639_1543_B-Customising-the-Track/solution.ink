// Translated from solution.cpp.

var ll = dynamic;

var mp = cpp_expression("#include<");

var pb = cpp_expression("#include<");

var MOD = cpp_expression("#include<i");

var fi = cpp_expression("#incl");

var se = cpp_expression("#inclu");

var inf = cpp_expression("#includ");

var FASTIO = cpp_expression("#include<iostream> #include<bits/");

func all(v: dynamic)
{
  return cpp_expression("#include<iostream");
}

func sumof(v: dynamic)
{
  return cpp_expression("#include<iostream> #include<bits/st");
}

func maxof(v: dynamic)
{
  return cpp_expression("#include<iostream>");
}

func minof(v: dynamic)
{
  return cpp_expression("#include<iostream>");
}

var Vl = cpp_expression("#include<i");

var Vlp = cpp_expression("#include<iostream>");

var Vll = cpp_expression("#include<iostream>");

var Vi = cpp_expression("#include<io");

var Vip = cpp_expression("#include<iostream> #i");

var Vii = cpp_expression("#include<iostream>");

func repU(i: dynamic, x: dynamic, y: dynamic, d: dynamic)
{
  cpp_macro("for(int i=x;i<y;i+=d)");
}

func repD(i: dynamic, x: dynamic, y: dynamic, d: dynamic)
{
  cpp_macro("for(int i=x;i>=y;i-=d)");
}

var n: dynamic;

func main()
{
  var int_cpp = 1;
  read(t);
  while (cpp_update(t, "--"))
  {
    var val: dynamic;
    read(n);
    var sm = 0;
    repU(i, 0, n, 1);
    {
      read(val);
      sm += val;
    }
    if ((sm % n))
    {
      write((((sm % n)) * ((n - ((sm % n))))), cpp_char("\n"));
    } else
    {
      write(0, cpp_char("\n"));
    }
  }
  return 0;
}
