// Translated from solution.cpp.

var MAXN = (4e2 + 7);

func main()
{
  var n: dynamic;
  var mp = cpp_array(MAXN, MAXN);
  var cnt = 1;
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      if (((i % 2) == 1))
      {
        {
          var j = 1;
          while ((j <= n))
          {
            mp[j][i] = cpp_update(cnt, "++");
            j += 1;
          }
        }
      } else
      {
        {
          var j = n;
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
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
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
