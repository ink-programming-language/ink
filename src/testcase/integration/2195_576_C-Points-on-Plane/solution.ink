// Translated from solution.cpp.

var maxn = (1e6 + 5);

class Point
{
  var x: dynamic;
  var y: dynamic;
  var id: dynamic;
  func read(i: dynamic)
  {
      scanf("%d%d", (&x), (&y));
      x /= 1000;
      id = i;
    }
}

var a = cpp_array(maxn);

func cmp(a: dynamic, b: dynamic)
{
  return ((a.x < b.x) || ((a.x == b.x) && (if ((a.x & 1)) (a.y < b.y) else (a.y > b.y))));
}

func main(argument_0: dynamic)
{
  var n: dynamic;
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      a[i].read(i);
      i += 1;
    }
  }
  sort((a + 1), ((a + n) + 1), cmp);
  {
    var i = 1;
    while ((i <= n))
    {
      printf("%d%c", a[i].id, if ((i == n)) cpp_char("\n") else cpp_char(" "));
      i += 1;
    }
  }
  return 0;
}
