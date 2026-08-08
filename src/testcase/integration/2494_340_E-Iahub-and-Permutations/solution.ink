// Translated from solution.cpp.

var dp = cpp_array(2010, 2010);

var fact = cpp_array(4010);

func solve(n: dynamic, k: dynamic)
{
  if (cpp_binary((k < 0), "or", (n < 0)))
  {
    return 0;
  } else if ((k == 0))
  {
    return fact[n];
  } else if ((dp[n][k] != -1))
  {
    return dp[n][k];
  }
  var ret = 0;
  ret += (n * solve(n, (k - 1)));
  ret += (((k - 1)) * solve((n + 1), (k - 2)));
  ret %= 1000000007;
  return (cpp_assign(dp[n][k], "=", ret));
}

func main()
{
  memset(dp, -1, cpp_sizeof((dp)));
  fact[0] = 1;
  {
    var i = 1;
    while ((i < 4000))
    {
      fact[i] = (((fact[(i - 1)] * i)) % 1000000007);
      i += 1;
    }
  }
  var N: dynamic;
  read(N);
  var used: dynamic;
  var a = 0;
  var b = 0;
  var arr: dynamic;
  {
    var i = 0;
    while ((i < N))
    {
      var x: dynamic;
      read(x);
      arr.push_back(x);
      if ((x != -1))
      {
        used.insert(x);
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= N))
    {
      if ((used.find(i) != used.end()))
      {
        i += 1;
        continue;
      }
      if ((arr[(i - 1)] == -1))
      {
        b += 1;
      } else
      {
        a += 1;
      }
      i += 1;
    }
  }
  write(solve(a, b), "\n");
  return 0;
}
