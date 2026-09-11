// Translated from solution.cpp.

var mod: dynamic = 1000000007;

var inf: dynamic = (1e18 + 5);

var MX: dynamic = 303030;

var cox: dynamic = [1, -1, 0, 0];

var coy: dynamic = [0, 0, 1, -1];

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  return  (b) ? gcd(b, (a % b)) : a;
}

func lcm(a: dynamic, b: dynamic) -> dynamic
{
  return ((a * b) / gcd(a, b));
}

func leastbit(a: dynamic) -> dynamic
{
  return (a & ((-a)));
}

func C(a: dynamic, b: dynamic) -> dynamic
{
  var res: dynamic = 1;
  {
    var i: dynamic = 0;
    while ((i < b))
    {
      res = ((res * ((a - i))) / ((i + 1)));
      i += 1;
    }
  }
  return res;
}

func powmod(a: dynamic, b: dynamic) -> dynamic
{
  if ((b == 0))
  {
    return 1;
  }
  var cnt: dynamic = powmod(a, (b / 2));
  (cpp_assign(cnt, "*=", cnt)) %= mod;
  if ((b & 1))
  {
    (cpp_assign(cnt, "*=", a)) %= mod;
  }
  return cnt;
}

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

var d: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var arr: dynamic = cpp_array(200, 200);

var ch: dynamic = cpp_char("a");

func valid(i: dynamic, j: dynamic) -> dynamic
{
  if ((j >= a))
  {
    return (i < d);
  } else
  {
    return (i < b);
  }
}

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(null);
  cout.tie(null);
  var st: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < 200))
    {
      {
        var j: dynamic = 0;
        while ((j < 200))
        {
          arr[i][j] = cpp_char(".");
          j += 1;
        }
      }
      i += 1;
    }
  }
  read(a, b, c, d, n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var x: dynamic = cpp_uninitialized();
      read(x);
      while (cpp_update(x, "--"))
      {
        st.push(ch);
      }
      ch += 1;
      i += 1;
    }
  }
  var flag: dynamic = 1;
  if ((((((min(b, d) & 1)) && (d > b))) || (((((min(b, d) & 1) ^ 1)) && (b > d)))))
  {
    flag = 0;
  }
  {
    var i: dynamic = 0;
    while ((i < max(b, d)))
    {
      if ((flag == 0))
      {
        {
          var j: dynamic = 0;
          while ((j < (a + c)))
          {
            if (valid(i, j))
            {
              arr[i][j] = st.top();
              st.pop();
            }
            j += 1;
          }
        }
      } else
      {
        {
          var j: dynamic = ((a + c) - 1);
          while ((j >= 0))
          {
            if (valid(i, j))
            {
              arr[i][j] = st.top();
              st.pop();
            }
            j -= 1;
          }
        }
      }
      flag = (!flag);
      i += 1;
    }
  }
  write("YES", "\n");
  {
    var i: dynamic = 0;
    while ((i < max(b, d)))
    {
      {
        var j: dynamic = 0;
        while ((j < (a + c)))
        {
          write(arr[i][j]);
          j += 1;
        }
      }
      write("\n");
      i += 1;
    }
  }
  return 0;
}
