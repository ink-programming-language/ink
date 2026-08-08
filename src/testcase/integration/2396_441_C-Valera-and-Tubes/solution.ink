// Translated from solution.cpp.

var N = 305;

var x = cpp_array((N * N));

var y = cpp_array((N * N));

var tot: dynamic;

func main()
{
  var n: dynamic;
  var m: dynamic;
  var k: dynamic;
  scanf("%d%d%d", (&n), (&m), (&k));
  {
    var i = 1;
    while ((i <= n))
    {
      if ((i & 1))
      {
        {
          var j = 1;
          while ((j <= m))
          {
            x[cpp_update(tot, "++")] = i;
            y[tot] = j;
            j += 1;
          }
        }
      } else
      {
        {
          var j = m;
          while ((j >= 1))
          {
            x[cpp_update(tot, "++")] = i;
            y[tot] = j;
            j -= 1;
          }
        }
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < k))
    {
      printf("2 %d %d %d %d\n", x[((2 * i) - 1)], y[((2 * i) - 1)], x[(2 * i)], y[(2 * i)]);
      i += 1;
    }
  }
  printf("%d ", (((n * m) - (2 * k)) + 2));
  {
    var i = ((2 * k) - 1);
    while ((i <= (n * m)))
    {
      printf("%d %d ", x[i], y[i]);
      i += 1;
    }
  }
}
