// Translated from solution.cpp.

func mod(n: dynamic, m: dynamic) -> dynamic
{
  var ret: dynamic = (n % m);
  if ((ret < 0))
  {
    ret += m;
  }
  return ret;
}

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  return ( ((b == 0)) ? a : gcd(b, (a % b)));
}

func exp(a: dynamic, b: dynamic, m: dynamic) -> dynamic
{
  if ((b == 0))
  {
    return 1;
  }
  if ((b == 1))
  {
    return mod(a, m);
  }
  var k: dynamic = mod(exp(a, (b / 2), m), m);
  if ((b & 1))
  {
    return mod((a * mod((k * k), m)), m);
  } else
  {
    return mod((k * k), m);
  }
}

var N: dynamic = 710;

var G: dynamic = cpp_array(N, N);

var dp: dynamic = cpp_array(2, N, N);

func solve(l: dynamic, r: dynamic, f: dynamic) -> dynamic
{
  if ((l > r))
  {
    return 1;
  }
  if ((l == r))
  {
    var root: dynamic = ( ((f == 0)) ? (l - 1) : (r + 1));
    if ((G[root][l] > 1))
    {
      return 1;
    }
    return 0;
  }
  var x: dynamic = dp[l][r][f];
  if ((x != -1))
  {
    return x;
  }
  x = 0;
  var root: dynamic = ( ((f == 0)) ? (l - 1) : (r + 1));
  {
    var k: dynamic = l;
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

var v: dynamic = cpp_array(N);

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 1;
    while ((i < ((n + 1))))
    {
      read(v[i]);
      i += 1;
    }
  }
  sort((v + 1), ((v + 1) + n));
  {
    var i: dynamic = 1;
    while ((i < ((n + 1))))
    {
      {
        var j: dynamic = 1;
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
  var ok: dynamic = 0;
  {
    var k: dynamic = 1;
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
