// Translated from solution.cpp.

func gcd(a: dynamic, b: dynamic)
{
  return (if ((b == 0)) a else gcd(b, (a % b)));
}

func lcm(a: dynamic, b: dynamic)
{
  return ((((a * b)) / gcd(a, b)));
}

func pw(b: dynamic, p: dynamic)
{
  if ((!p))
  {
    return 1;
  }
  var sq = pw(b, (p / 2));
  sq *= sq;
  if ((p % 2))
  {
    sq *= b;
  }
  return sq;
}

func sd(x: dynamic)
{
  return if ((x < 10)) x else ((x % 10) + sd((x / 10)));
}

func sq(x: dynamic)
{
  {
    var i = 0;
    while ((i < x))
    {
      if (((cpp_cast(i) * i) > x))
      {
        return ((i - 1));
      }
      i += 1;
    }
  }
  return double(1);
}

func main()
{
  var n: dynamic;
  var k: dynamic;
  read(n, k);
  var cnt = (k - 1);
  {
    var i = 0;
    while ((i < k))
    {
      var x: dynamic;
      read(x);
      v[i].resize(x);
      {
        var j = 0;
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
    var i = 0;
    while ((i < k))
    {
      if ((v[i][0] == 1))
      {
        {
          var j = 0;
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
