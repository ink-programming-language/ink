// Translated from solution.cpp.

var a: dynamic = cpp_array(105, 105);

func main() -> dynamic
{
  var k: dynamic = cpp_uninitialized();
  read(k);
  memset(a, 0, cpp_sizeof((a)));
  {
    var i: dynamic = 1;
    while ((i <= 3))
    {
      {
        var j: dynamic = 1;
        while ((j <= 3))
        {
          if ((i != j))
          {
            a[i][j] = 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  k -= 1;
  var ans: dynamic = 3;
  if (k)
  {
    {
      ans = (ans + 1);
      while ((ans <= 100))
      {
        {
          var i: dynamic = 1;
          while ((i < ans))
          {
            var cnt: dynamic = 0;
            {
              var j: dynamic = 1;
              while ((j < i))
              {
                if ((a[i][j] && a[j][ans]))
                {
                  cnt += 1;
                }
                j += 1;
              }
            }
            if ((k >= cnt))
            {
              k -= cnt;
              a[i][ans] = cpp_assign(a[ans][i], "=", 1);
            }
            if ((!k))
            {
              break;
            }
            i += 1;
          }
        }
        if ((!k))
        {
          break;
        }
        ans += 1;
      }
    }
  }
  write(ans, "\n");
  {
    var i: dynamic = 1;
    while ((i <= ans))
    {
      {
        var j: dynamic = 1;
        while ((j <= ans))
        {
          write(a[i][j]);
          j += 1;
        }
      }
      write("\n");
      i += 1;
    }
  }
  return 0;
}
