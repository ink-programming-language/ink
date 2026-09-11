// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var su: dynamic = [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, -1];

var MODER: dynamic = [223092870, 2756205443, 907383479, 42600829, 97];

var b: dynamic = cpp_array(100);

var a: dynamic = cpp_array(20000);

var c: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var ss: dynamic = cpp_array(100);

var f: dynamic = cpp_array(5, 17);

func init() -> dynamic
{
  {
    var i: dynamic = 2;
    while ((i <= 16))
    {
      var l: dynamic = 1;
      {
        var k: dynamic = 1;
        while ((k <= n))
        {
          k *= i;
          l *= (i + 1);
        }
      }
      {
        var j: dynamic = 0;
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

func calc(d: dynamic, p: dynamic) -> dynamic
{
  var x: dynamic = 0;
  var y: dynamic = 0;
  {
    var i: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < t))
    {
      if ((b[i] == d))
      {
        {
          var j: dynamic = 0;
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
    var i: dynamic = 0;
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

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 0;
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
    var d: dynamic = cpp_uninitialized();
    var c: dynamic = cpp_uninitialized();
    scanf("%d%s%I64d", (&d), ss, (&c));
    t = 0;
    {
      var j: dynamic = 1;
      while ((j <= n))
      {
        t += 1;
        j *= d;
      }
    }
    var len: dynamic = strlen(ss);
    {
      var j: dynamic = 0;
      while ((j < t))
      {
        if ((((len - t) + j) >= 0))
        {
          b[j] = ( (((ss[((len - t) + j)]) == cpp_char("?"))) ? (d) : ( (((cpp_char("0") <= cpp_cast((&(&(ss[((len - t) + j)]))))) <= cpp_char("9"))) ? (ss[((len - t) + j)] - cpp_char("0")) : ((ss[((len - t) + j)] - cpp_char("A")) + 10)));
        } else
        {
          b[j] = 0;
        }
        j += 1;
      }
    }
    var j: dynamic = cpp_uninitialized();
    {
      var i: dynamic = 0;
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
