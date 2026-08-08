// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var c: dynamic;

var x: dynamic;

var a = cpp_array(100005);

var mars = cpp_array((2 * 100005));

func main()
{
  scanf("%d %d %d", (&n), (&m), (&c));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      scanf("%d", (&x));
      mars[i] += x;
      mars[(i + (((n - m) + 1)))] -= x;
      i += 1;
    }
  }
  var act = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      act += mars[i];
      mars[i] = (act + a[i]);
      mars[i] %= c;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      write(mars[i], cpp_char(" "));
      i += 1;
    }
  }
  write(cpp_char("\n"));
  return 0;
}
