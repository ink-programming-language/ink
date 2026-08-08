// Translated from solution.cpp.

var q = cpp_array(110);

var a = cpp_array(110);

var p1 = cpp_array(110, 110);

var p2 = cpp_array(110, 110);

func main()
{
  var n: dynamic;
  var k: dynamic;
  var v1: dynamic;
  var v2: dynamic;
  scanf("%d%d", (&n), (&k));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&q[i]));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      p1[0][i] = cpp_assign(p2[0][i], "=", i);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= k))
    {
      {
        var j = 1;
        while ((j <= n))
        {
          p1[i][q[j]] = p1[(i - 1)][j];
          p2[i][j] = p2[(i - 1)][q[j]];
          j += 1;
        }
      }
      i += 1;
    }
  }
  var fff: dynamic;
  v1 = cpp_assign(v2, "=", -1);
  {
    var i = 0;
    while ((i <= k))
    {
      fff = true;
      {
        var j = 1;
        while (((j <= n) && fff))
        {
          if ((p1[i][j] != a[j]))
          {
            fff = false;
          }
          j += 1;
        }
      }
      if ((fff && (v1 == -1)))
      {
        v1 = i;
      }
      fff = true;
      {
        var j = 1;
        while (((j <= n) && fff))
        {
          if ((p2[i][j] != a[j]))
          {
            fff = false;
          }
          j += 1;
        }
      }
      if ((fff && (v2 == -1)))
      {
        v2 = i;
      }
      i += 1;
    }
  }
  var flag = false;
  if (((v1 >= k) || (v2 >= k)))
  {
    if (((v1 == k) || (v2 == k)))
    {
      flag = true;
    }
    if ((((v1 > k)) && ((((v1 - k)) % 2) == 0)))
    {
      flag = true;
    }
    if ((((v2 > k)) && ((((v2 - k)) % 2) == 0)))
    {
      flag = true;
    }
  } else if (((v1 <= k) || (v2 <= k)))
  {
    if (((((v1 > 0)) && ((k > v1))) && ((((k - v1)) % 2) == 0)))
    {
      flag = true;
    }
    if (((((v2 > 0)) && ((k > v2))) && ((((k - v2)) % 2) == 0)))
    {
      flag = true;
    }
  }
  if ((((v1 == 0) && (v2 == 0)) || ((v1 == -1) && (v2 == -1))))
  {
    flag = false;
  } else if ((((v1 == 1) && (v2 == 1)) && (k > 1)))
  {
    flag = false;
  }
  printf("%s\n", if (flag) "YES" else "NO");
  return 0;
}
