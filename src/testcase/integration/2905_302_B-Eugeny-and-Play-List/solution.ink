// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  scanf("%d%d", (&n), (&m));
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  scanf("%d%d", (&a), (&b));
  vec[0] = (a * b);
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      scanf("%d%d", (&a), (&b));
      vec[i] = (vec[(i - 1)] + (a * b));
      i += 1;
    }
  }
  var l: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var v: dynamic = cpp_uninitialized();
      scanf("%d", (&v));
      {
        var j: dynamic = l;
        while ((j < n))
        {
          if ((v <= vec[j]))
          {
            j += 1;
            printf("%d\n", j);
            l = (j - 1);
            break;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return 0;
}
