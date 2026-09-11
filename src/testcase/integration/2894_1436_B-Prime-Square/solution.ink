// Translated from solution.cpp.

var inf: dynamic = 0x3f3f3f3f;

var N: dynamic = 107;

var mp: dynamic = cpp_array(N, N);

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&t));
  while (cpp_update(t, "--"))
  {
    scanf("%d", (&n));
    var x: dynamic = 0;
    var y: dynamic = 1;
    var z: dynamic = 2;
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        {
          var j: dynamic = 1;
          while ((j <= n))
          {
            if ((j > 1))
            {
              printf(" ");
            }
            if ((((j == x) || (j == y)) || (j == z)))
            {
              printf("1");
            } else
            {
              printf("0");
            }
            j += 1;
          }
        }
        printf("\n");
        x += 1;
        y += 1;
        z += 1;
        i += 1;
      }
    }
  }
  return 0;
}
