// Translated from solution.cpp.

var inf = 0x3f3f3f3f;

var N = 107;

var mp = cpp_array(N, N);

func main()
{
  var t: dynamic;
  var n: dynamic;
  scanf("%d", (&t));
  while (cpp_update(t, "--"))
  {
    scanf("%d", (&n));
    var x = 0;
    var y = 1;
    var z = 2;
    {
      var i = 1;
      while ((i <= n))
      {
        {
          var j = 1;
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
