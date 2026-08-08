// Translated from solution.cpp.

var INF = cpp_expression("#include <iostream>");

var H: dynamic;

var W: dynamic;

var a_max: dynamic;

var a_min: dynamic;

var ary = cpp_array(2010, 2010);

var lma = cpp_array(2010, 2010);

var lmi = cpp_array(2010, 2010);

var rma = cpp_array(2010, 2010);

var rmi = cpp_array(2010, 2010);

var dp = cpp_array(2010, 2);

func amax(a: dynamic, b: dynamic)
{
  if ((abs(a) > ((INF / 2))))
  {
    a = 0;
  }
  if ((abs(b) > ((INF / 2))))
  {
    b = 0;
  }
  return max(abs(a), abs(b));
}

func amin(a: dynamic, b: dynamic)
{
  return min(abs(a), abs(b));
}

func main()
{
  read(H, W);
  a_min = INF;
  a_max = 0;
  {
    var i = 1;
    while ((i <= H))
    {
      {
        var j = 1;
        while ((j <= W))
        {
          read(ary[i][j]);
          a_min = min(a_min, ary[i][j]);
          a_max = max(a_max, ary[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= H))
    {
      lma[i][0] = 0;
      lmi[i][0] = INF;
      {
        var j = 1;
        while ((j <= W))
        {
          lma[i][j] = max(lma[i][(j - 1)], ary[i][j]);
          lmi[i][j] = min(lmi[i][(j - 1)], ary[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= H))
    {
      rma[i][(W + 1)] = 0;
      rmi[i][(W + 1)] = INF;
      {
        var j = W;
        while ((j >= 1))
        {
          rma[i][j] = max(rma[i][(j + 1)], ary[i][j]);
          rmi[i][j] = min(rmi[i][(j + 1)], ary[i][j]);
          j -= 1;
        }
      }
      i += 1;
    }
  }
  var res = INF;
  var in_cpp: dynamic;
  var ou: dynamic;
  in_cpp = 0;
  ou = 1;
  fill(dp[in_cpp], (dp[in_cpp] + 2010), INF);
  {
    var i = 0;
    while ((i <= W))
    {
      dp[in_cpp][i] = amax((a_max - lmi[1][i]), (rma[1][(i + 1)] - a_min));
      i += 1;
    }
  }
  {
    var i = 2;
    while ((i <= H))
    {
      var nowmin = INF;
      {
        var j = 0;
        while ((j <= W))
        {
          nowmin = min(nowmin, dp[in_cpp][j]);
          dp[ou][j] = max(nowmin, amax((a_max - lmi[i][j]), (rma[i][(j + 1)] - a_min)));
          j += 1;
        }
      }
      swap(in_cpp, ou);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= W))
    {
      res = min(res, dp[in_cpp][i]);
      i += 1;
    }
  }
  in_cpp = 0;
  ou = 1;
  fill(dp[in_cpp], (dp[in_cpp] + 2010), INF);
  {
    var i = 0;
    while ((i <= W))
    {
      dp[in_cpp][i] = amax((a_max - rmi[1][(i + 1)]), (lma[1][i] - a_min));
      i += 1;
    }
  }
  {
    var i = 2;
    while ((i <= H))
    {
      var nowmin = INF;
      {
        var j = 0;
        while ((j <= W))
        {
          nowmin = min(nowmin, dp[in_cpp][j]);
          dp[ou][j] = max(nowmin, amax((a_max - rmi[i][(j + 1)]), (lma[i][j] - a_min)));
          j += 1;
        }
      }
      swap(in_cpp, ou);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= W))
    {
      res = min(res, dp[in_cpp][i]);
      i += 1;
    }
  }
  in_cpp = 0;
  ou = 1;
  fill(dp[in_cpp], (dp[in_cpp] + 2010), INF);
  {
    var i = 0;
    while ((i <= W))
    {
      dp[in_cpp][i] = amax((a_max - lmi[H][i]), (rma[H][(i + 1)] - a_min));
      i += 1;
    }
  }
  {
    var i = (H - 1);
    while ((i >= 1))
    {
      var nowmin = INF;
      {
        var j = 0;
        while ((j <= W))
        {
          nowmin = min(nowmin, dp[in_cpp][j]);
          dp[ou][j] = max(nowmin, amax((a_max - lmi[i][j]), (rma[i][(j + 1)] - a_min)));
          j += 1;
        }
      }
      swap(in_cpp, ou);
      i -= 1;
    }
  }
  {
    var i = 0;
    while ((i <= W))
    {
      res = min(res, dp[in_cpp][i]);
      i += 1;
    }
  }
  in_cpp = 0;
  ou = 1;
  fill(dp[in_cpp], (dp[in_cpp] + 2010), INF);
  {
    var i = 0;
    while ((i <= W))
    {
      dp[in_cpp][i] = amax((a_max - rmi[H][(i + 1)]), (lma[H][i] - a_min));
      i += 1;
    }
  }
  {
    var i = (H - 1);
    while ((i >= 1))
    {
      var nowmin = INF;
      {
        var j = 0;
        while ((j <= W))
        {
          nowmin = min(nowmin, dp[in_cpp][j]);
          dp[ou][j] = max(nowmin, amax((a_max - rmi[i][(j + 1)]), (lma[i][j] - a_min)));
          j += 1;
        }
      }
      swap(in_cpp, ou);
      i -= 1;
    }
  }
  {
    var i = 0;
    while ((i <= W))
    {
      res = min(res, dp[in_cpp][i]);
      i += 1;
    }
  }
  write(res, "\n");
  return 0;
}
