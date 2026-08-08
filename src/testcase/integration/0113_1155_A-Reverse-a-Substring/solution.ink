// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var a: dynamic;

var b: dynamic;

var c: dynamic;

var d: dynamic;

class node
{
  var a: dynamic;
  var b: dynamic;
}

func cmp(x: dynamic, y: dynamic)
{
  if ((x.a == y.a))
  {
    return (x.b > y.b);
  }
  return (x.a > y.a);
}

var s: dynamic;

var num = cpp_array(30);

var cr: dynamic;

var cr1: dynamic;

func check(cc: dynamic)
{
  {
    var i = ((cc - cpp_char("a")) + 1);
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

func main()
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
