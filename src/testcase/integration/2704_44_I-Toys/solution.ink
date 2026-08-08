// Translated from solution.cpp.

var dp = cpp_array(123);

func ot(a: dynamic)
{
  {
    var i = 0;
    while ((i < a.size()))
    {
      if (i)
      {
        printf(",");
      }
      printf("{");
      {
        var q = 0;
        while ((q < a[i].size()))
        {
          if (q)
          {
            printf(",");
          }
          printf("%d", a[i][q]);
          q += 1;
        }
      }
      printf("}");
      i += 1;
    }
  }
  puts("");
}

func main()
{
  var e1: dynamic;
  var e2: dynamic;
  e1.push_back(1);
  e2.push_back(e1);
  dp[1].push_back(e2);
  var i: dynamic;
  var q: dynamic;
  var z: dynamic;
  {
    i = 1;
    while ((i < 10))
    {
      var fk = 1;
      {
        q = 0;
        while ((q < dp[i].size()))
        {
          if ((fk & 1))
          {
            {
              z = 0;
              while ((z < dp[i][q].size()))
              {
                e2 = dp[i][q];
                e2[z].push_back((i + 1));
                dp[(i + 1)].push_back(e2);
                z += 1;
              }
            }
            e2 = dp[i][q];
            e1.clear();
            e1.push_back((i + 1));
            e2.push_back(e1);
            dp[(i + 1)].push_back(e2);
          } else
          {
            e2 = dp[i][q];
            e1.clear();
            e1.push_back((i + 1));
            e2.push_back(e1);
            dp[(i + 1)].push_back(e2);
            {
              z = dp[i][q].size();
              while (cpp_update(z, "--"))
              {
                e2 = dp[i][q];
                e2[z].push_back((i + 1));
                dp[(i + 1)].push_back(e2);
              }
            }
          }
          q += 1;
          fk ^= 1;
        }
      }
      i += 1;
    }
  }
  var n: dynamic;
  while ((cin >> n))
  {
    write(dp[n].size(), "\n");
    {
      q = 0;
      while ((q < dp[n].size()))
      {
        ot(dp[n][q]);
        q += 1;
      }
    }
  }
}
