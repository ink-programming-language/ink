// Translated from solution.cpp.

func main() -> dynamic
{
  var maxn: dynamic = 999999999;
  var m: dynamic = cpp_array(105, 105);
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
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
    var k: dynamic = 0;
    while ((k < n))
    {
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          {
            var j: dynamic = 0;
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
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
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
