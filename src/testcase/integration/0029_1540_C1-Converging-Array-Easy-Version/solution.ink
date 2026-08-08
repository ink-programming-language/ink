// Translated from solution.cpp.

var pii = cpp_expression("#include<bits");

var fi = cpp_expression("#incl");

var sc = cpp_expression("#inclu");

var pb = cpp_expression("#include<");

var ll = dynamic;

func trav(v: dynamic, x: dynamic)
{
  return cpp_expression("#include<bits");
}

func all(x: dynamic)
{
  return cpp_expression("#include<bits/stdc++.h");
}

var VI = cpp_expression("#include<bi");

var VLL = cpp_expression("#include<b");

var N = (1e6 + 100);

var mod = (1e9 + 7);

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  var n: dynamic;
  read(n);
  var mx = cpp_construct((n + 1));
  {
    var i = 1;
    while ((i <= n))
    {
      read(mx[i]);
      i += 1;
    }
  }
  var b = cpp_construct((n + 1));
  {
    var i = 1;
    while ((i < n))
    {
      read(b[i]);
      b[i] += b[(i - 1)];
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < n))
    {
      b[i] += b[(i - 1)];
      i += 1;
    }
  }
  var dp = cpp_construct((n + 1), VLL(10010, 0));
  var ans = 0;
  var x: dynamic;
  read(x);
  read(x);
  {
    dp[0][0] = 1;
    var suma = 0;
    {
      var i = 1;
      while ((i <= n))
      {
        fill(all(dp[i]), 0);
        {
          var val = 0;
          while ((val <= mx[i]))
          {
            var mn = ((b[(i - 1)] + (x * i)) - val);
            mn = max(mn, 0);
            {
              var bf = mn;
              while ((bf <= suma))
              {
                dp[i][(val + bf)] += dp[(i - 1)][bf];
                if ((dp[i][(val + bf)] >= mod))
                {
                  dp[i][(val + bf)] -= mod;
                }
                bf += 1;
              }
            }
            val += 1;
          }
        }
        suma += mx[i];
        i += 1;
      }
    }
    ans = 0;
    {
      var i = 0;
      while ((i <= suma))
      {
        ans += dp[n][i];
        if ((ans >= mod))
        {
          ans -= mod;
        }
        i += 1;
      }
    }
    write(ans, cpp_char("\n"));
  }
}
