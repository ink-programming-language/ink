// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(100005);

var mars: dynamic = cpp_array((2 * 100005));

func main() -> dynamic
{
  scanf("%d %d %d", (&n), (&m), (&c));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      scanf("%d", (&x));
      mars[i] += x;
      mars[(i + (((n - m) + 1)))] -= x;
      i += 1;
    }
  }
  var act: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      act += mars[i];
      mars[i] = (act + a[i]);
      mars[i] %= c;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      write(mars[i], cpp_char(" "));
      i += 1;
    }
  }
  write(cpp_char("\n"));
  return 0;
}
