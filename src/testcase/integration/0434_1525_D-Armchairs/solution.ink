// Translated from solution.cpp.

var pb = cpp_expression("#include");

func fr(i: dynamic, l: dynamic, r: dynamic)
{
  cpp_macro("for(ll i=l;i<=r;i++)");
}

func rf(i: dynamic, r: dynamic, l: dynamic)
{
  cpp_macro("for(ll i=r;i>=l;i--)");
}

func done(i: dynamic)
{
  cpp_macro("cout<<\"done = \"<<i<<endl;");
}

func show(x: dynamic, y: dynamic)
{
  cpp_macro("cout<<x<<\" : \";for(auto z:y)cout<<z<<\" \";cout<<endl;");
}

var fast = cpp_expression("#include<bits/stdc++.h> using na");

func all(x: dynamic)
{
  return cpp_expression("#include<bits/st");
}

var yes = cpp_expression("#include<bits/st");

var no = cpp_expression("#include<bits/s");

var dp = cpp_array(5005, 5005);

var inf = 1e18;

func Test()
{
  var n: dynamic;
  read(n);
  var X: dynamic;
  var Y: dynamic;
  X.pb(0);
  Y.pb(0);
  {
    var i = 1;
    while ((i <= n))
    {
      var x: dynamic;
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
    var i = 1;
    while ((i < X.size()))
    {
      {
        var j = 1;
        while ((j < Y.size()))
        {
          var cost = abs((X[i] - Y[j]));
          dp[i][j] = min(dp[i][(j - 1)], (dp[(i - 1)][(j - 1)] + cost));
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(dp[(X.size() - 1)][(Y.size() - 1)], "\n");
}

func main()
{
  var t = 1;
  while (cpp_update(t, "--"))
  {
    Test();
  }
}
