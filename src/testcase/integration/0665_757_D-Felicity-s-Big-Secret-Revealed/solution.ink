// Translated from solution.cpp.

var n: dynamic;

var b = cpp_array(30);

var s: dynamic;

var mod: dynamic;

var memo = cpp_array(76, 602222);

func solve(mask: dynamic, i: dynamic)
{
  if ((i == n))
  {
    {
      var j = 0;
      while ((j < 30))
      {
        if ((mask == b[j]))
        {
          return 1;
        }
        j += 1;
      }
    }
    return 0;
  }
  var ret = memo[mask][i];
  if ((ret != -1))
  {
    return ret;
  }
  var sum = 0;
  {
    var j = 0;
    while ((j < 75))
    {
      if (((i + j) <= n))
      {
        var num = 0;
        {
          var k = 0;
          while ((k < j))
          {
            num = ((num * 2) + ((s[(i + k)] - cpp_char("0"))));
            if ((num > 20))
            {
              break;
            }
            k += 1;
          }
        }
        num -= 1;
        if (((num <= 18) && (num >= 0)))
        {
          sum += solve((mask | ((1 << num))), (i + j));
          sum %= mod;
          if (((i + j) != n))
          {
            sum += solve((mask | ((1 << num))), n);
          }
        }
        sum = (sum % mod);
      }
      j += 1;
    }
  }
  ret = sum;
  return sum;
}

func main()
{
  memset(memo, -1, cpp_sizeof((memo)));
  mod = (1e9 + 7);
  {
    var j = 0;
    while ((j < 30))
    {
      b[j] = (((1 << ((j + 1)))) - 1);
      j += 1;
    }
  }
  read(n, s);
  var sum = 0;
  {
    var i = 0;
    while ((i < n))
    {
      sum += solve(0, i);
      sum %= mod;
      i += 1;
    }
  }
  write(sum, "\n");
}
