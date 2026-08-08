// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var d: dynamic;

var a = cpp_array(1005);

var c = cpp_array(1005);

var i: dynamic;

var j: dynamic;

var k: dynamic;

var l: dynamic;

var sum: dynamic;

func main()
{
  {
    scanf("%d%d%d", (&n), (&m), (&d));
    i = 0;
    while ((i < m))
    {
      scanf("%d", (&c[i]));
      sum += c[i];
      i += 1;
    }
  }
  if ((((sum + (((d - 1)) * ((m + 1)))) < n) || (sum > n)))
  {
    printf("NO\n");
    return 0;
  } else
  {
    printf("YES\n");
  }
  {
    i = cpp_assign(j, "=", 0);
    while ((i < n))
    {
      var b = false;
      if (((sum + j) >= n))
      {
        i -= 1;
        break;
      }
      j += (d - 1);
      sum -= c[i];
      {
        k = j;
        while (((k - j) < c[i]))
        {
          if (((k + sum) >= n))
          {
            {
              k = j;
              while (((k + sum) < n))
              {
                a[k] = 0;
                k += 1;
              }
            }
            i -= 1;
            b = true;
            break;
          }
          a[k] = (i + 1);
          k += 1;
        }
      }
      j = k;
      if (b)
      {
        break;
      }
      i += 1;
    }
  }
  k = cpp_assign(j, "=", (n - 1));
  {
    l = (m - 1);
    while ((l > i))
    {
      {
        k = j;
        while (((j - k) < c[l]))
        {
          a[k] = (l + 1);
          k -= 1;
        }
      }
      j = k;
      l -= 1;
    }
  }
  {
    i = 0;
    while ((i < n))
    {
      printf("%d ", a[i]);
      i += 1;
    }
  }
  return 0;
}
