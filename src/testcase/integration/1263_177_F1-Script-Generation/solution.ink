// Translated from solution.cpp.

var maxi: dynamic = 2000000000;

var maxq: dynamic = 1000000000;

var eps: dynamic = 1e-10;

var pi: dynamic = 3.1415926535897932;

var inf: dynamic = 1e+18;

var mo: dynamic = 1000000007;

var stn: dynamic = cpp_uninitialized();

var ms: dynamic = cpp_array(1111, 1111);

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var st: dynamic = cpp_array(1111111);

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var z: dynamic = cpp_uninitialized();

var sum: dynamic = cpp_uninitialized();

var f: dynamic = cpp_array(11111);

func rec(x: dynamic, y: dynamic) -> dynamic
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
      var i: dynamic = 0;
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

func main() -> dynamic
{
  read(n, k, t);
  {
    var i: dynamic = 0;
    while ((i < k))
    {
      read(x, y, z);
      ms[x][y] = z;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
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
