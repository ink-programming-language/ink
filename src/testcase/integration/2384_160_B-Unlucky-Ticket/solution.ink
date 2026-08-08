// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var i: dynamic;
  var j: dynamic;
  var flag = 1;
  var figure: dynamic;
  var c1 = cpp_array(101);
  var c2 = cpp_array(101);
  var k: dynamic;
  var c = cpp_array(201);
  scanf("%d", (&n));
  scanf("%s", c);
  {
    i = 0;
    while ((i < n))
    {
      c1[i] = c[i];
      c2[i] = c[(i + n)];
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < (n - 1)))
    {
      {
        j = (i + 1);
        while ((j < n))
        {
          if ((c1[i] > c1[j]))
          {
            k = c1[i];
            c1[i] = c1[j];
            c1[j] = k;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < (n - 1)))
    {
      {
        j = (i + 1);
        while ((j < n))
        {
          if ((c2[i] > c2[j]))
          {
            k = c2[i];
            c2[i] = c2[j];
            c2[j] = k;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  if ((c1[0] < c2[0]))
  {
    figure = 0;
  }
  if ((c1[0] > c2[0]))
  {
    figure = 1;
  }
  if ((c1[0] == c2[0]))
  {
    printf("NO\n");
    figure = 2;
    flag = 0;
  }
  {
    i = 1;
    while ((i < n))
    {
      if (((figure == 1) && (c1[i] <= c2[i])))
      {
        printf("NO\n");
        flag = 0;
        break;
      }
      if (((figure == 0) && (c1[i] >= c2[i])))
      {
        printf("NO\n");
        flag = 0;
        break;
      }
      i += 1;
    }
  }
  if (flag)
  {
    printf("YES\n");
  }
  return 0;
}
