// Translated from solution.cpp.

var a = cpp_array(100010);

var n: dynamic;

var m: dynamic;

func main()
{
  var i: dynamic;
  var x: dynamic;
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
    var k: dynamic;
    var l: dynamic;
    var r: dynamic;
    var s: dynamic;
    while (cpp_update(m, "--"))
    {
      scanf("%d%d%d%f", (&k), (&l), (&r), (&x));
      if ((k == 1))
      {
        {
          i = l;
          while ((i <= r))
          {
            a[i] -= if ((a[i] > x)) x else 0;
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
            if ((a[i] == x)) cpp_update(s, "++") else 0;
            i += 1;
          }
        }
        printf("%d\n", s);
      }
    }
  }
}
