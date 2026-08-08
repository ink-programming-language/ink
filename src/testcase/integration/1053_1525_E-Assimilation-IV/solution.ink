// Translated from solution.cpp.

var nn = 5100;

var inff = 0x3fffffff;

var eps = 1e-8;

var pi = acos(-1.0);

var mod = 998244353;

var n: dynamic;

var m: dynamic;

func POW(x: dynamic, y: dynamic)
{
  var ret = 1;
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

var d = cpp_array(51000, 25);

func main()
{
  scanf("%d%d", (&n), (&m));
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
        while ((j <= m))
        {
          scanf("%d", (&d[i][j]));
          j += 1;
        }
      }
      i += 1;
    }
  }
  var jc = 1;
  {
    var i = 1;
    while ((i <= n))
    {
      jc = (((jc * i)) % mod);
      i += 1;
    }
  }
  var tem = 0;
  {
    var i = 1;
    while ((i <= m))
    {
      var ve: dynamic;
      {
        var j = 1;
        while ((j <= n))
        {
          ve.push_back(d[j][i]);
          j += 1;
        }
      }
      sort(ve.begin(), ve.end());
      var num = 1;
      {
        var j = 0;
        while ((j < n))
        {
          var chose = max(0, ((ve[j] - 1) - j));
          num = (((num * chose)) % mod);
          j += 1;
        }
      }
      tem = (((tem + num)) % mod);
      i += 1;
    }
  }
  var ans = ((((m * jc) - tem)) % mod);
  ans = (((ans + mod)) % mod);
  ans = (((ans * POW(jc, (mod - 2)))) % mod);
  write(ans, "\n");
  return 0;
}
