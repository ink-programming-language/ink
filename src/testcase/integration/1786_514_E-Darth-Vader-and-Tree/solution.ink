// Translated from solution.cpp.

var n: dynamic;

var x: dynamic;

var MAXN = 222222;

var mod = (1e9 + 7);

var a = cpp_array(MAXN);

var cnt = [0];

var dp = [0];

func preProcess()
{
  dp[0] = 1;
  {
    var i = 1;
    while ((i <= 100))
    {
      {
        var j = 0;
        while ((j < i))
        {
          dp[i] += (dp[j] * cnt[(i - j)]);
          dp[i] %= mod;
          j += 1;
        }
      }
      i += 1;
    }
  }
}

func makeMatrix(r: dynamic, c: dynamic, m: dynamic)
{
  m.resize(r);
  {
    var i = 0;
    while ((i < r))
    {
      m[i].resize(c);
      i += 1;
    }
  }
}

func unit(r: dynamic, c: dynamic)
{
  var temp: dynamic;
  makeMatrix(r, c, temp);
  {
    var i = 0;
    while ((i < r))
    {
      {
        var j = 0;
        while ((j < c))
        {
          temp[i][j] = if (((i == j))) 1 else 0;
          j += 1;
        }
      }
      i += 1;
    }
  }
  return temp;
}

func mul(a: dynamic, b: dynamic)
{
  var temp: dynamic;
  makeMatrix(cpp_cast(a.size()), cpp_cast(b[0].size()), temp);
  {
    var i = 0;
    while ((i < a.size()))
    {
      {
        var j = 0;
        while ((j < a[0].size()))
        {
          {
            var k = 0;
            while ((k < b.size()))
            {
              temp[i][j] += (a[i][k] * b[k][j]);
              temp[i][j] %= mod;
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return temp;
}

func Pow(n: dynamic, c: dynamic)
{
  var u: dynamic;
  u = unit(101, 101);
  var i = 0;
  while (n)
  {
    if ((n & ((1 << i))))
    {
      n -= ((1 << i));
      u = mul(u, c);
    }
    c = mul(c, c);
    i += 1;
  }
  return u;
}

func main()
{
  read(n, x);
  memset(dp, 0, cpp_sizeof((dp)));
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i]);
      cnt[a[i]] += 1;
      i += 1;
    }
  }
  preProcess();
  var sum = 0;
  if ((x <= 100))
  {
    {
      var i = 0;
      while ((i <= x))
      {
        sum += dp[i];
        sum %= mod;
        i += 1;
      }
    }
    write(sum);
    exit(0);
  }
  sum = 0;
  {
    var i = 0;
    while ((i <= 100))
    {
      sum += dp[i];
      sum %= mod;
      i += 1;
    }
  }
  var w: dynamic;
  var ans: dynamic;
  makeMatrix(101, 101, w);
  w[100][100] = 1;
  {
    var i = 0;
    while ((i <= 98))
    {
      w[i][(i + 1)] = 1;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= 99))
    {
      w[99][i] = cnt[(100 - i)];
      w[100][i] = cnt[(100 - i)];
      i += 1;
    }
  }
  ans = Pow((x - 100), w);
  sum *= ans[100][100];
  {
    var i = 0;
    while ((i < 100))
    {
      sum += (((dp[(i + 1)] % mod)) * ((ans[100][i] % mod)));
      sum %= mod;
      i += 1;
    }
  }
  write(sum);
  return 0;
}
