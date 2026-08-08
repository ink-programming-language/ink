// Translated from solution.cpp.

var mod = 1000000007;

var N = 200005;

func dfs(v: dynamic)
{
  vis[v] = 1;
  for (var i in adj[v])
  {
    if ((!vis[i]))
    {
      dfs(i);
    }
  }
  return;
}

func isPrime(n: dynamic)
{
  if ((n < 2))
  {
    return false;
  }
  {
    var i = 2;
    while (((i * i) <= n))
    {
      if (((n % i) == 0))
      {
        return false;
      }
      i += 1;
    }
  }
  return true;
}

func factorial(n: dynamic)
{
  return if ((((n == 1) || (n == 0)))) 1 else (n * factorial((n - 1)));
}

func power(x: dynamic, y: dynamic)
{
  var res = 1;
  x = x;
  while ((y > 0))
  {
    if ((y & 1))
    {
      res = (((res * x)) % mod);
    }
    y = (y >> 1);
    x = (((x * x)) % mod);
  }
  return (res % mod);
}

func gcd(a: dynamic, b: dynamic)
{
  if ((a == 0))
  {
    return b;
  }
  return gcd((b % a), a);
}

func lcm(a: dynamic, b: dynamic)
{
  return (((a / gcd(a, b)) * b));
}

func max(a: dynamic, b: dynamic)
{
  var ans = if ((a > b)) a else b;
  return ans;
}

func min(a: dynamic, b: dynamic)
{
  var ans = if ((a < b)) a else b;
  return ans;
}

func root(a: dynamic, i: dynamic)
{
  while ((a[i] != i))
  {
    a[i] = a[a[i]];
    i = a[i];
  }
  return i;
}

func unionn(a: dynamic, i: dynamic, j: dynamic)
{
  var root_i = root(a, i);
  var root_j = root(a, j);
  a[root_i] = root_j;
  return;
}

func fun()
{
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  fun();
  var tt = 1;
  while (cpp_update(tt, "--"))
  {
    var k: dynamic;
    read(k);
    var a: dynamic;
    read(a);
    var n = a.size();
    if ((n % k))
    {
      var s = "1";
      {
        var i = 1;
        while ((i < k))
        {
          s += cpp_char("0");
          i += 1;
        }
      }
      var m = (n / k);
      var ans = s;
      {
        var i = 0;
        while ((i < m))
        {
          ans += s;
          i += 1;
        }
      }
      write(ans, "\n");
    } else
    {
      var t = a.substr(0, k);
      var s = "";
      var m = (n / k);
      {
        var i = 0;
        while ((i < m))
        {
          s += t;
          i += 1;
        }
      }
      if ((s > a))
      {
        write(s, "\n");
      } else
      {
        var j = -1;
        {
          var i = (k - 1);
          while ((i >= 0))
          {
            if ((a[i] != cpp_char("9")))
            {
              j = i;
              break;
            }
            i -= 1;
          }
        }
        if ((j == -1))
        {
          var s = "1";
          {
            var i = 1;
            while ((i < k))
            {
              s += cpp_char("0");
              i += 1;
            }
          }
          var m = (n / k);
          var ans = s;
          {
            var i = 0;
            while ((i < m))
            {
              ans += s;
              i += 1;
            }
          }
          write(ans, "\n");
        } else
        {
          var t = a.substr(0, k);
          t[j] += 1;
          {
            var i = (j + 1);
            while ((i < k))
            {
              t[i] = cpp_char("0");
              i += 1;
            }
          }
          var m = (n / k);
          var ans = t;
          {
            var i = 0;
            while ((i < (m - 1)))
            {
              ans += t;
              i += 1;
            }
          }
          write(ans, "\n");
        }
      }
    }
  }
  return 0;
}
