// Translated from solution.cpp.

var INF: dynamic = cpp_expression("#includ");

func all(c: dynamic) -> dynamic
{
  return cpp_expression("#include<vector> #inc");
}

func D(x: dynamic) -> dynamic
{
  return cpp_expression("#include<vector> #include<iostre");
}

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i = 0; i < n; i++)");
}

var x: dynamic = cpp_expression("#incl");

var y: dynamic = cpp_expression("#inclu");

var dp: dynamic = cpp_array(1002, 1002);

var K: dynamic = cpp_array(1002, 1002);

func main(argument_0: dynamic) -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  rep(i, n);
  read(v[i].x, v[i].y);
  fill(dp[0], dp[1001], INT_MAX);
  fill(K[0], K[1001], -1);
  rep(i, 1001)[i][i] = 0;
  K[i][i] = i;
  {
    var w: dynamic = 1;
    while ((w < n))
    {
      {
        var i: dynamic = 0;
        while (((i + w) <= n))
        {
          {
            var s: dynamic = K[i][((i + w) - 1)];
            while ((s <= K[(i + 1)][(i + w)]))
            {
              var cost: dynamic = (abs((v[i].x - v[(s + 1)].x)) + abs((v[s].y - v[(i + w)].y)));
              if ((dp[i][(i + w)] > ((dp[i][s] + dp[(s + 1)][(i + w)]) + cost)))
              {
                dp[i][(i + w)] = ((dp[i][s] + dp[(s + 1)][(i + w)]) + cost);
                K[i][(i + w)] = s;
              }
              s += 1;
            }
          }
          i += 1;
        }
      }
      w += 1;
    }
  }
  write(dp[0][(n - 1)], "\n");
  return 0;
}
