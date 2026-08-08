// Translated from solution.cpp.

func sol(d1: dynamic, d2: dynamic)
{
  {
    var i = 0;
    while ((i < 4))
    {
      var tmp1 = d1[0];
      d1[0] = d1[2];
      d1[2] = d1[5];
      d1[5] = d1[3];
      d1[3] = tmp1;
      {
        var j = 0;
        while ((j < 4))
        {
          var tmp2 = d1[0];
          d1[0] = d1[1];
          d1[1] = d1[5];
          d1[5] = d1[4];
          d1[4] = tmp2;
          {
            var k = 0;
            while ((k < 4))
            {
              var tmp3 = d1[1];
              d1[1] = d1[2];
              d1[2] = d1[4];
              d1[4] = d1[3];
              d1[3] = tmp3;
              if (equal(d1.cbegin(), d1.cend(), d2.cbegin()))
              {
                printf("No\n");
                return false;
              }
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return true;
}

func main()
{
  var n: dynamic;
  scanf("%d", (&n));
  var d = cpp_construct(n, vector(6));
  {
    var i = 0;
    while ((i < n))
    {
      for (var e in d[i])
      {
        scanf("%d", (&e));
      }
      i += 1;
    }
  }
  var flag: dynamic;
  {
    var i = 0;
    while ((i < (n - 1)))
    {
      {
        var j = (i + 1);
        while ((j < n))
        {
          flag = sol(d[i], d[j]);
          if ((flag == false))
          {
            return 0;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  printf("Yes\n");
  return 0;
}
