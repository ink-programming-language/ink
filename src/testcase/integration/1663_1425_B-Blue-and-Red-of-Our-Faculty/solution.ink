// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

var G: dynamic = cpp_array(2005);

var dp: dynamic = cpp_array(4005, 2005);

var ldp: dynamic = cpp_array(8005, 2005);

var rdp: dynamic = cpp_array(8005, 2005);

var zero: dynamic = 4002;

var used: dynamic = cpp_array(2005);

func dfs(v: dynamic) -> dynamic
{
  used[v] = true;
  {
    var i: dynamic = (0);
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

func main(argument_0: dynamic) -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  read(n, m);
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  {
    var i: dynamic = (1);
    while (((i) <= (m)))
    {
      read(u, v);
      G[u].push_back(v);
      G[v].push_back(u);
      (i) += 1;
    }
  }
  var vec: dynamic = cpp_uninitialized();
  used[1] = true;
  {
    var i: dynamic = (0);
    while (((i) <= ((cpp_cast(G[1].size()) - 1))))
    {
      var v: dynamic = G[1][i];
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
    var i: dynamic = (0);
    while (((i) <= ((c - 1))))
    {
      {
        var j: dynamic = (0);
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
    var i: dynamic = (0);
    while (((i) <= ((c - 1))))
    {
      {
        var j: dynamic = ((-m));
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
    var i: dynamic = (c + 1);
    while ((i >= 2))
    {
      {
        var j: dynamic = ((-m));
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
  var ans: dynamic = 0;
  if (((m % 2) == 0))
  {
    ans += dp[c][(m / 2)];
  }
  {
    var i: dynamic = (1);
    while (((i) <= (c)))
    {
      {
        var j: dynamic = (vec[(i - 1)] - 2);
        while ((j >= ((-vec[(i - 1)]) + 2)))
        {
          {
            var k: dynamic = ((-m));
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
      var i: dynamic = (0);
      while (((i) <= ((c + 1))))
      {
        {
          var j: dynamic = ((-m));
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
      var i: dynamic = (0);
      while (((i) <= ((c - 1))))
      {
        {
          var j: dynamic = ((-m));
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
      var i: dynamic = (c + 1);
      while ((i >= 2))
      {
        {
          var j: dynamic = ((-m));
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
      var i: dynamic = (1);
      while (((i) <= (c)))
      {
        var j: dynamic = (vec[(i - 1)] - 1);
        {
          var k: dynamic = ((-m));
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
          var k: dynamic = ((-m));
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
