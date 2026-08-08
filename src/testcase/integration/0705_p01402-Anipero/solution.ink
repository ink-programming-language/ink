// Translated from solution.cpp.

var in_cpp = cpp_array(100);

var dp = cpp_array(1010, 110);

var val = cpp_array(1010);

var sp = cpp_array(110);

var sq = cpp_array(110);

var np = cpp_array(110);

var nq = cpp_array(110);

func main()
{
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var d: dynamic;
  while (cpp_comma(scanf("%d%d%d%d", (&a), (&b), (&c), (&d)), a))
  {
    {
      var i = 0;
      while ((i < b))
      {
        scanf("%s%d%d", in_cpp, (sp + i), (sq + i));
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < c))
      {
        scanf("%s%d%d", in_cpp, (np + i), (nq + i));
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < 110))
      {
        {
          var j = 0;
          while ((j < 1010))
          {
            dp[i][j] = -99999999;
            j += 1;
          }
        }
        i += 1;
      }
    }
    dp[0][0] = 0;
    {
      var i = 0;
      while ((i < c))
      {
        {
          var j = i;
          while ((j >= 0))
          {
            {
              var k = (a - np[i]);
              while ((k >= 0))
              {
                dp[(j + 1)][(k + np[i])] = max(dp[(j + 1)][(k + np[i])], (dp[j][k] + nq[i]));
                k -= 1;
              }
            }
            j -= 1;
          }
        }
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < 1010))
      {
        val[i] = -99999999;
        i += 1;
      }
    }
    {
      var i = d;
      while ((i <= c))
      {
        {
          var j = 0;
          while ((j <= a))
          {
            val[j] = max(val[j], dp[i][j]);
            j += 1;
          }
        }
        i += 1;
      }
    }
    var ret = 0;
    {
      var i = 0;
      while ((i < b))
      {
        {
          var j = 0;
          while ((j <= (a - sp[i])))
          {
            ret = max(ret, (sq[i] + val[j]));
            j += 1;
          }
        }
        {
          var j = (i + 1);
          while ((j < b))
          {
            {
              var k = 0;
              while ((k <= ((a - sp[i]) - sp[j])))
              {
                ret = max(ret, ((sq[i] + sq[j]) + val[k]));
                k += 1;
              }
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    printf("%d\n", ret);
  }
}
