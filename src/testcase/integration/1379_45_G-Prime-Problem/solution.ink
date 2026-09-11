// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var sum: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(6010);

var bo: dynamic = cpp_array(40000010);

var prime: dynamic = cpp_array(2000010);

var tot: dynamic = cpp_uninitialized();

func getprime(n: dynamic) -> dynamic
{
  {
    var i: dynamic = 2;
    while ((i < n))
    {
      if ((!bo[i]))
      {
        prime[cpp_update(tot, "++")] = i;
      }
      {
        var j: dynamic = 0;
        while (((j < tot) && ((i * prime[j]) < n)))
        {
          bo[(i * prime[j])] = 1;
          if ((!((i % prime[j]))))
          {
            break;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return;
}

func main() -> dynamic
{
  scanf("%d", (&n));
  sum = ((n * ((n + 1))) / 2);
  bo[1] = true;
  getprime((sum + 1));
  {
    var i: dynamic = (prime[(tot - 1)] + 1);
    while ((i <= sum))
    {
      bo[i] = true;
      i += 1;
    }
  }
  if ((!bo[sum]))
  {
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        printf("1 ");
        i += 1;
      }
    }
    printf("\n");
    return 0;
  }
  if (((sum % 2) == 1))
  {
    if ((!bo[(sum - 2)]))
    {
      {
        var i: dynamic = 1;
        while ((i <= n))
        {
          if ((i == 2))
          {
            printf("2 ");
          } else
          {
            printf("1 ");
          }
          i += 1;
        }
      }
      printf("\n");
      return 0;
    } else
    {
      var t: dynamic = n;
      while (bo[t])
      {
        t -= 1;
      }
      a[t] = 3;
      sum -= t;
    }
  }
  if (((sum % 2) == 0))
  {
    {
      var j: dynamic = (sum - 2);
      while ((j > 1))
      {
        if (((!bo[j]) && (!bo[(sum - j)])))
        {
          var pos: dynamic = n;
          var tmp: dynamic = j;
          while (tmp)
          {
            while (((pos > tmp) || a[pos]))
            {
              if ((pos > tmp))
              {
                pos = tmp;
              } else
              {
                pos -= 1;
              }
            }
            a[pos] = 1;
            tmp -= pos;
          }
          {
            var i: dynamic = 1;
            while ((i <= n))
            {
              if ((a[i] == 0))
              {
                printf("2 ");
              } else
              {
                printf("%d ", a[i]);
              }
              i += 1;
            }
          }
          return 0;
        }
        j -= 1;
      }
    }
  }
  return 0;
}
