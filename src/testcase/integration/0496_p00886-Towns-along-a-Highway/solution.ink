// Translated from solution.cpp.

var d = cpp_array(310);

var c = cpp_array(510);

var v = cpp_array(310);

var at = cpp_array(30);

var n: dynamic;

var tmp = cpp_array(30);

var sz: dynamic;

var S: dynamic;

func ABS(a: dynamic)
{
  return max(a, (-a));
}

func solve(a: dynamic, b: dynamic)
{
  if ((a == n))
  {
    {
      var i = 0;
      while ((i < n))
      {
        tmp[i] = at[i];
        i += 1;
      }
    }
    sort(tmp, (tmp + n));
    var val: dynamic;
    {
      var i = 0;
      while ((i < n))
      {
        val.push_back(tmp[i]);
        i += 1;
      }
    }
    if (S.count(val))
    {
      return;
    }
    S.insert(val);
    {
      var i = 0;
      while ((i < (n - 1)))
      {
        if (i)
        {
          printf(" ");
          printf("%d", (tmp[(i + 1)] - tmp[i]));
        } else
        {
          printf("%d", tmp[1]);
        }
        i += 1;
      }
    }
    printf("\n");
    return;
  }
  {
    var i = b;
    while ((i > 0))
    {
      if ((!c[i]))
      {
        i -= 1;
        continue;
      }
      {
        var j = 0;
        while ((j < a))
        {
          if (((at[j] - i) > 0))
          {
            var ok = true;
            var t = (at[j] - i);
            {
              var k = 0;
              while ((k < a))
              {
                if ((!c[ABS((t - at[k]))]))
                {
                  ok = false;
                }
                c[ABS((t - at[k]))] -= 1;
                k += 1;
              }
            }
            if (ok)
            {
              at[a] = t;
              solve((a + 1), i);
            }
            {
              var k = 0;
              while ((k < a))
              {
                c[ABS((t - at[k]))] += 1;
                k += 1;
              }
            }
          }
          if (((at[j] + i) < d[0]))
          {
            var ok = true;
            var t = (at[j] + i);
            {
              var k = 0;
              while ((k < a))
              {
                if ((!c[ABS((t - at[k]))]))
                {
                  ok = false;
                }
                c[ABS((t - at[k]))] -= 1;
                k += 1;
              }
            }
            if (ok)
            {
              at[a] = t;
              solve((a + 1), i);
            }
            {
              var k = 0;
              while ((k < a))
              {
                c[ABS((t - at[k]))] += 1;
                k += 1;
              }
            }
          }
          j += 1;
        }
      }
      break;
      i -= 1;
    }
  }
}

func main()
{
  var a: dynamic;
  while (cpp_comma(scanf("%d", (&a)), a))
  {
    n = a;
    S.clear();
    sz = ((n * ((n - 1))) / 2);
    {
      var i = 0;
      while ((i < ((a * ((a - 1))) / 2)))
      {
        scanf("%d", (d + i));
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < 510))
      {
        c[i] = 0;
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < 310))
      {
        v[i] = 0;
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < ((a * ((a - 1))) / 2)))
      {
        c[d[i]] += 1;
        i += 1;
      }
    }
    c[d[0]] -= 1;
    at[0] = d[0];
    at[1] = 0;
    solve(2, (d[0] - 1));
    printf("-----\n");
  }
}
