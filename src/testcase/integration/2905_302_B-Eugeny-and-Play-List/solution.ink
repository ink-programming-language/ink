// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var m: dynamic;
  scanf("%d%d", (&n), (&m));
  var a: dynamic;
  var b: dynamic;
  scanf("%d%d", (&a), (&b));
  vec[0] = (a * b);
  {
    var i = 1;
    while ((i < n))
    {
      scanf("%d%d", (&a), (&b));
      vec[i] = (vec[(i - 1)] + (a * b));
      i += 1;
    }
  }
  var l = 0;
  {
    var i = 0;
    while ((i < m))
    {
      var v: dynamic;
      scanf("%d", (&v));
      {
        var j = l;
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
