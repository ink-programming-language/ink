// Translated from solution.cpp.

var ll: dynamic = dynamic;

var mp: dynamic = cpp_expression("#include<");

var pb: dynamic = cpp_expression("#include<");

var MOD: dynamic = cpp_expression("#include<i");

var fi: dynamic = cpp_expression("#incl");

var se: dynamic = cpp_expression("#inclu");

var inf: dynamic = cpp_expression("#includ");

var FASTIO: dynamic = cpp_expression("#include<iostream> #include<bits/");

func all(v: dynamic) -> dynamic
{
  return cpp_expression("#include<iostream");
}

func sumof(v: dynamic) -> dynamic
{
  return cpp_expression("#include<iostream> #include<bits/st");
}

func maxof(v: dynamic) -> dynamic
{
  return cpp_expression("#include<iostream>");
}

func minof(v: dynamic) -> dynamic
{
  return cpp_expression("#include<iostream>");
}

var Vl: dynamic = cpp_expression("#include<i");

var Vlp: dynamic = cpp_expression("#include<iostream>");

var Vll: dynamic = cpp_expression("#include<iostream>");

var Vi: dynamic = cpp_expression("#include<io");

var Vip: dynamic = cpp_expression("#include<iostream> #i");

var Vii: dynamic = cpp_expression("#include<iostream>");

func repU(i: dynamic, x: dynamic, y: dynamic, d: dynamic) -> dynamic
{
  cpp_macro("for(int i=x;i<y;i+=d)");
}

func repD(i: dynamic, x: dynamic, y: dynamic, d: dynamic) -> dynamic
{
  cpp_macro("for(int i=x;i>=y;i-=d)");
}

var n: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var int_cpp: dynamic = 1;
  read(t);
  while (cpp_update(t, "--"))
  {
    var val: dynamic = cpp_uninitialized();
    read(n);
    var sm: dynamic = 0;
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
