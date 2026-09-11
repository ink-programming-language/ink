// Translated from solution.cpp.

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  return ( ((b == 0)) ? a : gcd(b, (a % b)));
}

func lcm(a: dynamic, b: dynamic) -> dynamic
{
  return ((((a * b)) / gcd(a, b)));
}

func pw(b: dynamic, p: dynamic) -> dynamic
{
  if ((!p))
  {
    return 1;
  }
  var sq: dynamic = pw(b, (p / 2));
  sq *= sq;
  if ((p % 2))
  {
    sq *= b;
  }
  return sq;
}

func sd(x: dynamic) -> dynamic
{
  return  ((x < 10)) ? x : ((x % 10) + sd((x / 10)));
}

func sq(x: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < x))
    {
      if (((cpp_cast(i) * i) > x))
      {
        return ((i - 1));
      }
      i += 1;
    }
  }
  return cpp_double(1);
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  var cnt: dynamic = (k - 1);
  {
    var i: dynamic = 0;
    while ((i < k))
    {
      var x: dynamic = cpp_uninitialized();
      read(x);
      v[i].resize(x);
      {
        var j: dynamic = 0;
        while ((j < x))
        {
          read(v[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < k))
    {
      if ((v[i][0] == 1))
      {
        {
          var j: dynamic = 0;
          while ((j < (v[i].size() - 1)))
          {
            if (((v[i][(j + 1)] - 1) != v[i][j]))
            {
              cnt += (2 * (((v[i].size() - j) - 1)));
              break;
            }
            j += 1;
          }
        }
        i += 1;
        continue;
      }
      cnt += (2 * ((v[i].size() - 1)));
      i += 1;
    }
  }
  write(cnt);
}
