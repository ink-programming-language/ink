// Translated from solution.cpp.

var BIG_NUM = cpp_expression("#include<b");

var MOD = cpp_expression("#include<b");

var EPS = cpp_expression("#include<bi");

var NUM = cpp_expression("#i");

var N: dynamic;

var energy = cpp_array(NUM);

var dp = cpp_array(NUM, 321, 321);

func func_cpp()
{
  scanf("%d", (&N));
  var sum = 0;
  {
    var i = 0;
    while ((i < N))
    {
      scanf("%d", (&energy[i]));
      sum += energy[i];
      i += 1;
    }
  }
  if ((N <= 2))
  {
    printf("%d\n", energy[(N - 1)]);
    return;
  }
  {
    var a = 0;
    while ((a <= sum))
    {
      {
        var b = 0;
        while ((b <= sum))
        {
          {
            var c = 0;
            while ((c < NUM))
            {
              dp[a][b][c] = false;
              c += 1;
            }
          }
          b += 1;
        }
      }
      a += 1;
    }
  }
  dp[energy[0]][energy[1]][0] = true;
  var right_value: dynamic;
  {
    var i = 1;
    while ((i <= (N - 2)))
    {
      right_value = energy[(i + 1)];
      {
        var left_value = 0;
        while ((left_value <= sum))
        {
          {
            var self_value = 0;
            while ((self_value <= sum))
            {
              if ((!dp[left_value][self_value][(i - 1)]))
              {
                self_value += 1;
                continue;
              }
              dp[self_value][right_value][i] = true;
              if ((self_value > 0))
              {
                dp[(self_value - 1)][(right_value + left_value)][i] = true;
              }
              self_value += 1;
            }
          }
          left_value += 1;
        }
      }
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 0;
    while ((i <= sum))
    {
      {
        var k = 0;
        while ((k <= sum))
        {
          if (dp[i][k][(N - 2)])
          {
            ans = max(ans, k);
          }
          k += 1;
        }
      }
      i += 1;
    }
  }
  printf("%d\n", ans);
}

func main()
{
  var num_case: dynamic;
  scanf("%d", (&num_case));
  {
    var loop = 0;
    while ((loop < num_case))
    {
      func_cpp();
      loop += 1;
    }
  }
  return 0;
}
