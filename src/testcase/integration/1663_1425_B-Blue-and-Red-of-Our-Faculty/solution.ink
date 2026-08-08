// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var c: dynamic;

var G = cpp_array(2005);

var dp = cpp_array(4005, 2005);

var ldp = cpp_array(8005, 2005);

var rdp = cpp_array(8005, 2005);

var zero = 4002;

var used = cpp_array(2005);

func dfs(v: dynamic)
{
  used[v] = true;
  {
    var i = (0);
    while (((i) <= ((cpp_cast(G[v].size()) - 1))))
    {
      if (used[G[v][i]])
      {
        (i) += 1;
        continue;
      }
      return (dfs(G[v][i]) + 1);
      (i) += 1;
    }
  }
  return 1;
}

func main(argument_0: dynamic)
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  read(n, m);
  var u: dynamic;
  var v: dynamic;
  {
    var i = (1);
    while (((i) <= (m)))
    {
      read(u, v);
      G[u].push_back(v);
      G[v].push_back(u);
      (i) += 1;
    }
  }
  var vec: dynamic;
  used[1] = true;
  {
    var i = (0);
    while (((i) <= ((cpp_cast(G[1].size()) - 1))))
    {
      var v = G[1][i];
      if (used[v])
      {
        (i) += 1;
        continue;
      }
      vec.push_back((dfs(v) + 1));
      (i) += 1;
    }
  }
  c = vec.size();
  dp[0][0] = 1;
  {
    var i = (0);
    while (((i) <= ((c - 1))))
    {
      {
        var j = (0);
        while (((j) <= (m)))
        {
          (cpp_assign(dp[(i + 1)][j], "+=", dp[i][j])) %= 1000000007;
          if (((j + vec[i]) <= m))
          {
            (cpp_assign(dp[(i + 1)][(j + vec[i])], "+=", dp[i][j])) %= 1000000007;
          }
          (j) += 1;
        }
      }
      (i) += 1;
    }
  }
  ldp[0][zero] = 1;
  {
    var i = (0);
    while (((i) <= ((c - 1))))
    {
      {
        var j = ((-m));
        while (((j) <= (m)))
        {
          (cpp_assign(ldp[(i + 1)][(zero + j)], "+=", ldp[i][(zero + j)])) %= 1000000007;
          if (((j + vec[i]) <= m))
          {
            (cpp_assign(ldp[(i + 1)][((zero + j) + vec[i])], "+=", ldp[i][(zero + j)])) %= 1000000007;
          }
          if (((j - vec[i]) >= (-m)))
          {
            (cpp_assign(ldp[(i + 1)][((zero + j) - vec[i])], "+=", ldp[i][(zero + j)])) %= 1000000007;
          }
          (j) += 1;
        }
      }
      (i) += 1;
    }
  }
  rdp[(c + 1)][zero] = 1;
  {
    var i = (c + 1);
    while ((i >= 2))
    {
      {
        var j = ((-m));
        while (((j) <= (m)))
        {
          (cpp_assign(rdp[(i - 1)][(zero + j)], "+=", rdp[i][(zero + j)])) %= 1000000007;
          if (((j + vec[(i - 2)]) <= m))
          {
            (cpp_assign(rdp[(i - 1)][((zero + j) + vec[(i - 2)])], "+=", rdp[i][(zero + j)])) %= 1000000007;
          }
          if (((j - vec[(i - 2)]) >= (-m)))
          {
            (cpp_assign(rdp[(i - 1)][((zero + j) - vec[(i - 2)])], "+=", rdp[i][(zero + j)])) %= 1000000007;
          }
          (j) += 1;
        }
      }
      i -= 1;
    }
  }
  var ans = 0;
  if (((m % 2) == 0))
  {
    ans += dp[c][(m / 2)];
  }
  {
    var i = (1);
    while (((i) <= (c)))
    {
      {
        var j = (vec[(i - 1)] - 2);
        while ((j >= ((-vec[(i - 1)]) + 2)))
        {
          {
            var k = ((-m));
            while (((k) <= (m)))
            {
              if ((((k + j) < (-m)) || ((k + j) > m)))
              {
                (k) += 1;
                continue;
              }
              ans += ((((cpp_cast(ldp[(i - 1)][(zero + k)]) * cpp_cast(rdp[(i + 1)][((zero + k) + j)])) % 1000000007) * 2) % 1000000007);
              ans %= 1000000007;
              (k) += 1;
            }
          }
          j -= 1;
        }
      }
      (i) += 1;
    }
  }
  if ((m % 2))
  {
    {
      var i = (0);
      while (((i) <= ((c + 1))))
      {
        {
          var j = ((-m));
          while (((j) <= (m)))
          {
            ldp[i][(zero + j)] = cpp_assign(rdp[i][(zero + j)], "=", 0);
            (j) += 1;
          }
        }
        (i) += 1;
      }
    }
    ldp[0][zero] = 1;
    {
      var i = (0);
      while (((i) <= ((c - 1))))
      {
        {
          var j = ((-m));
          while (((j) <= (m)))
          {
            if (((j + vec[i]) <= m))
            {
              (cpp_assign(ldp[(i + 1)][((zero + j) + vec[i])], "+=", ldp[i][(zero + j)])) %= 1000000007;
            }
            if (((j - vec[i]) >= (-m)))
            {
              (cpp_assign(ldp[(i + 1)][((zero + j) - vec[i])], "+=", ldp[i][(zero + j)])) %= 1000000007;
            }
            (j) += 1;
          }
        }
        (i) += 1;
      }
    }
    rdp[(c + 1)][zero] = 1;
    {
      var i = (c + 1);
      while ((i >= 2))
      {
        {
          var j = ((-m));
          while (((j) <= (m)))
          {
            if (((j + vec[(i - 2)]) <= m))
            {
              (cpp_assign(rdp[(i - 1)][((zero + j) + vec[(i - 2)])], "+=", rdp[i][(zero + j)])) %= 1000000007;
            }
            if (((j - vec[(i - 2)]) >= (-m)))
            {
              (cpp_assign(rdp[(i - 1)][((zero + j) - vec[(i - 2)])], "+=", rdp[i][(zero + j)])) %= 1000000007;
            }
            (j) += 1;
          }
        }
        i -= 1;
      }
    }
    {
      var i = (1);
      while (((i) <= (c)))
      {
        var j = (vec[(i - 1)] - 1);
        {
          var k = ((-m));
          while (((k) <= (m)))
          {
            if ((((k + j) < (-m)) || ((k + j) > m)))
            {
              (k) += 1;
              continue;
            }
            ans += ((((cpp_cast(ldp[(i - 1)][(zero + k)]) * cpp_cast(rdp[(i + 1)][((zero + k) + j)])) % 1000000007) * 2) % 1000000007);
            ans %= 1000000007;
            (k) += 1;
          }
        }
        j = ((-vec[(i - 1)]) + 1);
        {
          var k = ((-m));
          while (((k) <= (m)))
          {
            if ((((k + j) < (-m)) || ((k + j) > m)))
            {
              (k) += 1;
              continue;
            }
            ans += ((((cpp_cast(ldp[(i - 1)][(zero + k)]) * cpp_cast(rdp[(i + 1)][((zero + k) + j)])) % 1000000007) * 2) % 1000000007);
            ans %= 1000000007;
            (k) += 1;
          }
        }
        (i) += 1;
      }
    }
  }
  write(ans, "\n");
  return 0;
}
