// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var t: dynamic;

var su = [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, -1];

var MODER = [223092870, 2756205443, 907383479, 42600829, 97];

var b = cpp_array(100);

var a = cpp_array(20000);

var c: dynamic;

var ans: dynamic;

var ss = cpp_array(100);

var f = cpp_array(5, 17);

func init()
{
  {
    var i = 2;
    while ((i <= 16))
    {
      var l = 1;
      {
        var k = 1;
        while ((k <= n))
        {
          k *= i;
          l *= (i + 1);
        }
      }
      {
        var j = 0;
        while ((j < 5))
        {
          f[i][j].resize(l, -1);
          j += 1;
        }
      }
      i += 1;
    }
  }
}

func calc(d: dynamic, p: dynamic)
{
  var x = 0;
  var y = 0;
  {
    var i = 0;
    while ((i < t))
    {
      x = ((x * ((d + 1))) + b[i]);
      i += 1;
    }
  }
  if ((f[d][p][x] != -1))
  {
    return x;
  }
  f[d][p][x] = 1;
  {
    var i = 0;
    while ((i < t))
    {
      if ((b[i] == d))
      {
        {
          var j = 0;
          while ((j < d))
          {
            b[i] = j;
            f[d][p][x] = (((f[d][p][x] * f[d][p][calc(d, p)])) % MODER[p]);
            j += 1;
          }
        }
        b[i] = d;
        return x;
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < t))
    {
      y = ((y * d) + b[i]);
      i += 1;
    }
  }
  if ((y < n))
  {
    f[d][p][x] = (a[y] % MODER[p]);
  }
  return x;
}

func main()
{
  scanf("%d", (&n));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%I64d", (&a[i]));
      i += 1;
    }
  }
  scanf("%d", (&m));
  init();
  while (cpp_update(m, "--"))
  {
    var d: dynamic;
    var c: dynamic;
    scanf("%d%s%I64d", (&d), ss, (&c));
    t = 0;
    {
      var j = 1;
      while ((j <= n))
      {
        t += 1;
        j *= d;
      }
    }
    var len = strlen(ss);
    {
      var j = 0;
      while ((j < t))
      {
        if ((((len - t) + j) >= 0))
        {
          b[j] = (if (((ss[((len - t) + j)]) == cpp_char("?"))) (d) else (if (((cpp_char("0") <= cpp_cast((&(&(ss[((len - t) + j)]))))) <= cpp_char("9"))) (ss[((len - t) + j)] - cpp_char("0")) else ((ss[((len - t) + j)] - cpp_char("A")) + 10)));
        } else
        {
          b[j] = 0;
        }
        j += 1;
      }
    }
    var j: dynamic;
    {
      var i = 0;
      while ((i < 5))
      {
        j = calc(d, i);
        ans = (((f[d][i][j] + c)) % MODER[i]);
        {
          j = 0;
          while ((j < 25))
          {
            if ((((MODER[i] % su[j]) == 0) && ((ans % su[j]) == 0)))
            {
              break;
            }
            j += 1;
          }
        }
        if ((j < 25))
        {
          break;
        }
        i += 1;
      }
    }
    printf("%d\n", su[j]);
  }
}
