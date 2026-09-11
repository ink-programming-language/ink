// Translated from solution.cpp.

var dp: dynamic = cpp_array(123);

func ot(a: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < a.size()))
    {
      if (i)
      {
        printf(",");
      }
      printf("{");
      {
        var q: dynamic = 0;
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

func main() -> dynamic
{
  var e1: dynamic = cpp_uninitialized();
  var e2: dynamic = cpp_uninitialized();
  e1.push_back(1);
  e2.push_back(e1);
  dp[1].push_back(e2);
  var i: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  var z: dynamic = cpp_uninitialized();
  {
    i = 1;
    while ((i < 10))
    {
      var fk: dynamic = 1;
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
  var n: dynamic = cpp_uninitialized();
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
