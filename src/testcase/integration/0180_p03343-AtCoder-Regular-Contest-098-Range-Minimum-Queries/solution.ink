// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var q: dynamic;

var a = cpp_array(2020);

func ok(M: dynamic)
{
  {
    var i = 0;
    while ((i < n))
    {
      var z = 0;
      var c = 0;
      {
        var j = 0;
        var k: dynamic;
        while ((j < n))
        {
          {
            k = j;
            while ((k < n))
            {
              if ((a[k] < a[i]))
              {
                break;
              }
              if ((a[k] <= (a[i] + M)))
              {
                c += 1;
              }
              k += 1;
            }
          }
          if (((k - j) >= m))
          {
            z += min((((k - j) - m) + 1), c);
          }
          c = 0;
          j = (k + 1);
        }
      }
      if ((z >= q))
      {
        return true;
      }
      i += 1;
    }
  }
  return false;
}

func main()
{
  scanf("%d%d%d", (&n), (&m), (&q));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  var L = -1;
  var R = 1e9;
  while ((L < (R - 1)))
  {
    var M = (((L + R)) / 2);
    if (ok(M))
    {
      R = M;
    } else
    {
      L = M;
    }
  }
  printf("%d\n", R);
  return 0;
}
