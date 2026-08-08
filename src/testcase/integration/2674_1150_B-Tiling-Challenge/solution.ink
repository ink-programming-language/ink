// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  read(n);
  var i: dynamic;
  var second = cpp_array(n);
  {
    i = 0;
    while ((i < n))
    {
      read(second[i]);
      i += 1;
    }
  }
  var a = cpp_array((n + 2), (n + 2));
  var j: dynamic;
  {
    i = 0;
    while ((i <= (n + 1)))
    {
      {
        j = 0;
        while ((j <= (n + 1)))
        {
          a[i][j] = 1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    i = 0;
    while ((i < n))
    {
      {
        j = 0;
        while ((j < n))
        {
          if ((second[i][j] == cpp_char(".")))
          {
            a[(i + 1)][(j + 1)] = 0;
          } else
          {
            a[(i + 1)][(j + 1)] = 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var count = 0;
  {
    i = 1;
    while ((i <= n))
    {
      {
        j = 1;
        while ((j <= n))
        {
          if ((((((a[i][j] == 0) && (a[(i - 1)][j] == 0)) && (a[i][(j - 1)] == 0)) && (a[(i + 1)][j] == 0)) && (a[i][(j + 1)] == 0)))
          {
            a[i][j] = 1;
            a[(i - 1)][j] = 1;
            a[i][(j - 1)] = 1;
            a[(i + 1)][j] = 1;
            a[i][(j + 1)] = 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      {
        j = 1;
        while ((j <= n))
        {
          if ((a[i][j] == 0))
          {
            count = 1;
            break;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  if ((count == 1))
  {
    write("NO\n");
  } else
  {
    write("YES\n");
  }
}
