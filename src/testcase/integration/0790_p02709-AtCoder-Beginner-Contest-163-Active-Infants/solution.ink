// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<n;i++)");
}

func all(v: dynamic)
{
  return cpp_expression("#include<bits/stdc");
}

var dp = cpp_array(2001, 2001);

func main()
{
  var n: dynamic;
  var A = cpp_array(200001);
  read(n);
  rep(i, n);
  read(A[i]);
  var B: dynamic;
  rep(i, n).push_back(P(A[i], i));
  sort(all(B));
  rep(i, n);
  rep(j, (n - i))[(i + 1)][j] = max((dp[i][j] + (abs(((j + i) - B[i].second)) * B[i].first)), (dp[i][(j + 1)] + (abs((j - B[i].second)) * B[i].first)));
  write(dp[n][0], "\n");
}
