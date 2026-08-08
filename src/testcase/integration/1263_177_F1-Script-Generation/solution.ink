// Translated from solution.cpp.

var maxi = 2000000000;

var maxq = 1000000000;

var eps = 1e-10;

var pi = 3.1415926535897932;

var inf = 1e+18;

var mo = 1000000007;

var stn: dynamic;

var ms = cpp_array(1111, 1111);

var n: dynamic;

var k: dynamic;

var t: dynamic;

var st = cpp_array(1111111);

var x: dynamic;

var y: dynamic;

var z: dynamic;

var sum: dynamic;

var f = cpp_array(11111);

func rec(x: dynamic, y: dynamic)
{
  sum += ms[x][y];
  if ((x == n))
  {
    stn += 1;
    st[stn] = sum;
  }
  if (y)
  {
    f[y] = true;
  }
  if ((x < n))
  {
    {
      var i = 0;
      while ((i <= n))
      {
        if (((f[i] == false) && ((ms[(x + 1)][i] || (i == 0)))))
        {
          rec((x + 1), i);
        }
        i += 1;
      }
    }
  }
  sum -= ms[x][y];
  f[y] = false;
}

func main()
{
  read(n, k, t);
  {
    var i = 0;
    while ((i < k))
    {
      read(x, y, z);
      ms[x][y] = z;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= n))
    {
      if ((ms[1][i] || (i == 0)))
      {
        rec(1, i);
      }
      i += 1;
    }
  }
  sort((st + 1), ((st + stn) + 1));
  write(st[t], "\n");
  return 0;
}
