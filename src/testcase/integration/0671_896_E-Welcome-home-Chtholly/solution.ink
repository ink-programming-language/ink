// Translated from solution.cpp.

var a: dynamic = cpp_array(100010);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  scanf("%d%d", (&n), (&m));
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var k: dynamic = cpp_uninitialized();
    var l: dynamic = cpp_uninitialized();
    var r: dynamic = cpp_uninitialized();
    var s: dynamic = cpp_uninitialized();
    while (cpp_update(m, "--"))
    {
      scanf("%d%d%d%f", (&k), (&l), (&r), (&x));
      if ((k == 1))
      {
        {
          i = l;
          while ((i <= r))
          {
            a[i] -=  ((a[i] > x)) ? x : 0;
            i += 1;
          }
        }
      } else
      {
        {
          s = 0;
          i = l;
          while ((i <= r))
          {
             ((a[i] == x)) ? cpp_update(s, "++") : 0;
            i += 1;
          }
        }
        printf("%d\n", s);
      }
    }
  }
}
