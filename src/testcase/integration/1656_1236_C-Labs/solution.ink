// Translated from solution.cpp.

var MAXN: dynamic = (4e2 + 7);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var mp: dynamic = cpp_array(MAXN, MAXN);
  var cnt: dynamic = 1;
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if (((i % 2) == 1))
      {
        {
          var j: dynamic = 1;
          while ((j <= n))
          {
            mp[j][i] = cpp_update(cnt, "++");
            j += 1;
          }
        }
      } else
      {
        {
          var j: dynamic = n;
          while ((j >= 1))
          {
            mp[j][i] = cpp_update(cnt, "++");
            j -= 1;
          }
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
        while ((j <= n))
        {
          printf("%d ", mp[i][j]);
          j += 1;
        }
      }
      printf("\n");
      i += 1;
    }
  }
}
