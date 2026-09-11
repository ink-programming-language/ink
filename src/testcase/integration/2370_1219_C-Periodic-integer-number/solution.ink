// Translated from solution.cpp.

var mod: dynamic = 1000000007;

var N: dynamic = 200005;

func dfs(v: dynamic) -> dynamic
{
  vis[v] = 1;
  for (var i: dynamic in adj[v])
  {
    if ((!vis[i]))
    {
      dfs(i);
    }
  }
  return;
}

func isPrime(n: dynamic) -> dynamic
{
  if ((n < 2))
  {
    return false;
  }
  {
    var i: dynamic = 2;
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

func factorial(n: dynamic) -> dynamic
{
  return  ((((n == 1) || (n == 0)))) ? 1 : (n * factorial((n - 1)));
}

func power(x: dynamic, y: dynamic) -> dynamic
{
  var res: dynamic = 1;
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

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  if ((a == 0))
  {
    return b;
  }
  return gcd((b % a), a);
}

func lcm(a: dynamic, b: dynamic) -> dynamic
{
  return (((a / gcd(a, b)) * b));
}

func max(a: dynamic, b: dynamic) -> dynamic
{
  var ans: dynamic =  ((a > b)) ? a : b;
  return ans;
}

func min(a: dynamic, b: dynamic) -> dynamic
{
  var ans: dynamic =  ((a < b)) ? a : b;
  return ans;
}

func root(a: dynamic, i: dynamic) -> dynamic
{
  while ((a[i] != i))
  {
    a[i] = a[a[i]];
    i = a[i];
  }
  return i;
}

func unionn(a: dynamic, i: dynamic, j: dynamic) -> dynamic
{
  var root_i: dynamic = root(a, i);
  var root_j: dynamic = root(a, j);
  a[root_i] = root_j;
  return;
}

func fun() -> dynamic
{
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  fun();
  var tt: dynamic = 1;
  while (cpp_update(tt, "--"))
  {
    var k: dynamic = cpp_uninitialized();
    read(k);
    var a: dynamic = cpp_uninitialized();
    read(a);
    var n: dynamic = a.size();
    if ((n % k))
    {
      var s: dynamic = "1";
      {
        var i: dynamic = 1;
        while ((i < k))
        {
          s += cpp_char("0");
          i += 1;
        }
      }
      var m: dynamic = (n / k);
      var ans: dynamic = s;
      {
        var i: dynamic = 0;
        while ((i < m))
        {
          ans += s;
          i += 1;
        }
      }
      write(ans, "\n");
    } else
    {
      var t: dynamic = a.substr(0, k);
      var s: dynamic = "";
      var m: dynamic = (n / k);
      {
        var i: dynamic = 0;
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
        var j: dynamic = -1;
        {
          var i: dynamic = (k - 1);
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
          var s: dynamic = "1";
          {
            var i: dynamic = 1;
            while ((i < k))
            {
              s += cpp_char("0");
              i += 1;
            }
          }
          var m: dynamic = (n / k);
          var ans: dynamic = s;
          {
            var i: dynamic = 0;
            while ((i < m))
            {
              ans += s;
              i += 1;
            }
          }
          write(ans, "\n");
        } else
        {
          var t: dynamic = a.substr(0, k);
          t[j] += 1;
          {
            var i: dynamic = (j + 1);
            while ((i < k))
            {
              t[i] = cpp_char("0");
              i += 1;
            }
          }
          var m: dynamic = (n / k);
          var ans: dynamic = t;
          {
            var i: dynamic = 0;
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
