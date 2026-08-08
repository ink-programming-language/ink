// Translated from solution.cpp.

var mod = cpp_expression("#include <");

var sp = cpp_expression("#in");

var intmax = cpp_expression("#include <");

var llmax = cpp_expression("#include <bits/stdc");

var mkp = cpp_expression("#include");

var N: dynamic;

var DP = cpp_array(4001, 4001);

var R = cpp_array(2001);

var B = cpp_array(2001);

var S: dynamic;

func main()
{
  read(S);
  N = S.size();
  {
    var i = 1;
    while ((i <= N))
    {
      var __cpp_switch_1 = S[(i - 1)];
      if (__cpp_switch_1 == cpp_char("0"))
      {
        R[i] = 2;
        break;
      }
      else if (__cpp_switch_1 == cpp_char("1"))
      {
        R[i] = cpp_assign(B[i], "=", 1);
        break;
      }
      else if (__cpp_switch_1 == cpp_char("2"))
      {
        B[i] = 2;
        break;
      }
      R[i] += R[(i - 1)];
      B[i] += B[(i - 1)];
      i += 1;
    }
  }
  DP[0][0] = 1;
  {
    var i = 0;
    while ((i < (N * 2)))
    {
      {
        var j = max(0, (i - B[min(i, N)]));
        while (((j <= R[min(i, N)]) && (j <= i)))
        {
          if (((j + 1) <= R[min((i + 1), N)]))
          {
            DP[(j + 1)][(i - j)] = (((DP[(j + 1)][(i - j)] + DP[j][(i - j)])) % 998244353);
          }
          if ((((i - j) + 1) <= B[min((i + 1), N)]))
          {
            DP[j][((i - j) + 1)] = (((DP[j][((i - j) + 1)] + DP[j][(i - j)])) % 998244353);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(DP[R[N]][B[N]], "\n");
  return 0;
}
