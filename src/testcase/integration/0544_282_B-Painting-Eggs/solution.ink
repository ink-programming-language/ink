// Translated from solution.cpp.

var dp = cpp_array(1000005, 3);

var A = cpp_array(1000005);

var B = cpp_array(1000005);

var ma = cpp_array(1000005, 3);

func main()
{
  var n: dynamic;
  var x: dynamic;
  var e1: dynamic;
  var e2: dynamic;
  scanf("%d", (&n));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d%d", (&A[i]), (&B[i]));
      i += 1;
    }
  }
  dp[0][0] = make_pair(A[0], 0);
  dp[1][0] = make_pair(0, B[0]);
  {
    var i = 1;
    while ((i < n))
    {
      e1 = dp[0][(i - 1)];
      e2 = dp[1][(i - 1)];
      if ((abs((((e1.first + A[i])) - e1.second)) <= abs((((e2.first + A[i])) - e2.second))))
      {
        dp[0][i] = make_pair(((e1.first + A[i])), e1.second);
        ma[0][i] = 1;
      } else
      {
        dp[0][i] = make_pair((e2.first + A[i]), e2.second);
        ma[0][i] = 2;
      }
      if ((abs((((e1.second + B[i])) - e1.first)) <= abs((((e2.second + B[i])) - e2.first))))
      {
        dp[1][i] = make_pair(e1.first, (e1.second + B[i]));
        ma[1][i] = 1;
      } else
      {
        dp[1][i] = make_pair(e2.first, (e2.second + B[i]));
        ma[1][i] = 2;
      }
      i += 1;
    }
  }
  if (((abs((dp[0][(n - 1)].first - dp[0][(n - 1)].second)) > 500) && (abs((dp[1][(n - 1)].first - dp[1][(n - 1)].second)) > 500)))
  {
    write("-1", "\n");
    return 0;
  }
  var l: dynamic;
  var idx = (n - 1);
  var padre: dynamic;
  if ((abs((dp[0][(n - 1)].first - dp[0][(n - 1)].second)) <= 500))
  {
    padre = 1;
    while ((idx > 0))
    {
      l.push_back(padre);
      padre = ma[(padre - 1)][cpp_update(idx, "--")];
    }
    l.push_back(padre);
  } else
  {
    padre = 2;
    while ((idx > 0))
    {
      l.push_back(padre);
      padre = ma[(padre - 1)][cpp_update(idx, "--")];
    }
    l.push_back(padre);
  }
  {
    var i = (n - 1);
    while ((i >= 0))
    {
      if ((l[i] == 1))
      {
        write("A");
      } else
      {
        write("G");
      }
      i -= 1;
    }
  }
  write("\n");
  return 0;
}
