// Translated from solution.cpp.

func mul(a: dynamic, b: dynamic, m: dynamic = 1000000007) -> dynamic
{
  var res: dynamic = 0;
  a = (a % m);
  while ((b > 0))
  {
    if (((b % 2) == 1))
    {
      res = (((res + a)) % m);
    }
    a = (((a * 2)) % m);
    b /= 2;
  }
  return (res % m);
}

func expMod(a: dynamic, b: dynamic, m: dynamic = 1000000007) -> dynamic
{
  var x: dynamic = 1;
  var y: dynamic = a;
  while (b)
  {
    if (((b % 2) == 1))
    {
      x = mul(x, y, m);
    }
    y = mul(y, y, m);
    b = (b / 2);
  }
  return x;
}

func getBit(n: dynamic, i: dynamic) -> dynamic
{
  return (((n >> i)) & 1);
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var N: dynamic = cpp_uninitialized();
  var M: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  read(N, M, k);
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      read(x);
      a.push_back(x);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < M))
    {
      read(x);
      b.push_back(x);
      i += 1;
    }
  }
  var ta: dynamic = cpp_uninitialized();
  var tb: dynamic = cpp_uninitialized();
  var cnt: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      if ((a[i] == 0))
      {
        if ((cnt != 0))
        {
          {
            var j: dynamic = 1;
            while ((j <= cnt))
            {
              ta[j] += ((cnt - j) + 1);
              j += 1;
            }
          }
        }
        cnt = 0;
      }
      if ((a[i] == 1))
      {
        cnt += 1;
      }
      i += 1;
    }
  }
  if ((cnt != 0))
  {
    {
      var j: dynamic = 1;
      while ((j <= cnt))
      {
        ta[j] += ((cnt - j) + 1);
        j += 1;
      }
    }
  }
  cnt = 0;
  {
    var i: dynamic = 0;
    while ((i < M))
    {
      if ((b[i] == 0))
      {
        if ((cnt != 0))
        {
          {
            var j: dynamic = 1;
            while ((j <= cnt))
            {
              tb[j] += ((cnt - j) + 1);
              j += 1;
            }
          }
        }
        cnt = 0;
      }
      if ((b[i] == 1))
      {
        cnt += 1;
      }
      i += 1;
    }
  }
  if ((cnt != 0))
  {
    {
      var j: dynamic = 1;
      while ((j <= cnt))
      {
        tb[j] += ((cnt - j) + 1);
        j += 1;
      }
    }
  }
  cnt = 0;
  var ans: dynamic = 0;
  for (var i: dynamic in ta)
  {
    if (((k % (i.first)) == 0))
    {
      if ((tb.find((k / (i.first))) != tb.end()))
      {
        ans += (i.second * tb[(k / (i.first))]);
      }
    }
  }
  write(ans, "\n");
}
