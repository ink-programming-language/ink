// Translated from solution.cpp.

var maxn: dynamic = (1e6 + 5);

class Point
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var id: dynamic = cpp_uninitialized();
  func read(i: dynamic) -> dynamic
  {
      scanf("%d%d", (&x), (&y));
      x /= 1000;
      id = i;
    }
}

var a: dynamic = cpp_array(maxn);

func cmp(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.x < b.x) || ((a.x == b.x) && ( ((a.x & 1)) ? (a.y < b.y) : (a.y > b.y))));
}

func main(argument_0: dynamic) -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      a[i].read(i);
      i += 1;
    }
  }
  sort((a + 1), ((a + n) + 1), cmp);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      printf("%d%c", a[i].id,  ((i == n)) ? cpp_char("\n") : cpp_char(" "));
      i += 1;
    }
  }
  return 0;
}
