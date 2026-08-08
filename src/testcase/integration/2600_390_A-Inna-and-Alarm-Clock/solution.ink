// Translated from solution.cpp.

var x: dynamic;

var y: dynamic;

var xx = cpp_array(105);

var yy = cpp_array(105);

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d%d", (&x), (&y));
      xx[x] = 1;
      yy[y] = 1;
      i += 1;
    }
  }
  var n1 = 0;
  var n2 = 0;
  {
    var i = 0;
    while ((i <= 100))
    {
      if ((xx[i] == 1))
      {
        n1 += 1;
      }
      if ((yy[i] == 1))
      {
        n2 += 1;
      }
      i += 1;
    }
  }
  printf("%d\n", min(n1, n2));
  return 0;
}
