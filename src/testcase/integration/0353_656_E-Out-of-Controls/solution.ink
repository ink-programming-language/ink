// Translated from solution.cpp.

func main()
{
  var maxn = 999999999;
  var m = cpp_array(105, 105);
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < n))
        {
          m[i][j] = maxn;
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < n))
        {
          read(m[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var k = 0;
    while ((k < n))
    {
      {
        var i = 0;
        while ((i < n))
        {
          {
            var j = 0;
            while ((j < n))
            {
              if ((m[i][j] > (m[i][k] + m[k][j])))
              {
                m[i][j] = (m[i][k] + m[k][j]);
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
      k += 1;
    }
  }
  var ans = 0;
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < n))
        {
          if ((m[i][j] < maxn))
          {
            ans = max(ans, m[i][j]);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  printf("%d\n", ans);
  return 0;
}
