// Translated from solution.cpp.

var p: dynamic = cpp_array(10000005);

var idxp: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var countt: dynamic = 0;
  scanf("%I64d", (&n));
  if (((n % 3) != 0))
  {
    printf("0\n");
  } else
  {
    n = (n / 3);
    var r: dynamic = cpp_cast(sqrt(n));
    {
      i = 1;
      while ((i <= r))
      {
        if (((n % i) == 0))
        {
          p[cpp_update(idxp, "++")] = i;
        }
        i += 1;
      }
    }
    {
      i = 0;
      while ((i < idxp))
      {
        {
          j = i;
          while ((j < idxp))
          {
            var C: dynamic = (n / ((p[i] * p[j])));
            if ((((p[i] * p[j]) * C) == n))
            {
              var A: dynamic = p[i];
              var B: dynamic = p[j];
              if (((((A + B) + C)) & 1))
              {
                j += 1;
                continue;
              }
              if (((A + B) <= C))
              {
                j += 1;
                continue;
              }
              if ((C < B))
              {
                j += 1;
                continue;
              }
              if (((A == B) && (B == C)))
              {
                countt += 1;
              } else if (((A == B) || (B == C)))
              {
                countt += 3;
              } else
              {
                countt += 6;
              }
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    printf("%I64d\n", countt);
  }
  scanf(" ");
}
