// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

var d: dynamic = cpp_uninitialized();

class node
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
}

func cmp(x: dynamic, y: dynamic) -> dynamic
{
  if ((x.a == y.a))
  {
    return (x.b > y.b);
  }
  return (x.a > y.a);
}

var s: dynamic = cpp_uninitialized();

var num: dynamic = cpp_array(30);

var cr: dynamic = cpp_uninitialized();

var cr1: dynamic = cpp_uninitialized();

func check(cc: dynamic) -> dynamic
{
  {
    var i: dynamic = ((cc - cpp_char("a")) + 1);
    while ((i < 26))
    {
      if (num[i])
      {
        return num[i];
      }
      i += 1;
    }
  }
  return 0;
}

func main() -> dynamic
{
  read(n);
  getchar();
  while (((cpp_assign(cr, "=", getchar())) && (cr != cpp_char("\n"))))
  {
    m += 1;
    if ((!num[(cr - cpp_char("a"))]))
    {
      num[(cr - cpp_char("a"))] = m;
    }
    if (check(cr))
    {
      write("YES", "\n");
      write(check(cr), " ", m, "\n");
      return 0;
    }
  }
  write("NO", "\n");
  return 0;
}
