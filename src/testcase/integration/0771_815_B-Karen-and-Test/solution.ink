// Translated from solution.cpp.

class outputer
{
}

class outputable
{
}

func sqr(x: dynamic)
{
  return (x * x);
}

func umx(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
    return 1;
  }
  return 0;
}

func umn(a: dynamic, b: dynamic)
{
  if ((b < a))
  {
    a = b;
    return 1;
  }
  return 0;
}

var N = 200000;

var mod = 1000000007;

class Input
{
  var n: dynamic;
  var a: dynamic = cpp_array(N);
  func read()
  {
      if ((!((cin >> n))))
      {
        return 0;
      }
      {
        var i = int_cpp(0);
        while ((i < int_cpp(n)))
        {
          scanf(cpp_expression("\"%\""), SCNd64, (&a[i]));
          i += 1;
        }
      }
      return 1;
    }
  func init(input: dynamic)
  {
      (*this) = input;
    }
}

class Data
{
  var ans: dynamic;
  func write()
  {
      write(ans, "\n");
    }
}

class Solution
{
  var f: dynamic = cpp_array((N + 1));
  var rf: dynamic = cpp_array((N + 1));
  func c(x: dynamic, y: dynamic)
  {
      return ((((f[(x + y)] * rf[x]) % mod) * rf[y]) % mod);
    }
  func pw(val: dynamic, k: dynamic)
  {
      var res = 1;
      while (k)
      {
        if ((k & 1))
        {
          res = ((res * val) % mod);
        }
        val = ((val * val) % mod);
        k >>= 1;
      }
      return res;
    }
  var sgn: dynamic;
  func iteration()
  {
      n -= 1;
      {
        var i = int_cpp(0);
        while ((i < int_cpp(n)))
        {
          a[i] = ((((a[i] + (sgn * a[(i + 1)])) + mod)) % mod);
          sgn *= -1;
          i += 1;
        }
      }
    }
  func calc(t: dynamic)
  {
      var m = ((((n + 1) - t)) / 2);
      var res = 0;
      {
        var i = int_cpp(0);
        while ((i < int_cpp(m)))
        {
          res = (((res + (a[(t + (2 * i))] * c(i, ((m - 1) - i))))) % mod);
          i += 1;
        }
      }
      return res;
    }
  func solve()
  {
      f[0] = 1;
      {
        var i = int_cpp(0);
        while ((i < int_cpp(n)))
        {
          f[(i + 1)] = ((f[i] * ((i + 1))) % mod);
          i += 1;
        }
      }
      {
        var i = int_cpp(0);
        while ((i < int_cpp((n + 1))))
        {
          rf[i] = pw(f[i], (mod - 2));
          i += 1;
        }
      }
      if ((n == 1))
      {
        ans = a[0];
        return;
      }
      sgn = 1;
      if ((n & 1))
      {
        iteration();
      }
      var val0 = calc(0);
      var val1 = calc(1);
      if ((((n / 2) % 2) == 0))
      {
        sgn *= -1;
      }
      ans = ((((val0 + (sgn * val1)) + mod)) % mod);
    }
  func clear()
  {
      (*this) = Solution();
    }
}

var sol: dynamic;

func main()
{
  cout.setf((ios.showpoint | ios.fixed));
  cout.precision(20);
  sol.read();
  sol.solve();
  sol.write();
  return 0;
}
