// Translated from solution.cpp.

var adj = cpp_construct(100001);

var m: dynamic;

var subtreeSize = cpp_array(100001);

var dp = cpp_array(202, 100001);

var s = cpp_array(202, 202);

var c = cpp_array(202);

var fact = cpp_array(202);

func DFS(i: dynamic, p: dynamic = 0)
{
  dp[i][0] = 2;
  subtreeSize[i] = 1;
  for (var j in adj[i])
  {
    if ((j == p))
    {
      continue;
    }
    DFS(j, i);
    {
      var z = min(m, ((subtreeSize[i] + subtreeSize[j]) - 1));
      while ((z > -1))
      {
        var val = 0;
        {
          var x = min(z, (subtreeSize[i] - 1));
          var y = (z - x);
          while (((x > -1) && (y <= subtreeSize[j])))
          {
            val = (((val + (dp[i][x] * ((dp[j][y] + (if (((y > 0))) ((dp[j][(y - 1)] - ((y == 1)))) else 0)))))) % 1000000007);
            x -= 1;
            y += 1;
          }
        }
        dp[i][z] = val;
        z -= 1;
      }
    }
    subtreeSize[i] += subtreeSize[j];
  }
  {
    var x = min(m, subtreeSize[i]);
    while ((x > -1))
    {
      c[x] = (((c[x] + dp[i][x])) % 1000000007);
      x -= 1;
    }
  }
  {
    var x = min(m, subtreeSize[i]);
    while (((p != 0) && (x > -1)))
    {
      c[x] = (((c[x] - ((dp[i][x] + (if (((x > 0))) ((dp[i][(x - 1)] - ((x == 1)))) else 0))))) % 1000000007);
      x -= 1;
    }
  }
}

func Initialise()
{
  s[1][1] = 1;
  {
    var i = 2;
    while ((i <= m))
    {
      {
        var j = 1;
        while ((j <= i))
        {
          s[i][j] = (((s[(i - 1)][(j - 1)] + (j * s[(i - 1)][j]))) % 1000000007);
          j += 1;
        }
      }
      i += 1;
    }
  }
  fact[0] = 1;
  {
    var i = 1;
    while ((i <= m))
    {
      fact[i] = (((i * fact[(i - 1)])) % 1000000007);
      i += 1;
    }
  }
}

func main()
{
  var n: dynamic;
  scanf("%d%d", (&n), (&m));
  {
    var x = 1;
    while ((x < n))
    {
      var i: dynamic;
      var j: dynamic;
      scanf("%d%d", (&i), (&j));
      adj[i].push_back(j);
      adj[j].push_back(i);
      x += 1;
    }
  }
  Initialise();
  DFS(1);
  var ans = 0;
  {
    var x = 1;
    while ((x <= m))
    {
      var ansx = (((s[m][x] * fact[x])) % 1000000007);
      ansx = (((ansx * c[x])) % 1000000007);
      ans = ((((ans + 1000000007) + ansx)) % 1000000007);
      x += 1;
    }
  }
  printf("%lld\n", ans);
  return 0;
}
