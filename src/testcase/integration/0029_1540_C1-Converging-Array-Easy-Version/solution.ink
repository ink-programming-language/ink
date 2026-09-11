// Translated from solution.cpp.

var pii: dynamic = cpp_expression("#include<bits");

var fi: dynamic = cpp_expression("#incl");

var sc: dynamic = cpp_expression("#inclu");

var pb: dynamic = cpp_expression("#include<");

var ll: dynamic = dynamic;

func trav(v: dynamic, x: dynamic) -> dynamic
{
  return cpp_expression("#include<bits");
}

func all(x: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++.h");
}

var VI: dynamic = cpp_expression("#include<bi");

var VLL: dynamic = cpp_expression("#include<b");

var N: dynamic = (1e6 + 100);

var mod: dynamic = (1e9 + 7);

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  var n: dynamic = cpp_uninitialized();
  read(n);
  var mx: dynamic = cpp_construct((n + 1));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(mx[i]);
      i += 1;
    }
  }
  var b: dynamic = cpp_construct((n + 1));
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      read(b[i]);
      b[i] += b[(i - 1)];
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      b[i] += b[(i - 1)];
      i += 1;
    }
  }
  var dp: dynamic = cpp_construct((n + 1), VLL(10010, 0));
  var ans: dynamic = 0;
  var x: dynamic = cpp_uninitialized();
  read(x);
  read(x);
  {
    dp[0][0] = 1;
    var suma: dynamic = 0;
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        fill(all(dp[i]), 0);
        {
          var val: dynamic = 0;
          while ((val <= mx[i]))
          {
            var mn: dynamic = ((b[(i - 1)] + (x * i)) - val);
            mn = max(mn, 0);
            {
              var bf: dynamic = mn;
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
      var i: dynamic = 0;
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
