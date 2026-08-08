// Translated from solution.cpp.

func main()
{
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var a: dynamic;
  var p: dynamic;
  var N: dynamic;
  var A: dynamic;
  var nb = cpp_array(108);
  while ((scanf("%d", (&N)) == 1))
  {
    A = (1 << 20);
    N *= 2;
    {
      i = 0;
      while ((i < N))
      {
        scanf("%d", (&nb[i]));
        i += 1;
      }
    }
    sort(nb, (nb + N));
    {
      i = 0;
      while ((i < (N - 1)))
      {
        {
          j = (i + 1);
          while ((j < N))
          {
            a = 0;
            p = (-1);
            {
              k = 0;
              while ((k < N))
              {
                if (((k != i) && (k != j)))
                {
                  if ((p < 0))
                  {
                    p = nb[k];
                  } else
                  {
                    a += abs((p - nb[k]));
                    p = (-1);
                  }
                }
                k += 1;
              }
            }
            A = min(A, a);
            j += 1;
          }
        }
        i += 1;
      }
    }
    printf("%d\n", A);
  }
  return 0;
}
