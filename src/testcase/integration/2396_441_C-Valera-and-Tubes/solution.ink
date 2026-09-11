// Translated from solution.cpp.

var N: dynamic = 305;

var x: dynamic = cpp_array((N * N));

var y: dynamic = cpp_array((N * N));

var tot: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  scanf("%d%d%d", (&n), (&m), (&k));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if ((i & 1))
      {
        {
          var j: dynamic = 1;
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
          var j: dynamic = m;
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
    var i: dynamic = 1;
    while ((i < k))
    {
      printf("2 %d %d %d %d\n", x[((2 * i) - 1)], y[((2 * i) - 1)], x[(2 * i)], y[(2 * i)]);
      i += 1;
    }
  }
  printf("%d ", (((n * m) - (2 * k)) + 2));
  {
    var i: dynamic = ((2 * k) - 1);
    while ((i <= (n * m)))
    {
      printf("%d %d ", x[i], y[i]);
      i += 1;
    }
  }
}
