// Translated from solution.cpp.

var pb: dynamic = cpp_expression("#include");

func fr(i: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  cpp_macro("for(ll i=l;i<=r;i++)");
}

func rf(i: dynamic, r: dynamic, l: dynamic) -> dynamic
{
  cpp_macro("for(ll i=r;i>=l;i--)");
}

func done(i: dynamic) -> dynamic
{
  cpp_macro("cout<<\"done = \"<<i<<endl;");
}

func show(x: dynamic, y: dynamic) -> dynamic
{
  cpp_macro("cout<<x<<\" : \";for(auto z:y)cout<<z<<\" \";cout<<endl;");
}

var fast: dynamic = cpp_expression("#include<bits/stdc++.h> using na");

func all(x: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/st");
}

var yes: dynamic = cpp_expression("#include<bits/st");

var no: dynamic = cpp_expression("#include<bits/s");

var dp: dynamic = cpp_array(5005, 5005);

var inf: dynamic = 1e18;

func Test() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  var X: dynamic = cpp_uninitialized();
  var Y: dynamic = cpp_uninitialized();
  X.pb(0);
  Y.pb(0);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var x: dynamic = cpp_uninitialized();
      read(x);
      if (x)
      {
        X.pb(i);
      } else
      {
        Y.pb(i);
      }
      i += 1;
    }
  }
  fr(i, 1, n);
  fr(j, 1, n);
  {
    dp[i][j] = inf;
    dp[i][0] = inf;
  }
  dp[0][0] = 0;
  {
    var i: dynamic = 1;
    while ((i < X.size()))
    {
      {
        var j: dynamic = 1;
        while ((j < Y.size()))
        {
          var cost: dynamic = abs((X[i] - Y[j]));
          dp[i][j] = min(dp[i][(j - 1)], (dp[(i - 1)][(j - 1)] + cost));
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(dp[(X.size() - 1)][(Y.size() - 1)], "\n");
}

func main() -> dynamic
{
  var t: dynamic = 1;
  while (cpp_update(t, "--"))
  {
    Test();
  }
}
