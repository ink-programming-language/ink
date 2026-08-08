// Translated from solution.cpp.

func mod(n: dynamic, m: dynamic)
{
  var ret = (n % m);
  if ((ret < 0))
  {
    ret += m;
  }
  return ret;
}

func gcd(a: dynamic, b: dynamic)
{
  return (if ((b == 0)) a else gcd(b, (a % b)));
}

func exp(a: dynamic, b: dynamic, m: dynamic)
{
  if ((b == 0))
  {
    return 1;
  }
  if ((b == 1))
  {
    return mod(a, m);
  }
  var k = mod(exp(a, (b / 2), m), m);
  if ((b & 1))
  {
    return mod((a * mod((k * k), m)), m);
  } else
  {
    return mod((k * k), m);
  }
}

var N = 710;

var G = cpp_array(N, N);

var dp = cpp_array(2, N, N);

func solve(l: dynamic, r: dynamic, f: dynamic)
{
  if ((l > r))
  {
    return 1;
  }
  if ((l == r))
  {
    var root = (if ((f == 0)) (l - 1) else (r + 1));
    if ((G[root][l] > 1))
    {
      return 1;
    }
    return 0;
  }
  var x = dp[l][r][f];
  if ((x != -1))
  {
    return x;
  }
  x = 0;
  var root = (if ((f == 0)) (l - 1) else (r + 1));
  {
    var k = l;
    while ((k <= r))
    {
      if ((G[root][k] > 1))
      {
        x |= ((solve(l, (k - 1), 1) & solve((k + 1), r, 0)));
      }
      k += 1;
    }
  }
  return x;
}

var v = cpp_array(N);

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic;
  read(n);
  {
    var i = 1;
    while ((i < ((n + 1))))
    {
      read(v[i]);
      i += 1;
    }
  }
  sort((v + 1), ((v + 1) + n));
  {
    var i = 1;
    while ((i < ((n + 1))))
    {
      {
        var j = 1;
        while ((j < ((n + 1))))
        {
          G[i][j] = gcd(v[i], v[j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  memset(dp, -1, cpp_sizeof((dp)));
  var ok = 0;
  {
    var k = 1;
    while ((k <= n))
    {
      ok |= ((solve(1, (k - 1), 1) & solve((k + 1), n, 0)));
      k += 1;
    }
  }
  if (ok)
  {
    write("Yes", "\n");
  } else
  {
    write("No", "\n");
  }
}
