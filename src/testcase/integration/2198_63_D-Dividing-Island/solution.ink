// Translated from solution.cpp.

var mod = 1000000007;

var inf = (1e18 + 5);

var MX = 303030;

var cox = [1, -1, 0, 0];

var coy = [0, 0, 1, -1];

func gcd(a: dynamic, b: dynamic)
{
  return if (b) gcd(b, (a % b)) else a;
}

func lcm(a: dynamic, b: dynamic)
{
  return ((a * b) / gcd(a, b));
}

func leastbit(a: dynamic)
{
  return (a & ((-a)));
}

func C(a: dynamic, b: dynamic)
{
  var res = 1;
  {
    var i = 0;
    while ((i < b))
    {
      res = ((res * ((a - i))) / ((i + 1)));
      i += 1;
    }
  }
  return res;
}

func powmod(a: dynamic, b: dynamic)
{
  if ((b == 0))
  {
    return 1;
  }
  var cnt = powmod(a, (b / 2));
  (cpp_assign(cnt, "*=", cnt)) %= mod;
  if ((b & 1))
  {
    (cpp_assign(cnt, "*=", a)) %= mod;
  }
  return cnt;
}

var a: dynamic;

var b: dynamic;

var c: dynamic;

var d: dynamic;

var n: dynamic;

var arr = cpp_array(200, 200);

var ch = cpp_char("a");

func valid(i: dynamic, j: dynamic)
{
  if ((j >= a))
  {
    return (i < d);
  } else
  {
    return (i < b);
  }
}

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(null);
  cout.tie(null);
  var st: dynamic;
  {
    var i = 0;
    while ((i < 200))
    {
      {
        var j = 0;
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
    var i = 0;
    while ((i < n))
    {
      var x: dynamic;
      read(x);
      while (cpp_update(x, "--"))
      {
        st.push(ch);
      }
      ch += 1;
      i += 1;
    }
  }
  var flag = 1;
  if ((((((min(b, d) & 1)) && (d > b))) || (((((min(b, d) & 1) ^ 1)) && (b > d)))))
  {
    flag = 0;
  }
  {
    var i = 0;
    while ((i < max(b, d)))
    {
      if ((flag == 0))
      {
        {
          var j = 0;
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
          var j = ((a + c) - 1);
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
    var i = 0;
    while ((i < max(b, d)))
    {
      {
        var j = 0;
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
