// Translated from solution.cpp.

var nn: dynamic = 5100;

var inff: dynamic = 0x3fffffff;

var eps: dynamic = 1e-8;

var pi: dynamic = acos(-1.0);

var mod: dynamic = 998244353;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

func POW(x: dynamic, y: dynamic) -> dynamic
{
  var ret: dynamic = 1;
  while (y)
  {
    if ((y & 1))
    {
      ret = (((ret * x)) % mod);
    }
    x = (((x * x)) % mod);
    y /= 2;
  }
  return ret;
}

var d: dynamic = cpp_array(51000, 25);

func main() -> dynamic
{
  scanf("%d%d", (&n), (&m));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
        while ((j <= m))
        {
          scanf("%d", (&d[i][j]));
          j += 1;
        }
      }
      i += 1;
    }
  }
  var jc: dynamic = 1;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      jc = (((jc * i)) % mod);
      i += 1;
    }
  }
  var tem: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      var ve: dynamic = cpp_uninitialized();
      {
        var j: dynamic = 1;
        while ((j <= n))
        {
          ve.push_back(d[j][i]);
          j += 1;
        }
      }
      sort(ve.begin(), ve.end());
      var num: dynamic = 1;
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          var chose: dynamic = max(0, ((ve[j] - 1) - j));
          num = (((num * chose)) % mod);
          j += 1;
        }
      }
      tem = (((tem + num)) % mod);
      i += 1;
    }
  }
  var ans: dynamic = ((((m * jc) - tem)) % mod);
  ans = (((ans + mod)) % mod);
  ans = (((ans * POW(jc, (mod - 2)))) % mod);
  write(ans, "\n");
  return 0;
}
