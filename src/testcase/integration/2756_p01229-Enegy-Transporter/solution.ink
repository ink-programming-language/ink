// Translated from solution.cpp.

var BIG_NUM: dynamic = cpp_expression("#include<b");

var MOD: dynamic = cpp_expression("#include<b");

var EPS: dynamic = cpp_expression("#include<bi");

var NUM: dynamic = cpp_expression("#i");

var N: dynamic = cpp_uninitialized();

var energy: dynamic = cpp_array(NUM);

var dp: dynamic = cpp_array(NUM, 321, 321);

func func_cpp() -> dynamic
{
  scanf("%d", (&N));
  var sum: dynamic = 0;
  {
    var i: dynamic = 0;
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
    var a: dynamic = 0;
    while ((a <= sum))
    {
      {
        var b: dynamic = 0;
        while ((b <= sum))
        {
          {
            var c: dynamic = 0;
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
  var right_value: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i <= (N - 2)))
    {
      right_value = energy[(i + 1)];
      {
        var left_value: dynamic = 0;
        while ((left_value <= sum))
        {
          {
            var self_value: dynamic = 0;
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
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i <= sum))
    {
      {
        var k: dynamic = 0;
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

func main() -> dynamic
{
  var num_case: dynamic = cpp_uninitialized();
  scanf("%d", (&num_case));
  {
    var loop: dynamic = 0;
    while ((loop < num_case))
    {
      func_cpp();
      loop += 1;
    }
  }
  return 0;
}
