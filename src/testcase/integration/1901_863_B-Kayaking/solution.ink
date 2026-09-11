// Translated from solution.cpp.

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  var N: dynamic = cpp_uninitialized();
  var A: dynamic = cpp_uninitialized();
  var nb: dynamic = cpp_array(108);
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
