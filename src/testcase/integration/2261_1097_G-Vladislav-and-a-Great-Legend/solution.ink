// Translated from solution.cpp.

var adj: dynamic = cpp_construct(100001);

var m: dynamic = cpp_uninitialized();

var subtreeSize: dynamic = cpp_array(100001);

var dp: dynamic = cpp_array(202, 100001);

var s: dynamic = cpp_array(202, 202);

var c: dynamic = cpp_array(202);

var fact: dynamic = cpp_array(202);

func DFS(i: dynamic, p: dynamic = 0) -> dynamic
{
  dp[i][0] = 2;
  subtreeSize[i] = 1;
  for (var j: dynamic in adj[i])
  {
    if ((j == p))
    {
      continue;
    }
    DFS(j, i);
    {
      var z: dynamic = min(m, ((subtreeSize[i] + subtreeSize[j]) - 1));
      while ((z > -1))
      {
        var val: dynamic = 0;
        {
          var x: dynamic = min(z, (subtreeSize[i] - 1));
          var y: dynamic = (z - x);
          while (((x > -1) && (y <= subtreeSize[j])))
          {
            val = (((val + (dp[i][x] * ((dp[j][y] + ( (((y > 0))) ? ((dp[j][(y - 1)] - ((y == 1)))) : 0)))))) % 1000000007);
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
    var x: dynamic = min(m, subtreeSize[i]);
    while ((x > -1))
    {
      c[x] = (((c[x] + dp[i][x])) % 1000000007);
      x -= 1;
    }
  }
  {
    var x: dynamic = min(m, subtreeSize[i]);
    while (((p != 0) && (x > -1)))
    {
      c[x] = (((c[x] - ((dp[i][x] + ( (((x > 0))) ? ((dp[i][(x - 1)] - ((x == 1)))) : 0))))) % 1000000007);
      x -= 1;
    }
  }
}

func Initialise() -> dynamic
{
  s[1][1] = 1;
  {
    var i: dynamic = 2;
    while ((i <= m))
    {
      {
        var j: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= m))
    {
      fact[i] = (((i * fact[(i - 1)])) % 1000000007);
      i += 1;
    }
  }
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d%d", (&n), (&m));
  {
    var x: dynamic = 1;
    while ((x < n))
    {
      var i: dynamic = cpp_uninitialized();
      var j: dynamic = cpp_uninitialized();
      scanf("%d%d", (&i), (&j));
      adj[i].push_back(j);
      adj[j].push_back(i);
      x += 1;
    }
  }
  Initialise();
  DFS(1);
  var ans: dynamic = 0;
  {
    var x: dynamic = 1;
    while ((x <= m))
    {
      var ansx: dynamic = (((s[m][x] * fact[x])) % 1000000007);
      ansx = (((ansx * c[x])) % 1000000007);
      ans = ((((ans + 1000000007) + ansx)) % 1000000007);
      x += 1;
    }
  }
  printf("%lld\n", ans);
  return 0;
}
